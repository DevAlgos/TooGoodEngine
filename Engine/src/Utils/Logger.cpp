#include <pch.h>
#include "Logger.h"

namespace
{
	static Utils::Job s_Job;
	static Utils::Logger* s_Instance = nullptr;

	static Utils::Logger::Platform s_Platform;
}

namespace Utils
{
	Logger::Logger()
	{
		s_Job.Init();
	}

	void Logger::Init(Platform platform)
	{
		if (!s_Instance)
		{
			s_Platform = platform;
			s_Instance = Create();
		}
		else
			LOGWARNING("Logger already exists! No need to re init.");
	}

	void Logger::Log(const std::string& prefix, const std::string& msg, const std::string& color)
	{
		std::string message = color + prefix + ": " + msg + "\x1b[0m\n";

		s_Job.Attach([message]() { printf(message.c_str()); });
		
	}

	void Logger::ShutDown()
	{
		s_Job.Join();
	}

	Logger* Logger::Create()
	{
		switch (s_Platform)
		{
		case Utils::Logger::Platform::None:
			std::cout << "Platform not supported yet!" << std::endl;
			return nullptr;
			break;
		case Utils::Logger::Platform::Windows: // "Once I would like to branch out from Windows this Logger class will become virtual and new seperate Classes that inherit this will take over"
			return new Logger();
			break;
		default:
			return nullptr;
			break;
		}
	}
	
}