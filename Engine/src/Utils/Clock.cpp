#include <pch.h>
#include "Clock.h"

namespace Utils
{
	Clock::Clock()
		: started(true), StartTimePoint(high_resolution_clock::now())
	{
	}
	Clock::~Clock()
	{
	}
	void Clock::StartTime()
	{
		started = true;
		StartTimePoint = high_resolution_clock::now();
	}
	void Clock::EndTime()
	{
		EndTimePoint = high_resolution_clock::now();
	}

	long long Clock::TimeElapsed(TimeUnit unit)
	{
		EndTimePoint = high_resolution_clock::now();
		long long start = 0;
		long long end = 0;

		
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

			LOG(std::to_string(end - start));

		return end - start;
	}

	long long Clock::GetCurrentTime(TimeUnit unit)
	{

		if (!started)
		{
			StartTime(); // StartTimePoint the timer if it hasn't been StartTimePointed
		}

		high_resolution_clock::time_point current = high_resolution_clock::now();

		long long start = 0;
		long long end = 0;

		switch (unit)
		{
		case TimeUnit::nano:
			start = std::chrono::time_point_cast<nanoseconds>(current).time_since_epoch().count();
			end = std::chrono::time_point_cast<nanoseconds>(StartTimePoint).time_since_epoch().count();
			break;
		case TimeUnit::micro:
			start = std::chrono::time_point_cast<microseconds>(current).time_since_epoch().count();
			end = std::chrono::time_point_cast<microseconds>(StartTimePoint).time_since_epoch().count();
			break;
		case TimeUnit::mili:
			start = std::chrono::time_point_cast<milliseconds>(current).time_since_epoch().count();
			end = std::chrono::time_point_cast<milliseconds>(StartTimePoint).time_since_epoch().count();
			break;
		case TimeUnit::second:
			start = std::chrono::time_point_cast<seconds>(current).time_since_epoch().count();
			end = std::chrono::time_point_cast<seconds>(StartTimePoint).time_since_epoch().count();
			break;
		default:
			start = std::chrono::time_point_cast<milliseconds>(current).time_since_epoch().count();
			end = std::chrono::time_point_cast<milliseconds>(StartTimePoint).time_since_epoch().count();
			break;
		}

		return end - start;
	}
	
}
