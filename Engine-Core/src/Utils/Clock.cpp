#include <pch.h>
#include "Clock.h"

namespace Utils
{
	Clock::Clock()
		: StartTimePoint(high_resolution_clock::now())
	{
	}
	Clock::~Clock()
	{
	}
	void Clock::StartTime()
	{
		StartTimePoint = high_resolution_clock::now();
	}
	void Clock::EndTime()
	{
		EndTimePoint = high_resolution_clock::now();
	}

	int64_t Clock::TimeElapsed(TimeUnit unit)
	{
		EndTimePoint = high_resolution_clock::now();
		int64_t start = 0;
		int64_t end = 0;

		
		switch (unit)
		{
		case TimeUnit::nano:
			start = std::chrono::time_point_cast<nanoseconds>(StartTimePoint).time_since_epoch().count();
			end = std::chrono::time_point_cast<nanoseconds>(EndTimePoint).time_since_epoch().count();

			break;
		case TimeUnit::micro:
			start = std::chrono::time_point_cast<microseconds>(StartTimePoint).time_since_epoch().count();
			end = std::chrono::time_point_cast<microseconds>(EndTimePoint).time_since_epoch().count();

			break;
		case TimeUnit::mili:
			start = std::chrono::time_point_cast<milliseconds>(StartTimePoint).time_since_epoch().count();
			end = std::chrono::time_point_cast<milliseconds>(EndTimePoint).time_since_epoch().count();


			break;
		case TimeUnit::second:
			start = std::chrono::time_point_cast<seconds>(StartTimePoint).time_since_epoch().count();
			end = std::chrono::time_point_cast<seconds>(EndTimePoint).time_since_epoch().count();

			break;
		default:
			start = std::chrono::time_point_cast<milliseconds>(StartTimePoint).time_since_epoch().count();
			end = std::chrono::time_point_cast<milliseconds>(EndTimePoint).time_since_epoch().count();
			break;
		}

		return end - start;
	}	
}
