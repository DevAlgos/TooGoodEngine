#pragma once

#include "Entity.h"
#include "Sparse.h"
#include "Utils/Log.h"

namespace Ecs
{
	class Bucket
	{
	public:
		Bucket(std::type_index TypeIndex);
		Bucket();
		~Bucket();

		bool HasComponent(Ecs::Entity Entity)
		{
			return m_ComponentsList.HasComponent(Entity.GetID());
		}

		template<class Type, class Func>
		void ForEach(Func func)
		{
			for (size_t i = 0; i < m_ComponentsList.GetNoElements(); i++)
				func(m_ComponentsList.GetDirect<Type>(i));
		}

		template<class Type>
		Type& Get(Ecs::Entity Entity)
		{
			std::type_index typeIndex = typeid(Type);
			if (typeIndex == m_BucketType)
				return m_ComponentsList.Get<Type>(Entity.GetID());
			else 
			{
				TGE_CLIENT_ERROR("inavlid component");
				assert(false);
			}
		}

		template<class Type>
		EntityID GetEntityFromComponent(uint64_t Index)
		{
			std::type_index typeIndex = typeid(Type);
			if (typeIndex == m_BucketType)
				return m_ComponentsList.GetEntityFromComponent<Type>(Index);
			else
			{
				TGE_CLIENT_ERROR("inavlid component");
				TGE_HAULT();
			}
		}

		template<class Type>
		Type* BeginComponent() { return m_ComponentsList.BeginDense<Type>(); }

		template<class Type>
		Type* EndComponent() { return m_ComponentsList.EndDense<Type>(); }


		template<class Type, typename ...Args>
		void Insert(Ecs::Entity entity, Args&&... args)
		{	
			m_ComponentsList.Insert<Type>(entity.GetID(), std::forward<Args>(args)...);
		}

		template<class Type>
		void Delete(Ecs::Entity Entity)
		{
			if (typeid(Type) != m_BucketType)
			{
				TGE_LOG_WARN("This bucket is not of that type");
				return;
			}

			m_ComponentsList.Delete<Type>(Entity.GetID());
		}

	private:
		//std::vector<Component> m_ComponentsList;
		SparseSet m_ComponentsList;
		std::type_index m_BucketType;
	};

}