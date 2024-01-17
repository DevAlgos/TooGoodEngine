#include <pch.h>
#include "CommandQueue.h"


namespace
{
}

namespace Utils
{
	CommandQueue::CommandQueue()
		: m_CommandQueueThread(std::jthread(&CommandQueue::Worker, this)), m_ThreadActive(true), m_WorkQueue()
	{
	}

	CommandQueue::~CommandQueue()
	{
		{
			std::unique_lock<std::mutex> Lock(m_CommandQueueMutex);
			m_ThreadActive = false;
			m_CommandQueueCondition.notify_all();
		}

		if (m_CommandQueueThread.joinable()) { //not needed because using jthread but still good to have
			m_CommandQueueThread.join();
		}
	}



	void CommandQueue::Init()
	{
		m_ThreadActive = true;
		m_CommandQueueThread = std::jthread(&CommandQueue::Worker, this);
	}


	void CommandQueue::Attach(const Work& work)
	{
		if (m_ThreadActive)
		{
			std::unique_lock<std::mutex> lock(m_CommandQueueMutex);
			m_WorkQueue.push(work);
			m_CommandQueueCondition.notify_one();
			m_ExecuteCount += 1;
		}

	}

	void CommandQueue::Join()
	{
		{
			std::unique_lock<std::mutex> Lock(m_CommandQueueMutex);
			m_ThreadActive = false;
			m_CommandQueueCondition.notify_all();
		}

		if (m_CommandQueueThread.joinable()) { //not needed because using jthread but still good to have
			m_CommandQueueThread.join();
		}

	}
	void CommandQueue::Wait()
	{
		std::unique_lock<std::mutex> Lock(m_CommandQueueMutex);
		m_CommandQueueCondition.wait(Lock, [this]() {return this->m_WorkQueue.empty(); });
	}
	bool CommandQueue::IsWorkDone()
	{
		std::unique_lock<std::mutex> Lock(m_CommandQueueMutex);
		return m_WorkQueue.empty();
	}
	void CommandQueue::Worker()
	{
		while (m_ThreadActive)
		{
			std::unique_lock<std::mutex> lock(m_CommandQueueMutex);
			std::function<void()> ThisFunc;


			while (!m_WorkQueue.empty() && m_ThreadActive)
			{
				ThisFunc = m_WorkQueue.front().Func;

				ThisFunc();
				m_WorkQueue.pop();
				m_CommandQueueCondition.notify_one();
				m_ExecuteCount--;
			}

			while (m_WorkQueue.empty() && m_ThreadActive)
				m_CommandQueueCondition.wait(lock);

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

				m_WorkQueue.pop();
			}
		}
	}
}