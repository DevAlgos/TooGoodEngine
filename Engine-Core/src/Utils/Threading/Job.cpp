#include <pch.h>
#include "Job.h"

namespace Utils
{
	Job::Job()
		: AvailableThreads(std::thread::hardware_concurrency())
	{
		ThreadPool.reserve(AvailableThreads);
	}
	Job::Job(size_t DesiredThreads) 
		:  AvailableThreads(std::min(DesiredThreads, static_cast<size_t>(std::thread::hardware_concurrency())))
	{
		ThreadPool.reserve(AvailableThreads);
	}

	Job::~Job()
	{
		for (std::thread& Thread : ThreadPool)
		{
			if (Thread.joinable())
				Thread.join();
		}
	}

	void Job::Wait()
	{
		for (std::thread& Thread : ThreadPool)
		{
			if (Thread.joinable())
				Thread.join();
		}
	}
}