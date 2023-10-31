#pragma once

#include "Job.h"
#include "PriorityQueue.h"

namespace Utils
{

	struct ThreadData // in the future will contain data such as timings of how long thread takes or memory usuage etc...
	{
		const char* ThreadName;
		std::unique_ptr<Job> Thread;
	};

	/*essage : 'Utils::ThreadData::ThreadData(const Utils::ThreadData &)': 
	function was implicitly deleted because a data member invokes a deleted or 
	inaccessible function '
	std::unique_ptr<Utils::Job,
	std::default_delete<Utils::Job>>::
	unique_ptr(const 
	std::unique_ptr<Utils::Job,std::
	default_delete<Utils::Job>> &)*/

	struct JobManagerData
	{
		uint32_t MaxThreads = std::thread::hardware_concurrency() - 1; //1 thread reserved for logger

		std::vector<ThreadData> ThreadPool;
		size_t ThreadIndex = 0;
	};

	class JobManager
	{
	public:
		static void InitalizeManager();

		static void AttachWork(size_t ThreadID, const Work& work, 
			const Priority& priority);
		static void AttachWork(const char* ThreadName, const Work& work,
			const Priority& priority);

		static void CreateThread(const char* ThreadName);

		static void ShutdownManager();
	};
}