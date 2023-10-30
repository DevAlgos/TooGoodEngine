#pragma once

namespace Utils
{
	enum class Priority { Bottom = 0, Middle, Top };

	template<typename T>
	struct PriorityData
	{
		T data;
		Priority p = Priority::Bottom;
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
			return m_Stack[Index].data;
		}

		void Resize(size_t NewSize);

		void PushBack(const PriorityData<T>& elem); //by default will be at bottom if priority not specified
		void PushBack(PriorityData<T>&& elem);

		void Pop();

		inline T& Front() { return m_Stack[m_Index - 1].data; };
		inline PriorityData<T>& FrontStruct() { return m_Stack[m_Index - 1]; } //incase priority data is needed

	private:
		void MoveStackUp(size_t start); //moves all elements in stack up by 1 from starting point provided
	private:
		PriorityData<T>* m_Stack = nullptr;
		size_t m_Size;
		size_t m_Index = 0;
	};

}