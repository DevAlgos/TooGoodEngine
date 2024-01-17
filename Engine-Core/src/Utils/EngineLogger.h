#pragma once

#include <sstream>
#include <string>
#include <queue>
#include <iostream>

#include <imgui.h>

#include "Threading/CommandQueue.h"


namespace TooGoodEngine {

	namespace TextColor {
		static constexpr std::string RESET = "\033[0m";
		static constexpr std::string BLACK = "\033[0;30m";
		static constexpr std::string RED = "\033[0;31m";
		static constexpr std::string GREEN = "\033[0;32m";
		static constexpr std::string YELLOW = "\033[0;33m";
		static constexpr std::string BLUE = "\033[0;34m";
		static constexpr std::string MAGENTA = "\033[0;35m";
		static constexpr std::string CYAN = "\033[0;36m";
		static constexpr std::string WHITE = "\033[0;37m";
	}

	class EditorLayer;

	enum class LogColor
	{
		Core = 0, Error, Warn
	};

	struct LogData
	{
		std::string LogMessage;
		LogColor MessageColor;
	};

	class EngineLogger
	{
	public:
		EngineLogger();
		~EngineLogger() = default;
		
		template<class ...Args>
		void Log(const std::string& color, Args&&... args);
		
		template<class ...Args>
		void ClientLog(const LogColor& color, Args&&... args);

		void DisplayLogToImGui();

	private:
		std::vector<LogData> m_MessagesToDraw;
		std::queue<LogData> m_CoreLogQueue;

		ImVec4 CoreColor = { 1.0f, 0.0f, 1.0f, 1.0f };
		ImVec4 ErrorColor = { 1.0f, 0.0f, 0.0f, 1.0f };
		ImVec4 WarnColor = { 1.0f, 1.0f, 0.0f, 1.0f };

		bool m_ScrollToBottom = false;
		bool m_ScrollToBottomCheckBox = true;
		bool m_LogOpen = true;

		friend class EditorLayer;
	};

	template<class ...Args>
	inline void EngineLogger::Log(const std::string& color, Args && ...args)
	{
		std::cout << color;
		(std::cout << ... << std::forward<Args>(args));
		std::cout << TextColor::RESET << std::endl;
	}
	template<class ...Args>
	inline void EngineLogger::ClientLog(const LogColor& color, Args && ...args)
	{
		LogData logData;
		logData.MessageColor = color;

		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));

		logData.LogMessage = oss.str();

		m_CoreLogQueue.push(logData);
	}
}