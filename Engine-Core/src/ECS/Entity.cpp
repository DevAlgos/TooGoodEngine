#include<pch.h>
#include "Entity.h"

namespace Ecs
{
	Entity::Entity(std::string_view Name)
		: m_EntityName(Name), m_EntityID(NullEntity)
	{
	}
	Entity::Entity(std::string_view Name, EntityID ID)
		: m_EntityName(Name), m_EntityID(ID)
	{
	}
	Entity::Entity(const Entity& other)
		: m_EntityName(other.m_EntityName), m_EntityID(other.m_EntityID)
	{
	}

	Entity::Entity()
		: m_EntityName("null entity"), m_EntityID(NullEntity)
	{
	}

	Entity::~Entity()
	{
	}
}