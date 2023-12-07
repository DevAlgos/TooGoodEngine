#include <pch.h>
#include <sndfile.h>

namespace
{
	static TGE::AudioData s_AudioData;
}

struct WavHeader
{
	char        ChunkID[4];
	uint32_t    ChunckSize;
	char        Format[4];
	char        SubChunk1ID[4];
	uint32_t    SubChunk1Size;
	uint16_t    AudioFormat;
	uint16_t    NumChannels;
	uint32_t    SampleRate;
	uint32_t    ByteRate;
	uint16_t    BlockAlign;
	uint16_t    BitsPerSample;
	char        SubChunk2ID[4];
	uint32_t    SubChunk2Size;
};

std::vector<char> ReadWavFile(const std::string_view& FileLocation, WavHeader& Header)
{
	std::ifstream file(FileLocation.data(), std::ios::binary);
	if (!file.is_open())
	{
		LOGERROR("Error reading file");
		return {};
	}

	file.read(reinterpret_cast<char*>(&Header), sizeof(WavHeader));

	if (std::string(Header.ChunkID, 4) != "RIFF" || std::string(Header.Format, 4) != "WAVE" ||
		std::string(Header.SubChunk1ID, 4) != "fmt " || Header.AudioFormat != 1) {
		LOG("Unsupported WAV format or not PCM.");
		return {};
	}


	std::vector<char> Data(Header.SubChunk2Size);

	file.read(Data.data(), Header.SubChunk2Size);

	if (!file)
	{
		LOGERROR("Error reading data chunk");
		return {};
	}

	return Data;

}


enum FormatType {
	Int16,
	Float,
	IMA4,
	MSADPCM
};

