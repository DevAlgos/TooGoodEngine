#pragma once

#include <thread>
#include <vector>
#include <mutex>

static std::mutex mut;

namespace Utils
{
	/*
	 Helper for job class
	*/

	template<class Iterator, class Func>
	static void ForEach(Iterator first, Iterator last, Func work)
	{
		do
		{
			work(*first);
			first++;
		} while (first != last);
	}

	class Job
	{
	public:
		Job();
		Job(size_t DesiredThreads);
		~Job();

		/*
		 Executes function in parallel with arguements in iterator
		*/

		template<class Iterator, class Func>
		void ForEachPar(Iterator first, Iterator last, Func work);

		/*
		 Executes function in parallel with arguements in iterator,
		 Also locks the data when being processed incase it is accessed 
		 by multiple threads.
		*/

		template<class Iterator, class Func>
		void ProtectedForEachPar(Iterator first, Iterator last, Func work);

		void Wait();

		

	private:
		std::vector<std::thread> ThreadPool;
		size_t AvailableThreads;
	};

	template<class Iterator, class Func>
	void Job::ForEachPar(Iterator first, Iterator last, Func work)
	{
		size_t Difference = std::distance(first, last);
		size_t WorkerThreads = static_cast<size_t>(std::min((uint32_t)Difference / AvailableThreads,
			AvailableThreads));

		size_t Step = Difference / WorkerThreads;

		Iterator Current = first;
		for (size_t i = 0; i < WorkerThreads; i++)
		{
			Iterator ThreadBegin = Current;
			std::advance(Current, Step);
			Iterator ThreadEnd = (i == WorkerThreads - 1) ? last : Current;

			ThreadPool.push_back(std::thread([ThreadBegin, ThreadEnd, &work]()
			{
					ForEach(ThreadBegin, ThreadEnd, work);
			}));
		}
	}


	template<class Iterator, class Func>
	void Job::ProtectedForEachPar(Iterator first, Iterator last, Func work)
	{
		size_t Difference = std::distance(first, last);
		size_t WorkerThreads = static_cast<size_t>(std::min((uint32_t)Difference / AvailableThreads, AvailableThreads));
		size_t Step = Difference / WorkerThreads;

		Iterator Current = first;
		for (size_t i = 0; i < WorkerThreads; i++)
		{
			Iterator ThreadBegin = Current;
			std::advance(Current, Step);
			Iterator ThreadEnd = (i == WorkerThreads - 1) ? last : Current;

			ThreadPool.push_back(std::thread([ThreadBegin, ThreadEnd, &work]()
			{
					std::unique_lock<std::mutex> Lock(mut);
					ForEach(ThreadBegin, ThreadEnd, work);
			}));
		}
	}
}