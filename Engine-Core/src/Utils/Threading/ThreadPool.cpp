#include "ThreadPool.h"

namespace TooGoodEngine {

	ThreadPool::ThreadPool()
	{
		for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
			m_ThreadPool.push_back(std::thread(&ThreadPool::SpawnedThread, this));
	}
	ThreadPool::~ThreadPool()
	{
		{
			std::lock_guard<std::recursive_mutex> LockGuard(m_Mutex);
			m_StopThreads = true;
			m_Condition.notify_all();
		}

		for (auto& Thread : m_ThreadPool)
		{
			if (Thread.joinable())
				Thread.join();
		}
	}
	void ThreadPool::Wait()
	{
		std::unique_lock<std::recursive_mutex> lock(m_Mutex);

		m_Condition.wait(lock, [&]() 
			{
				return m_NumTasks == 0 && m_WorkQueue.empty();
			});

	}
	void ThreadPool::SpawnedThread()
	{
		std::unique_lock<std::recursive_mutex> Lock(m_Mutex);

		while (!m_StopThreads || (m_StopThreads && !m_WorkQueue.empty()))
		{
			m_Condition.wait(Lock, [&]()
				{
					return !m_WorkQueue.empty() || m_StopThreads;
				});

			if (!m_WorkQueue.empty())
			{
				std::function<void()> fun = m_WorkQueue.front();
				m_WorkQueue.pop();
				{
					std::lock_guard<std::recursive_mutex> taskLock(m_Mutex);
					Lock.unlock();  
					fun();   

					m_NumTasks--;
					m_Condition.notify_one();
				}  

				Lock.lock();    
			}
		}
	}
}