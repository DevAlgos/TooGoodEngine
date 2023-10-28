#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <queue>


namespace Utils
{

	class Job
	{
	public:
		//Job();
		//~Job();

		void Init();

		void Attach(std::function<void()> Work);
		void Join();
		void Wait();
		bool IsWorkDone();

		uint32_t GetExecuteCount()
		{
			std::unique_lock<std::mutex> m_JobMutex;
			return m_ExecuteCount;
		}

		const int8_t GetMaxJobCount() const { return MaxJobCount; }
		const uint16_t GetJobID() const { return JobID; }


	private:
		void Worker();

	private:
		const int8_t MaxJobCount = 10;
		uint16_t JobID;

		uint16_t m_ExecuteCount = 0;

		std::jthread _ThisJobThread;
		bool m_ThreadActive;
		bool m_Execute;

		std::queue<std::function<void()>> m_WorkQueue;
		std::mutex m_JobMutex;
		std::condition_variable m_JobCondition;
	};
}