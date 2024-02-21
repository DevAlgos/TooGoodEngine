#pragma once

#include "Log.h"
#include "Utils/Layers/EditorLayer.h"

#include <chrono>
#include <iostream>


namespace Utils
{
	enum class TimeUnit {nano=0,micro, mili,second};

	using high_resolution_clock = std::chrono::high_resolution_clock;
	using nanoseconds = std::chrono::nanoseconds;
	using microseconds = std::chrono::microseconds;
	using milliseconds = std::chrono::milliseconds;
	using seconds = std::chrono::seconds;

	class Clock 
	{
	public:
		Clock();
		~Clock();

		void StartTime();
		void EndTime();
		
		int64_t TimeElapsed(TimeUnit unit);
	private:
		
		std::chrono::time_point<std::chrono::steady_clock> StartTimePoint;
		std::chrono::time_point<std::chrono::steady_clock> EndTimePoint;

	};

	class TimedScope
	{
	public:
		TimedScope(const std::string_view& name);
		~TimedScope();

	private:
		std::string_view m_Name;
		std::chrono::high_resolution_clock::time_point m_StartTimePoint;
		std::chrono::high_resolution_clock::time_point m_EndTimePoint;
	};
}