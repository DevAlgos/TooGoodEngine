#pragma once

#include <map>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <memory>

#include "Entity.h"
#include "Bucket.h"


namespace Ecs
{	
	class Registry
	{
	public:
		Registry();
		~Registry();

		template<class Type>
		bool HasComponent(Ecs::Entity Entity)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
				return m_SparseSet[DataType].HasComponent(Entity);
			else
				return false;
		}


		template<class Type, class Func>
		void View(Func func)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
				m_SparseSet[DataType].ForEach<Type>(func);	
		}

		template<class Type>
		Type* Get(Ecs::Entity Entity)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
				return m_SparseSet[DataType].GetComponent<Type>(Entity);

			LOG_CORE_ERROR("Not a valid type" + std::string(DataType.name()));
			assert(false);
			return nullptr;
		}

		template<class Type>
		Type& GetRef(Ecs::Entity Entity)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
				return m_SparseSet[DataType].GetComponentRef<Type>(Entity);

			LOG_CORE_ERROR("Not a valid type" + std::string(DataType.name()));
			assert(false);
		}

		template<class Type, typename ...Args>
		void Emplace(Ecs::Entity Entity, Args&&... args)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
			{
				m_SparseSet[DataType].PushComponent<Type>(Entity, std::forward<Args>(args)...);
			}
			else
			{
				m_SparseSet[DataType] = Bucket(DataType);
				m_SparseSet[DataType].PushComponent<Type>(Entity, std::forward<Args>(args)...);
			}
		}

		template<class Type>
		void Delete(Ecs::Entity Entity)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
				m_SparseSet[DataType].DeleteComponent<Type>(Entity);
		}

	private:
		std::unordered_map<std::type_index, Bucket> m_SparseSet;
	};
}