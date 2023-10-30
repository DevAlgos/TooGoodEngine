#pragma once

#include "Job.h"

namespace Utils
{
	struct JobManagerData
	{
		uint32_t NumberOfThreads = std::thread::hardware_concurrency();

		std::vector<std::unique_ptr<Job>> ThreadPool;
	};

	class JobManager
	{
	public:
		static void InitalizeManager();

		static void AttachWork(size_t ThreadID, const Work& work);
		static void AttachWork(const Work& work);

		static void ShutdownManager();
	};
}