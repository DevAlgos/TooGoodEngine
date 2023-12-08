#pragma once

#include <Utils/Threading/CommandQueue.h>

#include <AL/alc.h>

namespace TGE
{

	using AudioHandle = ALuint;

	enum class GlobalAudioState {On = 0, Playing, Off};

	enum class SourcePriority
	{
		Ambient = 0, /* In the background*/
		Immediate, /* As soon as pipeline begins*/
		OnCondition
	};

	enum class SourceState
	{
		NotPlaying = 0,
		Playing
	};

	struct SourceData
	{
		float Pitch = 1.0f;
		float Gain = 1.0f;
		bool Looping = false;
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Velocity = { 0.0f, 0.0f, 0.0f };
	};


	struct Source
	{
		ALuint Handle;
		SourceData Data;
		SourcePriority Priority;
		SourceState State = SourceState::NotPlaying;
		ALuint BufferHandle;
		bool Condition = false;
	};

	class AudioBuffers
	{
	public:
		AudioBuffers();
		~AudioBuffers();
		
		AudioHandle Generate(const std::string_view& FileLocation);
		void	Remove(AudioHandle Buffer);

	private:
		std::vector<AudioHandle> m_BufferList;
	};

	class AudioSources
	{
	public:
		AudioSources();
		~AudioSources();

		Source PushSource(const SourceData& source, const SourcePriority& Priority, ALuint BufferHandle);

		void	 RemoveSource(uint32_t Index);
		void	 EditSource(uint32_t Index, const SourceData& source);
		void	 EditSource(uint32_t Index, const SourceData& source, const SourcePriority& Priority);
		void	 NullAllSources();

	private:
		std::vector<Source> m_Sources;
		uint32_t m_SourceCount = 0;
	};

	struct AudioData
	{
		std::unique_ptr<Utils::CommandQueue> AudioQueue;

		AudioSources Sources;
		AudioBuffers Buffers;

		GlobalAudioState GlobalState = GlobalAudioState::On;
		
		ALCdevice* CurrentDevice = nullptr;
		ALCcontext* Context = nullptr;
	};

	class Audio
	{
	public:
		Audio() = delete;
		~Audio() = delete;

		static void		Init();
		static AudioHandle  Load(const std::string_view& AudioLocation);
		static Source	GenerateSource(const SourceData& data, AudioHandle BufferHandle);

		static void		Submit(const Source& src);
		static void		SubmitV(const std::vector<Source>& Sources);

		static void		Play(const Source& src);
		static void		PlayV(const std::vector<Source>& Sources);

		static void		PauseSource(const Source& src);
		static void		PauseSourceV(const std::vector<Source>& Sources);

		static void		StopSource(const Source& src);
		static void		StopSourceV(const std::vector<Source>& Sources);

		static void		EditSource(const Source& src);
		static void		EditSourceV(const std::vector<Source>& Sources);

		static void		Shutdown();

	};
}