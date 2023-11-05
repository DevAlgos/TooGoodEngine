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
		s_ManagerData.ThreadPool.reserve(s_ManagerData.MaxThreads);
		std::string msg = "Number of threads supported, " + std::to_string(s_ManagerData.MaxThreads);
		LOG(msg);

	}

	void JobManager::AttachWork(size_t ThreadID, const Work& work)
	{
		if (!(ThreadID > s_ManagerData.ThreadPool.size()))
			s_ManagerData.ThreadPool[ThreadID].Thread->Attach(work);
		else
			LOGWARNING("Thread ID out of index, work was not attached!");

	}

	void JobManager::AttachWork(const char* ThreadName, const Work& work)
	{
		for (size_t i = 0; i < s_ManagerData.ThreadPool.size(); i++)
		{
			if (s_ManagerData.ThreadPool[i].ThreadName == ThreadName)
			{
				s_ManagerData.ThreadPool[i].Thread->Attach(work);
				break;
			}
		}
	}

	void JobManager::CreateThread(const char* ThreadName)
	{
		if (s_ManagerData.MaxThreads > s_ManagerData.ThreadPool.size())
			s_ManagerData.ThreadPool.push_back({ThreadName, std::make_unique<Job>()});
		else
			LOGWARNING("Max number of threads supported on computer reached !, thread not created");
	}

	void JobManager::ShutdownManager()
	{
		for (ThreadData& data : s_ManagerData.ThreadPool)
			data.Thread.release(); // join all threads
	}
}