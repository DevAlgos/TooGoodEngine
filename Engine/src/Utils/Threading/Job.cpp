#include <pch.h>
#include "Job.h"
#include "PriorityQueue.h"


namespace
{
}

namespace Utils
{
	Job::Job()
		: m_JobThread(std::jthread(&Job::Worker, this)), m_ThreadActive(true), m_WorkQueue()
	{
	}

	Job::~Job()
	{
		{
			std::unique_lock<std::mutex> Lock(m_JobMutex);
			m_ThreadActive = false;
			m_JobCondition.notify_all();
		}

		if (m_JobThread.joinable()) { //not needed because using jthread but still good to have
			m_JobThread.join();
		}
	}



	void Job::Init()
	{
		m_ThreadActive = true;
		m_JobThread = std::jthread(&Job::Worker, this);
	}


	void Job::Attach(const Work& work)
	{
		if (m_ThreadActive)
		{
			std::unique_lock<std::mutex> lock(m_JobMutex);
			m_WorkQueue.push(work);
			m_JobCondition.notify_one();
			m_ExecuteCount += 1;
		}

	}

	void Job::Join()
	{
		{
			std::unique_lock<std::mutex> Lock(m_JobMutex);
			m_ThreadActive = false;
			m_JobCondition.notify_all();
		}

		if (m_JobThread.joinable()) { //not needed because using jthread but still good to have
			m_JobThread.join();
		}

	}
	void Job::Wait()
	{
		std::unique_lock<std::mutex> Lock(m_JobMutex);
		m_JobCondition.wait(Lock, [this]() {return this->m_WorkQueue.empty(); });
	}
	bool Job::IsWorkDone()
	{
		std::unique_lock<std::mutex> Lock(m_JobMutex);
		return m_WorkQueue.empty();
	}
	void Job::Worker()
	{
		while (m_ThreadActive)
		{
			std::unique_lock<std::mutex> lock(m_JobMutex);
			std::function<void()> ThisFunc;


			while (!m_WorkQueue.empty() && m_ThreadActive)
			{
				ThisFunc = m_WorkQueue.front().Func;

				ThisFunc();
				m_WorkQueue.pop();
				m_JobCondition.notify_one();
				m_ExecuteCount--;
			}

			while (m_WorkQueue.empty() && m_ThreadActive)
				m_JobCondition.wait(lock);

		}


		if (!m_WorkQueue.empty()) //any remaining functions that have not finished and must will be ran here
		{
			std::function<void()> ThisFunc;
			while (!m_WorkQueue.empty())
			{
				if (m_WorkQueue.front().MustFinishOnClose)
				{
					ThisFunc = m_WorkQueue.front().Func;
					ThisFunc();
				}
				else continue;

				m_WorkQueue.pop();
			}
		}
	}
}