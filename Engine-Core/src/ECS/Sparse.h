#pragma once

#include <typeindex>
#include <vector>
#include <cassert>

#include <xhash>
#include <unordered_map>
#include <Utils/Logger.h>

namespace Ecs
{
	struct NullType
	{
		void* type = nullptr;
	};



	class ComponentAllocator {
	public:
		ComponentAllocator() : m_Block(nullptr), m_ComponentType(typeid(NullType)) {}

		template<class Type>
		void Allocate(uint64_t Size)
		{
			m_ComponentType = typeid(Type);

			if (m_Capacity >= Size)
				return;

			void* TempBlock = calloc(Size, sizeof(Type));

			if (!TempBlock)
				return;

			if (m_Block)
				free(m_Block);

			m_Block = TempBlock;
			m_Capacity = Size;
		}

		template<class Type>
		void ReAllocate(uint64_t Size)
		{
			if (m_ComponentType == typeid(NullType))
				m_ComponentType = typeid(Type);

			if (m_ComponentType != typeid(Type))
				return; //incorrect type

			void* TempBlock = realloc(m_Block, sizeof(Type) * Size);

			if (!TempBlock)
			{
				assert(false);
				return;
			}


			m_Block = TempBlock;
			m_Capacity = Size;
		}

		template<class Type, typename ... Args>
		std::enable_if_t<std::is_constructible_v<Type, Args...>, void> Construct(uint64_t Index, Args&&... args)
		{
			if (Index >= m_Capacity)
				ReAllocate<Type>((m_Capacity + (Index - m_Capacity)) * 2);

			assert(m_ComponentType == typeid(Type));

			Type* ptr = static_cast<Type*>(m_Block) + Index;
			*ptr = Type(std::forward<Args>(args)...);
			m_Size++;
		}

		template<class Type, typename ...Args>
		std::enable_if_t<!std::is_constructible_v<Type, Args...>, void> Construct(uint64_t Index, Args&&... args)
		{
			printf("no suitable constructor!");
		}

		template<class Type>
		Type& Get(uint64_t Index)
		{
			Type* ptr = (static_cast<Type*>(m_Block) + Index);

			if (ptr)
				return *ptr;

			assert(false);
		}

		template<class Type>
		Type* Begin() { return static_cast<Type*>(m_Block); }

		template<class Type>
		Type* End() { return static_cast<Type*>(m_Block) + m_Size; }

		//TODO: change from shifting elements to swapping this 
		template<class Type>
		void Delete(uint64_t Index)
		{
			Type* ptr = (static_cast<Type*>(m_Block) + Index);

			if (ptr >= static_cast<Type*>(m_Block) && ptr < static_cast<Type*>(m_Block) + m_Capacity)
			{
				ptr->~Type();

				/*Type* Next = ptr + 1;
				for (uint64_t i = Index + 1; i < m_Capacity; i++)
				{
					*ptr = std::move(*Next);
					ptr++;
					Next++;
				}*/

				Type* Last = static_cast<Type*>(m_Block) + m_Size - 1;

				if (ptr != Last) 
					std::swap(*ptr, *Last);
				
				m_Capacity--;
				m_Size--;
			}
		}

		void Clear()
		{
			memset(m_Block, 0, m_Capacity);
		}

		const uint64_t GetCapacity() const { return m_Capacity; }
		const uint64_t GetSize() const { return m_Size; }

		~ComponentAllocator()
		{
			if (m_Block)
				free(m_Block);
		}

	private:
		void* m_Block = nullptr;
		uint64_t m_Capacity = 0;
		uint64_t m_Size = 0;
		std::type_index m_ComponentType;
	};

	constexpr uint64_t NoComponent = UINT64_MAX;

	class SparseSet
	{
	public:
		SparseSet() : m_NoElements(0), m_IndexToEntityMap()
		{
		}

		~SparseSet()
		{
		}

		template<class Type, typename ...Args>
		void Insert(uint64_t EntityID, Args&&... args)
		{
			if (m_Sparse.size() <= EntityID)
				m_Sparse.resize(m_Sparse.size() + (EntityID - m_Sparse.size() + 1) * 2, NoComponent);

			if (!m_Allocated)
				m_Dense.Allocate<Type>((EntityID + 1) * 2);

			m_Dense.Construct<Type>(m_CurrentTop, std::forward<Args>(args)...);
			m_Sparse[EntityID] = m_CurrentTop;

			m_IndexToEntityMap[m_CurrentTop] = EntityID;

			m_CurrentTop++;
			m_NoElements++;
			m_Allocated = true;
		}

		template<class Type>
		Type& GetDirect(uint64_t Index)
		{
			if (m_Dense.GetCapacity() <= Index)
				__debugbreak();

			return m_Dense.Get<Type>(Index);
		}

		template<class Type>
		Type& Get(uint64_t EntityID)
		{
			if (m_Sparse.size() <= EntityID)
				__debugbreak();

			if (m_Sparse[EntityID] == NoComponent)
			{
				LOG_CORE_ERROR("that component is invalid");
				__debugbreak();
			}

			return m_Dense.Get<Type>(m_Sparse[EntityID]);
		}

		template<class Type>
		Type* BeginDense() { return m_Dense.Begin<Type>(); }

		template<class Type>
		Type* EndDense() { return m_Dense.End<Type>(); }

		template<class Type>
		void Delete(uint64_t EntityID)
		{
			if (m_Sparse.size() <= EntityID)
				return;

			//Delete component
			m_Dense.Delete<Type>(m_Sparse[EntityID]);

			//Get the current top component and set the index to the componenet at the top to the deleted component
			uint64_t LastEntityID = m_IndexToEntityMap[m_CurrentTop - 1];
			m_Sparse[LastEntityID] = m_Sparse[EntityID];

			//mark as having no component
			m_Sparse[EntityID] = NoComponent;


			/*for (uint64_t CurrentIndex = EntityID + 1; CurrentIndex < m_Sparse.size(); CurrentIndex++)
				m_Sparse[CurrentIndex]--;*/

			//decrement number of elemetns and the current top
			m_NoElements--;
			m_CurrentTop--;
		}

		void Clear()
		{
			m_Dense.Clear();
			m_Sparse.clear();
			m_Sparse.resize(m_Sparse.capacity(), NoComponent);
			m_IndexToEntityMap.clear();
		}

		bool HasComponent(uint64_t EntityID) { return EntityID >= m_Sparse.size() || m_Sparse[EntityID] == NoComponent ? false : true; }

		const uint64_t GetNoElements() const { return m_NoElements; }
		const uint64_t GetSparseSize() const { return m_Sparse.size(); }

	private:
		ComponentAllocator m_Dense;
		std::vector<uint64_t> m_Sparse;

		uint64_t m_NoElements = 0;
		uint64_t m_CurrentTop = 0;
		bool m_Allocated = false;

		std::unordered_map<uint64_t, uint64_t> m_IndexToEntityMap;
	};
}