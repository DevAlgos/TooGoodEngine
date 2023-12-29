#pragma once

#include "Component.h"
#include <typeindex>
#include <optional>

namespace Ecs
{
	class Bucket
	{
	public:
		Bucket(std::type_index TypeIndex);
		Bucket();
		~Bucket();

		template<class Type, class Func>
		void ForEach(Func func)
		{
			for (size_t i = 0; i < m_ComponentsList.size(); i++)
			{
				if (!m_ComponentsList[i].GetRaw())
					continue;

				func(m_ComponentsList[i].Get<Type>());
			}
		}

		template<class Type>
		Type* GetComponent(Ecs::Entity Entity)
		{
			std::type_index typeIndex = typeid(Type);
			if (typeIndex == m_BucketType && Entity.GetID() < m_ComponentsList.size()) {
				auto component = m_ComponentsList[Entity.GetID()].Get<Type>();
				return component;
			}
			else {
				return nullptr;
			}
		}

		template<class Type, typename ...Args>
		void PushComponent(Ecs::Entity entity, Args&&... args)
		{	
			if (typeid(Type) != m_BucketType)
			{
				LOG_CORE_WARNING("This bucket is not of that type");
				return;
			}

			size_t Size = m_ComponentsList.size();

			if (entity.GetID() > m_ComponentsList.size())
				m_ComponentsList.resize(Size + (entity.GetID() - Size) + 10); //assures enough size for extra entites

			m_ComponentsList[entity.GetID()].Construct<Type>(entity, std::forward<Args>(args)...);
		}

		template<class Type>
		void DeleteComponent(Ecs::Entity Entity)
		{
			if (typeid(Type) != m_BucketType)
			{
				LOG_CORE_WARNING("This bucket is not of that type");
				return;
			}

			if (Entity.GetID() < m_ComponentsList.size())
				m_ComponentsList[Entity.GetID()].Clear();
			else
				LOG_CORE_WARNING("That entities component is not present within this bucket");
		}

	private:
		std::vector<Component> m_ComponentsList;
		std::type_index m_BucketType;
	};

}