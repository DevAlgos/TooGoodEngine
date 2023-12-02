#include <pch.h>
#include "Logger.h"

namespace
{
	static std::unique_ptr<Utils::CommandQueue> s_CommandQueue; 
	static Utils::Logger* s_Instance = nullptr;

	static Utils::Logger::Platform s_Platform; //incase we get different platforms the current method may not work on all
}

namespace Utils
{
	Logger::Logger()
	{
		s_CommandQueue = std::make_unique<Utils::CommandQueue>();
	}

	void Logger::Init(Platform platform)
	{
		if (!s_Instance)
		{
			s_Platform = platform;
			s_Instance = new Logger();
		}
		else
			LOGWARNING("Logger already exists! No need to re init.");
	}

	void Logger::Log(const std::string& prefix, const std::string& msg, const std::string& color)
	{
		std::string message = color + prefix + ": " + msg + "\x1b[0m\n";

		s_CommandQueue->Attach({ [message]() { printf(message.c_str()); }, false });
		
	}

	void Logger::ShutDown()
	{
		s_CommandQueue.release();
	}
	
}