#pragma once

#include <string>
#include <chrono>
#include <thread>
#include <queue>

namespace Utils
{
	enum class LogColor
	{
		Core = 0, Error, Warn
	};

	struct LogData
	{
		std::string LogMessage;
		LogColor MessageColor;
	};

	class Logger
	{
	public:
		enum class Platform { None = 0, Windows };
	public:
		Logger();

		static void Init(Platform platform);
		static void Log(const std::string& prefix, const std::string& msg, const std::string& color);

		static void CoreLog(const std::string_view& Prefix, const std::string_view& Msg, LogColor MsgColor);
		static void CoreLog(const std::string_view& Msg, LogColor MsgColor);
		static bool LogEmpty();

		static void ShutDown();

		static LogData GetFront();
		
	};
}



#define LOG(msg)         Utils::Logger::Log("Logger", msg, "\x1b[35m") //magenta
#define LOGERROR(msg)    Utils::Logger::Log("Error", msg, "\x1b[31m") //red 
#define LOGWARNING(msg)  Utils::Logger::Log("Warning", msg, "\x1b[33m") //yellow 

#define LOG_CORE(msg)			Utils::Logger::CoreLog("[Core] ",	msg,  Utils::LogColor::Core)	
#define LOG_CORE_ERROR(msg)		Utils::Logger::CoreLog("[Error] ",	msg,  Utils::LogColor::Error)	
#define LOG_CORE_WARNING(msg)   Utils::Logger::CoreLog("[Warning] ", msg, Utils::LogColor::Warn)	






