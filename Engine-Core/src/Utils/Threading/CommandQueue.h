#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <queue>

#include "PriorityQueue.h"


namespace Utils
{
	struct Work
	{
		std::function<void()> Func;
		bool MustFinishOnClose = false;
	};

	class CommandQueue
	{
	public:
		CommandQueue();
		~CommandQueue();

		void Init();

		void Attach(const Work& work);
		void Join();
		void Wait();
		bool IsWorkDone();

		uint32_t GetExecuteCount()
		{
			std::unique_lock<std::mutex> m_CommandQueueMutex;
			return m_ExecuteCount;
		}


	private:
		void Worker();

	private:
		uint16_t m_ExecuteCount = 0;

		std::jthread m_CommandQueueThread;
		volatile bool m_ThreadActive;

		std::queue<Work> m_WorkQueue;
		std::mutex m_CommandQueueMutex;
		std::condition_variable m_CommandQueueCondition;
	};
}