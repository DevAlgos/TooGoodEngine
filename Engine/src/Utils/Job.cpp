#include <pch.h>
#include "Job.h"
#include <iostream>

namespace
{
	static size_t IDCounter = 0;
	static Utils::JobManagerData s_ManagerData;
}


namespace Utils
{

	template<typename T>
	inline PriorityQueue<T>::PriorityQueue()
		: m_Stack(new T[10]), m_Size(10)
	{
	}
	template<typename T>
	PriorityQueue<T>::PriorityQueue(size_t Size)
		: m_Stack(new T[Size]), m_Size(Size)
	{
	}


	template<typename T>
	PriorityQueue<T>::~PriorityQueue()
	{
		delete[] m_Stack;
	}

	template<typename T>
	void PriorityQueue<T>::Resize(size_t NewSize)
	{
		T* temp = new T[NewSize];
		temp = m_Stack;
		m_Stack = temp;
		m_Size = NewSize;
	}

	template<typename T>
	void PriorityQueue<T>::PushBack(const T& elem)
	{
		if (m_Index >= m_Size)
		{
			size_t NewSize = m_Size * 1.5;
			Resize(NewSize);
		}

		m_Stack[m_Index] = elem;
		m_Index++;
	}

	template<typename T>
	void PriorityQueue<T>::PushBack(T&& elem)
	{
		if (m_Index >= m_Size)
		{
			size_t NewSize = m_Size * 1.5;
			Resize(NewSize);
		}

		m_Stack[m_Index] = elem;
		m_Index++;
	}

	template<typename T>
	void PriorityQueue<T>::Pop() //Data can be overwritten in the above index if needed
	{
		m_Index--;
	}

	Job::Job()
		: _ThisJobThread(std::jthread(&Job::Worker, this)), m_ThreadActive(true)
	{
		JobID = IDCounter;
		IDCounter++;
	}

	Job::~Job()
	{
		Join();
	}



	void Job::Init()
	{
		m_ThreadActive = true;
		m_Execute = false;
		_ThisJobThread = std::jthread(&Job::Worker, this);

		JobID = IDCounter;
		IDCounter++;
	}


	void Job::Attach(const Work& work)
	{
		if (m_ThreadActive)
		{
			std::unique_lock<std::mutex> lock(m_JobMutex);
			m_WorkQueue.push(work);
			m_JobCondition.notify_one();
			m_ExecuteCount += 1;
		}

	}

	void Job::Join()
	{
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


			while (!m_WorkQueue.empty() && m_ThreadActive)
			{
				ThisFunc = m_WorkQueue.front().Func;

				ThisFunc();
				m_WorkQueue.pop();
				m_JobCondition.notify_one();
				m_ExecuteCount--;
			}

			while (m_WorkQueue.empty() && m_ThreadActive)
				m_JobCondition.wait(lock);

		}


		if (!m_WorkQueue.empty())
		{
			std::function<void()> ThisFunc;
			while (!m_WorkQueue.empty())
			{
				if (m_WorkQueue.front().MustFinishOnClose)
				{
					ThisFunc = m_WorkQueue.front().Func;
					ThisFunc();
				}
				else continue;

				m_WorkQueue.pop();
			}
		}
	}

	void JobManager::InitalizeManager()
	{
		s_ManagerData.ThreadPool.resize(s_ManagerData.NumberOfThreads);

	}

	void JobManager::AttachWork(size_t ThreadID, const Work& work)
	{
	}

	void JobManager::AttachWork(const Work& work)
	{
	}

	void JobManager::ShutdownManager()
	{
	}

	
	
}