namespace TGE 
{
	void Audio::Init()
	{
		s_AudioData.AudioQueue = std::make_unique<Utils::CommandQueue>();

		s_AudioData.AudioQueue->Attach({ []()
			{
				s_AudioData.CurrentDevice = alcOpenDevice(nullptr);
				if (!s_AudioData.CurrentDevice)
				{
					LOGERROR("Failed to open device");
					__debugbreak();
				}
				
				s_AudioData.Context = alcCreateContext(s_AudioData.CurrentDevice, nullptr);
				if (s_AudioData.Context == nullptr || alcMakeContextCurrent(s_AudioData.Context) == ALC_FALSE)
				{
					if (s_AudioData.Context != nullptr)
						alcDestroyContext(s_AudioData.Context);

					alcCloseDevice(s_AudioData.CurrentDevice);
					LOGERROR("Could not set context!");

					__debugbreak();
				}

				const char* name = NULL;
				if (alcIsExtensionPresent(s_AudioData.CurrentDevice, "ALC_ENUMERATE_ALL_EXT"))
					name = alcGetString(s_AudioData.CurrentDevice, ALC_ALL_DEVICES_SPECIFIER);
				if (!name || alcGetError(s_AudioData.CurrentDevice) != AL_NO_ERROR)
					name = alcGetString(s_AudioData.CurrentDevice, ALC_DEVICE_SPECIFIER);
				
				

				std::string msg = "Opened " + std::string(name);
				LOG(msg);
				
			} });
	}
	AudioID Audio::Load(const std::string_view& AudioLocation)
	{
		AudioID ID = std::numeric_limits<uint32_t>::max();

		s_AudioData.AudioQueue->Attach({ [AudioLocation, &ID]()
			{
				void* membuf = nullptr;
				sf_count_t num_frames;
				ALsizei num_bytes;

				ALint byteblockalign = 0;
				ALint splblockalign = 0;

				enum FormatType sample_format = Int16;

				SF_INFO sfinfo {};
				SNDFILE* sndfile = sf_open(AudioLocation.data(), SFM_READ, &sfinfo);

				switch ((sfinfo.format & SF_FORMAT_SUBMASK))
				{
				case SF_FORMAT_PCM_24:
				case SF_FORMAT_PCM_32:
				case SF_FORMAT_FLOAT:
				case SF_FORMAT_DOUBLE:
				case SF_FORMAT_VORBIS:
				case SF_FORMAT_OPUS:
				case SF_FORMAT_ALAC_20:
				case SF_FORMAT_ALAC_24:
				case SF_FORMAT_ALAC_32:
				case 0x0080/*SF_FORMAT_MPEG_LAYER_I*/:
				case 0x0081/*SF_FORMAT_MPEG_LAYER_II*/:
				case 0x0082/*SF_FORMAT_MPEG_LAYER_III*/:
					if (alIsExtensionPresent("AL_EXT_FLOAT32"))
						sample_format = Float;
					break;
				case SF_FORMAT_IMA_ADPCM:
					/* ADPCM formats require setting a block alignment as specified in the
					 * file, which needs to be read from the wave 'fmt ' chunk manually
					 * since libsndfile doesn't provide it in a format-agnostic way.
					 */
					if (sfinfo.channels <= 2 && (sfinfo.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
						&& alIsExtensionPresent("AL_EXT_IMA4")
						&& alIsExtensionPresent("AL_SOFT_block_alignment"))
						sample_format = IMA4;
					break;
				case SF_FORMAT_MS_ADPCM:
					if (sfinfo.channels <= 2 && (sfinfo.format & SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
						&& alIsExtensionPresent("AL_SOFT_MSADPCM")
						&& alIsExtensionPresent("AL_SOFT_block_alignment"))
						sample_format = MSADPCM;
					break;
				}

				if (sample_format == IMA4 || sample_format == MSADPCM)
				{
					/* For ADPCM, lookup the wave file's "fmt " chunk, which is a
					 * WAVEFORMATEX-based structure for the audio format.
					 */
					SF_CHUNK_INFO inf = { "fmt ", 4, 0, NULL };
					SF_CHUNK_ITERATOR* iter = sf_get_chunk_iterator(sndfile, &inf);

					/* If there's an issue getting the chunk or block alignment, load as
					 * 16-bit and have libsndfile do the conversion.
					 */
					if (!iter || sf_get_chunk_size(iter, &inf) != SF_ERR_NO_ERROR || inf.datalen < 14)
						sample_format = Int16;
					else
					{
						ALubyte* fmtbuf = (ALubyte*)calloc(inf.datalen, 1);
						inf.data = fmtbuf;
						if (sf_get_chunk_data(iter, &inf) != SF_ERR_NO_ERROR)
							sample_format = Int16;
						else
						{
							/* Read the nBlockAlign field, and convert from bytes- to
							 * samples-per-block (verifying it's valid by converting back
							 * and comparing to the original value).
							 */
							byteblockalign = fmtbuf[12] | (fmtbuf[13] << 8);
							if (sample_format == IMA4)
							{
								splblockalign = (byteblockalign / sfinfo.channels - 4) / 4 * 8 + 1;
								if (splblockalign < 1
									|| ((splblockalign - 1) / 2 + 4) * sfinfo.channels != byteblockalign)
									sample_format = Int16;
							}
							else
							{
								splblockalign = (byteblockalign / sfinfo.channels - 7) * 2 + 2;
								if (splblockalign < 2
									|| ((splblockalign - 2) / 2 + 7) * sfinfo.channels != byteblockalign)
									sample_format = Int16;
							}
						}
						free(fmtbuf);
					}
				}

				if (sample_format == Int16)
				{
					splblockalign = 1;
					byteblockalign = sfinfo.channels * 2;
				}
				else if (sample_format == Float)
				{
					splblockalign = 1;
					byteblockalign = sfinfo.channels * 4;
				}

				ALenum format = AL_NONE;
				if (sfinfo.channels == 1)
				{
					if (sample_format == Int16)
						format = AL_FORMAT_MONO16;
					else if (sample_format == Float)
						format = AL_FORMAT_MONO_FLOAT32;
					else if (sample_format == IMA4)
						format = AL_FORMAT_MONO_IMA4;
					else if (sample_format == MSADPCM)
						format = AL_FORMAT_MONO_MSADPCM_SOFT;
				}
				else if (sfinfo.channels == 2)
				{
					if (sample_format == Int16)
						format = AL_FORMAT_STEREO16;
					else if (sample_format == Float)
						format = AL_FORMAT_STEREO_FLOAT32;
					else if (sample_format == IMA4)
						format = AL_FORMAT_STEREO_IMA4;
					else if (sample_format == MSADPCM)
						format = AL_FORMAT_STEREO_MSADPCM_SOFT;
				}
				else if (sfinfo.channels == 3)
				{
					if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
					{
						if (sample_format == Int16)
							format = AL_FORMAT_BFORMAT2D_16;
						else if (sample_format == Float)
							format = AL_FORMAT_BFORMAT2D_FLOAT32;
					}
				}
				else if (sfinfo.channels == 4)
				{
					if (sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
					{
						if (sample_format == Int16)
							format = AL_FORMAT_BFORMAT3D_16;
						else if (sample_format == Float)
							format = AL_FORMAT_BFORMAT3D_FLOAT32;
					}
				}
				if (!format)
				{
					sf_close(sndfile);
					return 0;
				}

				if (sfinfo.frames / splblockalign > (sf_count_t)(INT_MAX / byteblockalign))
				{
					sf_close(sndfile);
					return 0;
				}

				membuf = malloc((size_t)(sfinfo.frames / splblockalign * byteblockalign));

				if (sample_format == Int16)
					num_frames = sf_readf_short(sndfile, (short*)membuf, sfinfo.frames);
				else if (sample_format == Float)
					num_frames = sf_readf_float(sndfile, (float*)membuf, sfinfo.frames);
				else
				{
					sf_count_t count = sfinfo.frames / splblockalign * byteblockalign;
					num_frames = sf_read_raw(sndfile, membuf, count);
					if (num_frames > 0)
						num_frames = num_frames / byteblockalign * splblockalign;
				}
				if (num_frames < 1)
				{
					free(membuf);
					sf_close(sndfile);
					return 0;
				}
				num_bytes = (ALsizei)(num_frames / splblockalign * byteblockalign);

				ALuint buffer = 0;
				alGenBuffers(1, &buffer);

				if (splblockalign > 1)
					alBufferi(buffer, AL_UNPACK_BLOCK_ALIGNMENT_SOFT, splblockalign);
				alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

				free(membuf);
				sf_close(sndfile);

				/* Check if an error occurred, and clean up if so. */

				ID = buffer;
			} });
		

		s_AudioData.AudioQueue->Wait();

		if (ID != std::numeric_limits<uint32_t>::max())
			s_AudioData.AudioBuffers.push_back(ID);

		return ID;
	}
	void Audio::Submit(AudioID ID)
	{
		s_AudioData.AudioQueue->Attach({ [ID]()
			{
				ALuint source = 0;
				alGenSources(1, &source);
				alSourcei(source, AL_BUFFER, static_cast<ALint>(ID));

				alSourcePlay(source);
				ALint state = AL_PLAYING;
				do
				{
					alGetSourcei(source, AL_SOURCE_STATE, &state);

				} while (state == AL_PLAYING);

				alDeleteSources(1, &source);
			} });
	}
	void Audio::Shutdown()
	{
		s_AudioData.AudioQueue->Attach({ []()
			{
				for (AudioID& ID : s_AudioData.AudioBuffers)
					alDeleteBuffers(1, &ID);

				alcMakeContextCurrent(nullptr);
				alcDestroyContext(s_AudioData.Context);
				alcCloseDevice(s_AudioData.CurrentDevice);
			} });


		s_AudioData.AudioQueue->Wait();
		s_AudioData.AudioQueue.release();
	}
}