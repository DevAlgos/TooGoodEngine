#pragma once
#include <xhash>


namespace Ecs
{
	using	  EntityID = std::uint64_t;
	constexpr EntityID  NullEntity = std::numeric_limits<std::uint64_t>::max();

	class Registry;

	class Entity
	{
	public:
		Entity(std::string_view Name);
		Entity(std::string_view Name, EntityID ID);
		Entity(const Entity& other);
		Entity();

		virtual ~Entity();

		const std::string_view GetName() const { return m_EntityName; }
		const EntityID GetID() const { return m_EntityID; }
		
	private:
		std::string_view m_EntityName;
		EntityID m_EntityID;
	};
}

namespace std {
	template<>
	struct hash<Ecs::EntityID>
	{
		size_t operator()(const Ecs::EntityID& obj) const {
			return std::hash<uint64_t>{}(static_cast<uint64_t>(obj));
		}
	};
}