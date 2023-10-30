#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <queue>


namespace Utils
{
	struct Work
	{
		std::function<void()> Func;
		bool MustFinishOnClose;
	};

	class Job
	{
	public:
		Job();
		~Job();

		void Init();

		void Attach(const Work& work);
		void Join();
		void Wait();
		bool IsWorkDone();

		uint32_t GetExecuteCount()
		{
			std::unique_lock<std::mutex> m_JobMutex;
			return m_ExecuteCount;
		}

		const uint16_t GetJobID() const { return JobID; }


	private:
		void Worker();

	private:
		uint16_t JobID;

		uint16_t m_ExecuteCount = 0;

		std::jthread m_JobThread;
		volatile bool m_ThreadActive;

		std::queue<Work> m_WorkQueue;
		std::mutex m_JobMutex;
		std::condition_variable m_JobCondition;
	};
}