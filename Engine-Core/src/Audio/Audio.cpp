#include <pch.h>
#include <sndfile.h>

namespace
{
	static TGE::AudioData s_AudioData;
}

#pragma region testing

struct WavHeader
{
	char        ChunkHandle[4];
	uint32_t    ChunckSize;
	char        Format[4];
	char        SubChunk1Handle[4];
	uint32_t    SubChunk1Size;
	uint16_t    AudioFormat;
	uint16_t    NumChannels;
	uint32_t    SampleRate;
	uint32_t    ByteRate;
	uint16_t    BlockAlign;
	uint16_t    BitsPerSample;
	char        SubChunk2Handle[4];
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

	if (std::string(Header.ChunkHandle, 4) != "RIFF" || std::string(Header.Format, 4) != "WAVE" ||
		std::string(Header.SubChunk1Handle, 4) != "fmt " || Header.AudioFormat != 1) {
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

#pragma endregion testing

enum FormatType {
	Int16,
	Float,
	IMA4,
	MSADPCM
};

namespace TGE 
{
	
#pragma region Audio Buffer

	AudioBuffers::AudioBuffers()
	{
	}

	AudioBuffers::~AudioBuffers()
	{
		for (size_t i = 0; i < m_BufferList.size(); i++)
			alDeleteBuffers(1, &m_BufferList[i]);
	}

	AudioHandle AudioBuffers::Generate(const std::string_view& FileLocation)
	{
		void* membuf = nullptr;
		sf_count_t num_frames;
		ALsizei num_bytes;
		
		ALint byteblockalign = 0;
		ALint splblockalign = 0;
		
		enum FormatType sample_format = Int16;
		
		/*
			Opens an audio file and extracts data into SND_INFO struct
		*/
		
		SF_INFO sfinfo{};
		SNDFILE* sndfile = sf_open(FileLocation.data(), SFM_READ, &sfinfo);
		
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
			 * since libsndfile doesn't provHandlee it in a format-agnostic way.
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
					 * samples-per-block (verifying it's valHandle by converting back
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
		
		if(buffer != 0)
			m_BufferList.push_back(buffer);
		
		return buffer;
	}

	void AudioBuffers::Remove(AudioHandle Buffer)
	{
		auto Iterator = std::find(m_BufferList.begin(), m_BufferList.end(), Buffer);
		if (Iterator != m_BufferList.end())
			m_BufferList.erase(Iterator);
	}


#pragma endregion Audio Buffer

#pragma region Audio Sources

	AudioSources::AudioSources()
	{
	}
	AudioSources::~AudioSources()
	{
		for (Source& src : m_Sources)
		{
			alSourcei(src.Handle, AL_BUFFER, 0);
			alDeleteSources(1, &src.Handle);
		}
	}
	Source AudioSources::PushSource(const SourceData& source, const SourcePriority& Priority, AudioHandle BufferHandle)
	{
		Source TempSource;

		alGenSources(1, &TempSource.Handle);

		alSourcef(TempSource.Handle, AL_PITCH, source.Pitch);
		alSourcef(TempSource.Handle, AL_GAIN, source.Gain);
		alSource3f(TempSource.Handle, AL_POSITION, source.Position.x, source.Position.y, source.Position.z);
		alSource3f(TempSource.Handle, AL_VELOCITY, source.Velocity.x, source.Velocity.y, source.Velocity.z);

		ALenum Loop = source.Looping == true ? AL_TRUE : AL_FALSE;

		alSourcei(TempSource.Handle, AL_LOOPING, Loop);
		
		TempSource.Data = source;
		TempSource.Priority = Priority;
		TempSource.BufferHandle = BufferHandle;

		m_Sources.push_back(TempSource);

		return TempSource;
	}
	void AudioSources::RemoveSource(uint32_t Index)
	{
		if (Index >= m_Sources.size())
			return;

		m_Sources.erase(m_Sources.begin() + Index);
	}
	void AudioSources::EditSource(uint32_t Index, const SourceData& source)
	{
		if (Index >= m_Sources.size())
			return;

		m_Sources[Index].Data = source;
	}
	void AudioSources::EditSource(uint32_t Index, const SourceData& source, const SourcePriority& Priority)
	{
		if (Index >= m_Sources.size())
			return;


		m_Sources[Index].Data = source;
		m_Sources[Index].Priority = Priority;
	}

	void AudioSources::NullAllSources()
	{
		for (size_t i = 0; i < m_Sources.size(); i++)
		{
			alSourceStop(m_Sources[i].Handle);
			alSourcei(m_Sources[i].Handle, AL_BUFFER, 0);
		}
	}
#pragma endregion Audio Sources

#pragma region Main Audio 
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
				
				LOG(std::string(name));
				
			} });
	}
	AudioHandle Audio::Load(const std::string_view& AudioLocation)
	{	
		return s_AudioData.Buffers.Generate(AudioLocation);
	}
	Source Audio::GenerateSource(const SourceData& data, AudioHandle BufferHandle)
	{
		return s_AudioData.Sources.PushSource(data, SourcePriority::Ambient, BufferHandle);
	}
	void Audio::Submit(const Source& src)
	{
		s_AudioData.AudioQueue->Attach({ [src]()
			{
				alSourcei(src.Handle, AL_BUFFER, src.BufferHandle);
				alSourcePlay(src.Handle);
			} });
	}
	void Audio::SubmitV(const std::vector<Source>& Sources)
	{
		s_AudioData.AudioQueue->Attach({ [Sources] 
			{
				std::vector<ALuint> Handles;
				for (size_t i = 0; i < Sources.size(); i++)
				{
					alSourcei(Sources[i].Handle, AL_BUFFER, Sources[i].BufferHandle);
					Handles.push_back(Sources[i].Handle);
				}

				alSourcePlayv(Handles.size(), Handles.data());
			} });
	}

	void Audio::Play(const Source& src)
	{
		s_AudioData.AudioQueue->Attach({ [src]()
			{
				alSourcePlay(src.Handle);
			} });
	}
	void Audio::PlayV(const std::vector<Source>& Sources)
	{
	}

	void Audio::PauseSource(const Source& src)
	{
		s_AudioData.AudioQueue->Attach({ [src]() 
		{
				alSourcePause(src.Handle);
		} });
	}
	void Audio::PauseSourceV(const std::vector<Source>& Sources)
	{
		s_AudioData.AudioQueue->Attach({ [Sources]()
		{
				std::vector<ALuint> SourceHandle;
				for (size_t i = 0; i < Sources.size(); i++)
					SourceHandle.push_back(Sources[i].Handle);

				alSourcePausev(SourceHandle.size(), SourceHandle.data());
		} });
	}

	void Audio::StopSource(const Source& src)
	{
		s_AudioData.AudioQueue->Attach({ [src]()
		{
				alSourcei(src.Handle, AL_BUFFER, 0);
				alSourceStop(src.Handle);
		} });
	}
	void Audio::StopSourceV(const std::vector<Source>& Sources)
	{
		s_AudioData.AudioQueue->Attach({ [Sources]()
		{
				std::vector<ALuint> SourceHandle;
				for (const Source& Src : Sources)
				{
					alSourcei(Src.Handle, AL_BUFFER, 0);
					SourceHandle.push_back(Src.Handle);
				}

				alSourceStopv(SourceHandle.size(), SourceHandle.data());
		} });
	}

	void Audio::EditSource(const Source& src)
	{
		s_AudioData.AudioQueue->Attach({ [src]()
		{
			alSourcef(src.Handle, AL_PITCH, src.Data.Pitch);
			alSourcef(src.Handle, AL_GAIN, src.Data.Gain);
			alSource3f(src.Handle, AL_POSITION, src.Data.Position.x, src.Data.Position.y, src.Data.Position.z);
			alSource3f(src.Handle, AL_VELOCITY, src.Data.Velocity.x, src.Data.Velocity.y, src.Data.Velocity.z);

			ALenum Loop = src.Data.Looping == true ? AL_TRUE : AL_FALSE;

			alSourcei(src.Handle, AL_LOOPING, Loop);
		} });
	}
	void Audio::EditSourceV(const std::vector<Source>& Sources)
	{
		s_AudioData.AudioQueue->Attach({ [Sources]()
		{
			for(size_t i = 0; i < Sources.size(); i++)
			{
				alSourcef(Sources[i].Handle, AL_PITCH, Sources[i].Data.Pitch);
				alSourcef(Sources[i].Handle, AL_GAIN, Sources[i].Data.Gain);
				alSource3f(Sources[i].Handle, AL_POSITION, Sources[i].Data.Position.x, Sources[i].Data.Position.y, Sources[i].Data.Position.z);
				alSource3f(Sources[i].Handle, AL_VELOCITY, Sources[i].Data.Velocity.x, Sources[i].Data.Velocity.y, Sources[i].Data.Velocity.z);

				ALenum Loop = Sources[i].Data.Looping == true ? AL_TRUE : AL_FALSE;

				alSourcei(Sources[i].Handle, AL_LOOPING, Loop);
			}
		} });
	}

	void Audio::Shutdown()
	{
		s_AudioData.GlobalState = GlobalAudioState::Off;

		s_AudioData.AudioQueue->Attach({ []()
			{
				s_AudioData.Sources.NullAllSources();
				s_AudioData.Sources.~AudioSources();
				s_AudioData.Buffers.~AudioBuffers();


				alcMakeContextCurrent(nullptr);
				alcDestroyContext(s_AudioData.Context);
				alcCloseDevice(s_AudioData.CurrentDevice);
			} });


		s_AudioData.AudioQueue->Wait();
		s_AudioData.AudioQueue.release();

		
	}
#pragma endregion Main Audio 
	
}