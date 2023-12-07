#pragma once

#include <Utils/Threading/CommandQueue.h>

#include <AL/alc.h>

namespace TGE
{

	using AudioID = ALuint;

	//TODO removing Playing state
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
		ALuint BufferID;
		bool Condition = false;
	};

	class AudioBuffers
	{
	public:
		AudioBuffers();
		~AudioBuffers();
		
		AudioID Generate(const std::string_view& FileLocation);
		void	Remove(AudioID Buffer);

	private:
		std::vector<AudioID> m_BufferList;
	};

	//TODO: change return of push source to be the ID of the source itself
	class AudioSources
	{
	public:
		AudioSources();
		~AudioSources();

		uint32_t PushSource(const SourceData& source, const SourcePriority& Priority, ALuint BufferID);

		void	 RemoveSource(uint32_t Index);
		void	 EditSource(uint32_t Index, const SourceData& source);
		void	 EditSource(uint32_t Index, const SourceData& source, const SourcePriority& Priority);

		void PlaySource(uint32_t SourceIndex);

	private:
		std::vector<Source> m_Sources;
		uint32_t m_SourceCount = 0;
	};

	struct AudioData
	{
		std::unique_ptr<Utils::CommandQueue> AudioQueue;

		AudioSources Sources;
		AudioBuffers Buffers;

		std::vector<AudioID> AudioBuffers;

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
		static AudioID  Load(const std::string_view& AudioLocation);
		static void		Submit(AudioID ID);
		static void		Shutdown();

	};
}