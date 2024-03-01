#pragma once

#include <string_view>
#include <limits>
#include <cinttypes>

namespace Ecs
{
	using	  EntityID = std::uint64_t;
	constexpr EntityID  NullEntity = std::numeric_limits<std::uint64_t>::max();

	class Entity
	{
	public:
		Entity(std::string_view Name);
		Entity(std::string_view Name, EntityID ID);
		Entity(const Entity& other);
		Entity();

		bool operator==(const Ecs::Entity& Other) const 
		{
			return Other.m_EntityID == m_EntityID;
		}

		bool operator!=(const Ecs::Entity& Other) const
		{
			return Other.m_EntityID != m_EntityID;
		}

		virtual ~Entity();

		void SetName(const std::string_view& Name) { m_EntityName = Name; }
		const std::string_view GetName() const { return m_EntityName; }
		const EntityID GetID() const { return m_EntityID; }
		
	private:
		std::string_view m_EntityName;
		EntityID m_EntityID;

		friend class Registry;

	};
}

