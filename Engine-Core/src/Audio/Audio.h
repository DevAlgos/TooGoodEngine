#pragma once

#include <Utils/Threading/CommandQueue.h>

/*
	###PLAN###
	Audio::Init() -> Create audio thread and set up neccesary data
	Audio::Load(FileLocation) -> Loads audio file and returns an ID
	Audio::Submit(AudioID) -> Sends audio to be played on the audio thread with a file location
	Audio::Submit(AudioID, Duration) -> Sends audio to be played on the audio thread with a file location and duration
*/


namespace TGE
{

	using AudioID = ALuint;

	struct AudioData
	{
		std::unique_ptr<Utils::CommandQueue> AudioQueue;
		std::vector<AudioID> AudioBuffers;
		
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