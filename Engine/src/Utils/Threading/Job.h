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

	class Job
	{
	public:
		Job();
		~Job();

		void Init();

		void Attach(const Work& work, const Priority& priority);
		void Join();
		void Wait();
		bool IsWorkDone();

		uint32_t GetExecuteCount()
		{
			std::unique_lock<std::mutex> m_JobMutex;
			return m_ExecuteCount;
		}


	private:
		void Worker();

	private:
		uint16_t m_ExecuteCount = 0;

		std::jthread m_JobThread;
		volatile bool m_ThreadActive;

		Utils::PriorityQueue<Work> m_WorkQueue;
		std::mutex m_JobMutex;
		std::condition_variable m_JobCondition;
	};
}