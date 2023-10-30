#include <pch.h>
#include "ThreadManager.h"


namespace
{
	Utils::JobManagerData s_ManagerData;
}

namespace Utils
{
	void JobManager::InitalizeManager()
	{
		s_ManagerData.ThreadPool.resize(s_ManagerData.NumberOfThreads);

	}

	void JobManager::AttachWork(size_t ThreadID, const Work& work)
	{
	}

	void JobManager::AttachWork(const Work& work)
	{
	}

	void JobManager::ShutdownManager()
	{
	}
}