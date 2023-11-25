#pragma once

#include <string>
#include <chrono>
#include <thread>
#include <queue>

namespace Utils
{
	class Logger
	{
	public:
		enum class Platform { None = 0, Windows };
	public:
		Logger();

		static void Init(Platform platform);
		static void Log(const std::string& prefix, const std::string& msg, const std::string& color);

		static void ShutDown();
		
	};
}



#define LOG(msg)         Utils::Logger::Log("Logger", msg, "\x1b[35m") //magenta
#define LOGERROR(msg)    Utils::Logger::Log("Error", msg, "\x1b[31m") //red 
#define LOGWARNING(msg)  Utils::Logger::Log("Warning", msg, "\x1b[33m") //yellow 







