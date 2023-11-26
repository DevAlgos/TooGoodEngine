#pragma once

#include <Utils/Logger.h>

namespace Utils
{
	enum class Priority { Bottom = 0, Middle, Top };

	template<typename T>
	struct PriorityData
	{
		T data;
		Priority p;

		PriorityData() : p(Priority::Bottom)
		{}

		PriorityData(T d, Priority priority) : data(d), p(priority)
		{}

		~PriorityData()
		{}
	};

	template<typename T>
	class PriorityQueue
	{
	public:
		constexpr PriorityQueue();
		PriorityQueue(size_t Size);
		~PriorityQueue();

		T& operator[](size_t Index) noexcept
		{
			if (Index > m_Index || Index < 0)
				return nullptr; //Is index within range?
			return m_Queue[Index].data;
		}

		void Resize(size_t NewSize);

		void PushBack(const PriorityData<T>& elem); //by default will be at bottom if priority not specified
		void PushBack(PriorityData<T>&& elem);

		void Pop() noexcept;

		inline T& Front() noexcept { return m_Queue[m_Index - 1].data; };
		inline PriorityData<T>& FrontStruct() noexcept { return m_Queue[m_Index - 1]; } //incase we need to check priority
		inline bool IsEmpty() noexcept { return m_Index == 0; }

	private:
		void MoveQueue(size_t start); //moves all elements in stack up by 1 from starting point provided
	private:
		PriorityData<T>* m_Queue = nullptr;
		size_t m_Size;
		size_t m_Index = 0;
	};

	template<typename T>
	constexpr PriorityQueue<T>::PriorityQueue()
		: m_Queue(new PriorityData<T>[10]), m_Size(10)
	{
	}
	template<typename T>
    PriorityQueue<T>::PriorityQueue(size_t Size)
		: m_Queue(new PriorityData<T>[Size]), m_Size(Size)
	{
	}


	template<typename T>
	PriorityQueue<T>::~PriorityQueue()
	{
		delete[] m_Queue;
	}

	template<typename T>
	void PriorityQueue<T>::Resize(size_t NewSize)
	{
		PriorityData<T>* temp = new PriorityData<T>[NewSize];
		for (size_t i = 0; i < m_Size; i++)
			temp[i] = std::move(m_Queue[i]);

		delete[] m_Queue;
		m_Queue = temp;
		m_Size = NewSize;
	}

	template<typename T>
	void PriorityQueue<T>::PushBack(const PriorityData<T>& elem)
	{
		if (m_Index >= m_Size)
		{
			size_t NewSize = m_Size * 1.5;
			Resize(NewSize);
		}

		size_t point = m_Index;
		while (point > 0 && m_Queue[point - 1].p > elem.p) //keep going until either reach 0th element or priority becomes smaller than elem priority or equal to elem priority
		{
			point--;
		}

		if (m_Queue[point].p < elem.p) // if priority is less then we want to insert elem in front of point
		{
			if (point + 1 > m_Index)
				m_Index = point + 1;

			MoveQueue(point + 1); //make room for new element
			m_Queue[point + 1] = elem;
		}
		else
		{
			MoveQueue(point); //make room for new element
			m_Queue[point] = elem;

		}

	}

	template<typename T>
	void PriorityQueue<T>::PushBack(PriorityData<T>&& elem)
	{
		
		if (m_Index >= m_Size)
		{
			size_t NewSize = m_Size * 2;
			Resize(NewSize);
		}

		if (m_Index == 0)
		{
			m_Queue[0] = std::move(elem);
			m_Index++;
		}
		else
		{
			size_t point = m_Index;
			while (point > 0 && m_Queue[point - 1].p > elem.p) //keep going until either reach 0th element or priority becomes smaller than elem priority or equal to elem priority
			{
				point--;
			}


			if (m_Queue[point].p < elem.p) // if priority is less then we want to insert elem in front of point
			{
				MoveQueue(point); //make room for new element
				m_Queue[point] = std::move(m_Queue[point + 1]);
				m_Queue[point + 1] = std::move(elem);
			}
			else
			{
				MoveQueue(point); //make room for new element
				m_Queue[point] = std::move(elem);

			}
		}


	}

	template<typename T>
	void PriorityQueue<T>::MoveQueue(size_t start)
	{
		if (start >= m_Size) {
			LOGERROR("start is bigger than current size!");
			return;
		}

		if (start == m_Index)
		{
			m_Queue[start] = std::move(m_Queue[start + 1]);
		}
		else
		{
			while (start != m_Index)
			{
				m_Queue[start + 1] = std::move(m_Queue[start]);
				start++;
			}
		}


		m_Index++;
	}

	template<typename T>
	void PriorityQueue<T>::Pop() noexcept 
	{
		if (m_Index > 0)
			m_Index--;

		m_Queue[m_Index].~PriorityData<T>();
	}

}