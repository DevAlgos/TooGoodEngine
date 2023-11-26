#include <pch.h>
#include "Logger.h"

namespace
{
	static std::unique_ptr<Utils::Job> s_Job; 
	static Utils::Logger* s_Instance = nullptr;

	static Utils::Logger::Platform s_Platform; //incase we get different platforms the current method may not work on all
}

namespace Utils
{
	Logger::Logger()
	{
		s_Job = std::make_unique<Utils::Job>();
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

		s_Job->Attach({ [message]() { printf(message.c_str()); }, false }, Priority::Bottom);
		
	}

	void Logger::ShutDown()
	{
		s_Job.release();
	}
	
}