#pragma once

#include <chrono>
#include <iostream>
#include "Logger.h"

namespace Utils
{
	enum class TimeUnit {nano=0,micro, mili,second};

	class Clock 
	{
	public: //avoids using namespace std::chrono to avoid confilcting namespaces
		using high_resolution_clock =      std::chrono::high_resolution_clock;
		using nanoseconds           =      std::chrono::nanoseconds;
		using microseconds          =      std::chrono::microseconds;
		using milliseconds          =      std::chrono::milliseconds;
		using seconds               =      std::chrono::seconds;

	public:
		

		Clock();
		~Clock();

		void StartTime();
		void EndTime();
		
		long long TimeElapsed(TimeUnit unit);
	private:
		
		std::chrono::time_point<std::chrono::steady_clock> StartTimePoint;
		std::chrono::time_point<std::chrono::steady_clock> EndTimePoint;

	};
}