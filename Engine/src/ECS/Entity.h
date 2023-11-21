#pragma once


#include <limits>
#include <vector>
#include <optional>
#include "Component.h"


namespace Ecs
{
	using  Entity = std::uint64_t;
	static Entity EntityCount = 0;

	struct EntityComponentStorage
	{
		inline EntityComponentStorage(const EntityComponentStorage& other)
		{
			Components = other.Components;
			Parent = other.Parent;
		}

		inline EntityComponentStorage& operator=(const EntityComponentStorage& other)
		{
			Components = other.Components;
			Parent = other.Parent;

			return *this;
		}

		inline EntityComponentStorage(EntityComponentStorage&& other) noexcept
		{
			Components = std::move(other.Components);
			Parent = std::move(other.Parent);
		}
		inline EntityComponentStorage& operator=(EntityComponentStorage&& other) noexcept
		{
			Components = std::move(other.Components);
			Parent = std::move(other.Parent);

			return *this;
		}

		inline constexpr bool operator==(const EntityComponentStorage& other) const { return Parent == other.Parent; }
		inline constexpr bool operator!=(const EntityComponentStorage& other) const { return Parent != other.Parent; }

		inline constexpr bool operator>=(const EntityComponentStorage& other) const { return Parent >= other.Parent; }
		inline constexpr bool operator<=(const EntityComponentStorage& other) const { return Parent <= other.Parent; }

		inline constexpr bool operator>(const EntityComponentStorage& other) const { return Parent > other.Parent; }
		inline constexpr bool operator<(const EntityComponentStorage& other) const { return Parent < other.Parent; }


		constexpr EntityComponentStorage() = default;
		inline EntityComponentStorage(Entity p) : Parent(p) {}

		inline ~EntityComponentStorage() noexcept = default;

		template<typename T, typename ...Args>
		inline void EmplaceBack(Args&&... args)
		{
			Components.emplace_back();
			Components.back().Construct<T>(std::forward<Args>(args)...);
		}

		inline void EraseComponent(size_t Index)
		{
			Components.erase(Components.begin() + Index);
		}

		template<typename T>
		inline T* GetComponent(size_t Index)
		{
			if (Index >= Components.size())
				return nullptr; //Is index out of bounds?
			else
				return Components[Index].Get<T>();
		}


		inline size_t GetSize() { return Components.size(); }

		std::vector<ComponentStorage> Components;
		Entity Parent = std::numeric_limits<uint64_t>::max();
	};


	class Registry
	{
	public:
		constexpr Registry() = default;
		inline ~Registry() noexcept = default;


		template<typename T, typename ...Args>
		inline void PushBackComponent(Entity target, Args&&... args)
		{
			if (Components.size() < target)
				return; //out of bounds
			else
				Components[target].EmplaceBack<T>(std::forward<Args>(args)...);
		}

		inline void EraseComponent(Entity target, size_t Index)
		{
			if (Components.size() <= target || target < 0 || Components[target].GetSize() <= Index || Index < 0)
				return;
			else
				Components[target].EraseComponent(Index);
		}

		template<typename T>
		inline T* GetComponent(Entity entity, size_t Index)
		{
			return Components[entity].GetComponent<T>(Index);
		}

		inline void DeleteEntity(Entity entity)
		{
			if (entity >= Components.size())
				return;

			Components.erase(Components.begin() + entity);
		}

		inline Entity Create()
		{
			Entity current = EntityCount;
			EntityCount++;

			Components.push_back(EntityComponentStorage(current));
			return current;
		}


	private:
		std::vector<EntityComponentStorage> Components;

	};

}