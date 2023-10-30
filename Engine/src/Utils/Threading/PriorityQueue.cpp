#include <pch.h>
#include "PriorityQueue.h"

namespace Utils
{
	template<typename T>
	PriorityQueue<T>::PriorityQueue()
		: m_Stack(new PriorityData<T>[10]), m_Size(10)
	{
	}
	template<typename T>
	PriorityQueue<T>::PriorityQueue(size_t Size)
		: m_Stack(new PriorityData<T>[Size]), m_Size(Size)
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
		PriorityData<T>* temp = new PriorityData<T>[NewSize];
		for (size_t i = 0; i < m_Size; i++)
			temp[i] = std::move(m_Stack[i]);

		delete[] m_Stack;
		m_Stack = temp;
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
		while (point > 0 && m_Stack[point - 1].p > elem.p) //keep going until either reach 0th element or priority becomes smaller than elem priority or equal to elem priority
		{
			point--;
		}

		if (m_Stack[point].p < elem.p) // if priority is less then we want to insert elem in front of point
		{
			if (point + 1 > m_Index)
				m_Index = point + 1;

			MoveStackUp(point + 1); //make room for new element
			m_Stack[point + 1] = elem;
		}
		else
		{
			MoveStackUp(point); //make room for new element
			m_Stack[point] = elem;

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
			m_Stack[0] = std::move(elem);
			m_Index++;
		}
		else
		{
			size_t point = m_Index;
			while (point > 0 && m_Stack[point - 1].p > elem.p) //keep going until either reach 0th element or priority becomes smaller than elem priority or equal to elem priority
			{
				point--;
			}


			if (m_Stack[point].p < elem.p) // if priority is less then we want to insert elem in front of point
			{
				MoveStackUp(point); //make room for new element
				m_Stack[point] = std::move(m_Stack[point + 1]);
				m_Stack[point + 1] = std::move(elem);
			}
			else
			{
				MoveStackUp(point); //make room for new element
				m_Stack[point] = std::move(elem);

			}
		}

		elem = nullptr;


	}

	template<typename T>
	void PriorityQueue<T>::MoveStackUp(size_t start)
	{
		if (start >= m_Size) {
			LOGERROR("start is bigger than current size!");
			return;
		}

		if (start == m_Index)
		{
			m_Stack[start] = std::move(m_Stack[start + 1]);
		}
		else
		{
			while (start != m_Index)
			{
				m_Stack[start + 1] = std::move(m_Stack[start]);
				start++;
			}
		}


		m_Index++;
	}

	template<typename T>
	void PriorityQueue<T>::Pop() //Data can be overwritten in the above index if needed, data will also be deleted in destructor if not overwritten
	{
		if (m_Index > 0)
			m_Index--;
	}
}