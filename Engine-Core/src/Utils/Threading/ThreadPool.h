#pragma once

#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

#include <vector>
#include <queue>

namespace TooGoodEngine {

	class ThreadPool
	{
	public:
		ThreadPool();
		~ThreadPool();

		void Wait();

		template<typename Fun, typename ...Args>
		void AddTask(Fun&& fun, Args&&... args);

	private:
		void SpawnedThread();

	private:
		mutable std::recursive_mutex m_Mutex;
		
		std::vector<std::thread> m_ThreadPool;
		std::condition_variable_any m_Condition;

		std::queue<std::function<void()>> m_WorkQueue;

		bool m_StopThreads = false;
		uint32_t m_NumTasks = 0;
	};

	template<typename Fun, typename ...Args>
	inline void ThreadPool::AddTask(Fun&& fun, Args&& ...args)
	{
		auto Work = [func = std::forward<Fun>(fun), 
					 arguments = std::make_tuple(std::forward<Args>(args)...)]() mutable 
			{
				std::apply(std::move(func), std::move(arguments));
			};

		{
			std::lock_guard<std::recursive_mutex> LockGuard(m_Mutex);
			m_WorkQueue.push(Work);
			m_NumTasks++;
			m_Condition.notify_one();
		}
	}

}