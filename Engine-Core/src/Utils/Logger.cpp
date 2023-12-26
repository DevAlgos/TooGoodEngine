#include <pch.h>
#include "Logger.h"

namespace
{
	static std::unique_ptr<Utils::CommandQueue> s_CommandQueue; 
	static Utils::Logger* s_Instance = nullptr;

	static Utils::Logger::Platform s_Platform; //incase we get different platforms the current method may not work on all
	static std::queue<Utils::LogData> MessageQueue;
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

	void Logger::CoreLog(const std::string_view& Prefix, const std::string_view& Msg, LogColor MsgColor)
	{
		MessageQueue.push({ std::string(Prefix) + std::string(Msg), MsgColor });
	}

	void Logger::CoreLog(const std::string_view& Msg, LogColor MsgColor)
	{
		MessageQueue.push({ Msg.data(), MsgColor});
	}

	bool Logger::LogEmpty()
	{
		return MessageQueue.empty();
	}

	void Logger::ShutDown()
	{
		s_CommandQueue.release();
	}

	LogData Logger::GetFront()
	{
		if (MessageQueue.empty())
			return {};

		LogData data = MessageQueue.front();
		MessageQueue.pop();
		return data;
	}
	
}