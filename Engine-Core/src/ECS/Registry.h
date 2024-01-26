#pragma once

#include "Bucket.h"

#include <map>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <memory>




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
			if (m_TypeSet.contains(DataType))
				return m_TypeSet[DataType].HasComponent(Entity);
			else
				return false;
		}


		template<class Type, class Func>
		void View(Func func)
		{
			std::type_index DataType = typeid(Type);
			if (m_TypeSet.contains(DataType))
				m_TypeSet[DataType].ForEach<Type>(func);
		}

		template<class Type>
		Type& Get(Ecs::Entity Entity)
		{
			std::type_index DataType = typeid(Type);
			if (m_TypeSet.contains(DataType))
				return m_TypeSet[DataType].Get<Type>(Entity);

			TGE_CLIENT_ERROR("Not a valid type", DataType.name());
			TGE_HALT();
		}

		template<class Type>
		EntityID GetEntityFromComponent(uint64_t Index)
		{
			std::type_index DataType = typeid(Type);
			if (m_TypeSet.contains(DataType))
				return m_TypeSet[DataType].GetEntityFromComponent<Type>(Index);

			TGE_CLIENT_ERROR("Not a valid type", DataType.name());
			return NullEntity;
		}


		template<class Type>
		Type* Begin() 
		{ 
			if (!m_TypeSet.contains(typeid(Type)))
				return nullptr;

			return m_TypeSet[typeid(Type)].BeginComponent<Type>();
		}

		template<class Type>
		Type* End() 
		{
			if (!m_TypeSet.contains(typeid(Type)))
				return nullptr;

			return m_TypeSet[typeid(Type)].EndComponent<Type>();
		}


		template<class Type, typename ...Args>
		void Insert(Ecs::Entity Entity, Args&&... args)
		{
			std::type_index DataType = typeid(Type);
			if (m_TypeSet.contains(DataType))
			{
				m_TypeSet[DataType].Insert<Type>(Entity, std::forward<Args>(args)...);
			}
			else
			{
				m_TypeSet[DataType] = Bucket(DataType);
				m_TypeSet[DataType].Insert<Type>(Entity, std::forward<Args>(args)...);
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
			if (m_TypeSet.contains(DataType))
				m_TypeSet[DataType].Delete<Type>(Entity);
		}

	private:
		std::unordered_map<std::type_index, Bucket> m_TypeSet;
		EntityID m_EntityCount = 0;
	};
}