#pragma once

#include <map>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <memory>

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
		Type& Get(Ecs::Entity Entity)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
				return m_SparseSet[DataType].Get<Type>(Entity);

			LOG_CORE_ERROR("Not a valid type" + std::string(DataType.name()));
			assert(false);
		}

		template<class Type>
		Type* Begin() 
		{ 
			if (!m_SparseSet.contains(typeid(Type)))
				return nullptr;
			return m_SparseSet[typeid(Type)].BeginComponent<Type>(); 
		}

		template<class Type>
		Type* End() 
		{
			if (!m_SparseSet.contains(typeid(Type)))
				return nullptr;
			return m_SparseSet[typeid(Type)].EndComponent<Type>(); 
		}


		template<class Type, typename ...Args>
		void Insert(Ecs::Entity Entity, Args&&... args)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
			{
				m_SparseSet[DataType].Insert<Type>(Entity, std::forward<Args>(args)...);
			}
			else
			{
				m_SparseSet[DataType] = Bucket(DataType);
				m_SparseSet[DataType].Insert<Type>(Entity, std::forward<Args>(args)...);
			}
		}

		Ecs::Entity CreateEntity(std::string_view Name)
		{
			return Ecs::Entity(Name, m_EntityCount++);
		}

		template<class Type>
		void Delete(Ecs::Entity Entity)
		{
			std::type_index DataType = typeid(Type);
			if (m_SparseSet.contains(DataType))
				m_SparseSet[DataType].Delete<Type>(Entity);
		}

	private:
		std::unordered_map<std::type_index, Bucket> m_SparseSet;
		EntityID m_EntityCount = 0;
	};
}