#include <pch.h>
#include "Job.h"
#include <iostream>

namespace
{
	static size_t IDCounter = 0;
}


namespace Utils
{
	void Job::Init()
	{
		m_ThreadActive = true;
		m_Execute = false;
		_ThisJobThread = std::jthread(&Job::Worker, this);

		JobID = IDCounter;
		IDCounter++;
	}


	void Job::Attach(std::function<void()> Work)
	{
		std::unique_lock<std::mutex> lock(m_JobMutex);
		m_WorkQueue.push(Work);
		m_JobCondition.notify_one();
		m_ExecuteCount += 1;
	}

	
	void Job::Join()
	{
		while (GetExecuteCount() != 0 && !m_ThreadActive)
		{
			//wait until it equals 0
		}

		{
			std::unique_lock<std::mutex> Lock(m_JobMutex);
			m_ThreadActive = false;
			m_JobCondition.notify_all();
		}
		if (_ThisJobThread.joinable()) { //not needed because using jthread but still good to have
			_ThisJobThread.join();
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


			while (m_WorkQueue.empty() && m_ThreadActive)
				m_JobCondition.wait(lock);

			while (!m_WorkQueue.empty())
			{
				//std::cout << "This is a thread" << std::endl;
				ThisFunc = m_WorkQueue.front();

				ThisFunc();
				m_WorkQueue.pop();
				m_JobCondition.notify_one();
				m_ExecuteCount--;
				if (m_ExecuteCount == 0 && m_ThreadActive)
				{
					m_Execute = false;
					m_ThreadActive = true;

					continue;
				}
				else
				{
					m_ThreadActive = false;
					break;
				}
			}
		}
	}
}