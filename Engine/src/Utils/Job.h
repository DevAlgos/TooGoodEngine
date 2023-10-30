#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <queue>


namespace Utils
{	
	enum class Priority { Bottom = 0, Middle, Top };

	template<typename T>
	struct PriorityData
	{
		Priority p = Priority::Bottom;
		T data;
	};

	template<typename T>
	class PriorityQueue
	{
	public:
		PriorityQueue();
		PriorityQueue(size_t Size);
		~PriorityQueue();

		T& operator[](size_t Index)
		{
			return m_Stack[Index];
		}



		void Resize(size_t NewSize);

		void PushBack(const T& elem);
		void PushBack(T&& elem);


		void PushBack(const PriorityData<T>& elem);
		void PushBack(PriorityData<T>&& elem);

		void Pop();

		inline T& Front() { return m_Stack[m_Index - 1]; };


	private:
		PriorityData<T>* m_Stack;
		size_t m_Size;
		size_t m_Index = 0;
	};


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

		std::jthread _ThisJobThread;
		volatile bool m_ThreadActive;
		volatile bool m_Execute;

		std::queue<Work> m_WorkQueue;
		std::mutex m_JobMutex;
		std::condition_variable m_JobCondition;
	};

	struct JobManagerData
	{
		uint32_t NumberOfThreads = std::thread::hardware_concurrency();

		std::vector<std::unique_ptr<Job>> ThreadPool;
	};

	class JobManager
	{
	public:
		static void InitalizeManager();

		static void AttachWork(size_t ThreadID, const Work& work);
		static void AttachWork(const Work& work);

		static void ShutdownManager();
	};

	

	
	
}