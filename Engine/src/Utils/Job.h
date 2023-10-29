#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <queue>


namespace Utils
{
	enum class Priority {None = 0, Low, Middle, Top};
	
	template<typename T>
	class PriorityQueue
	{
	public:
		PriorityQueue();
		PriorityQueue(size_t Size);
		~PriorityQueue();

		void Resize(size_t NewSize);
		
		void PushBack(const T& elem);
		void Pop();

		inline T& Front() { return m_Stack[m_Index]; };


	private:
		T* m_Stack[];
		size_t m_Size;
		size_t m_Index = 0;
	};


	struct Work
	{
		std::function<void()> Func;
		bool MustFinishOnClose;
	};

	struct JobManagerData
	{
		uint32_t NumberOfThreads = std::thread::hardware_concurrency();

		std::vector<Job> ThreadPool;
	};

	class JobManager
	{
	public:
		static void InitalizeManager();

		static void AttachWork(size_t ThreadID, const Work& work);
		static void AttachWork(const Work& work);

		static void ShutdownManager();
	};

	

	class Job
	{
	public:
		//Job();
		//~Job();

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

		std::jthread _ThisJobThread;
		volatile bool m_ThreadActive;
		volatile bool m_Execute;

		std::queue<Work> m_WorkQueue;
		std::mutex m_JobMutex;
		std::condition_variable m_JobCondition;
	};
	
}