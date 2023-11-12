#pragma once

#include <iostream>
#include <algorithm>
#include <functional>
#include <type_traits>

namespace Ecs
{
	template<typename T, typename ...Args>
	T& ConstructObject(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	void* ConstructRawObject(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}

	struct ComponentStorage
	{
		inline constexpr bool operator==(const ComponentStorage& other) const { return Data == other.Data; }
		inline constexpr bool operator!=(const ComponentStorage& other) const { return Data != other.Data; }

		inline ComponentStorage(const ComponentStorage& other)
			: Data(other.Data)
		{
		}

		ComponentStorage& operator=(const ComponentStorage& other)
		{
			if (this != &other)
			{
				Clear();
				Data = other.Data;
			}
			return *this;
		}

		inline ComponentStorage(ComponentStorage&& other) noexcept
			: Data(std::move(other.Data))
		{
			other.RemoveOwnership();
		}

		ComponentStorage& operator=(ComponentStorage&& other) noexcept
		{
			if (this != &other)
			{
				Clear();
				Data = std::move(other.Data);
				other.Data = nullptr; 
			}
			return *this;
		}

		

		inline constexpr ComponentStorage() noexcept : Data(nullptr) {}

		template<typename T, typename ...Args>
		inline void Construct(Args&&... args)
		{
			Data = ConstructRawObject<T>(std::forward<Args>(args)...);
		}

		inline void Clear() 
		{ 
			delete Data; 
			RemoveOwnership();
		}
		inline void* GetRaw() { return Data; }

		template<typename T>
		inline T* Get() {  return static_cast<T*>(Data); }

		inline void RemoveOwnership() { Data = nullptr; }
		inline ~ComponentStorage() noexcept { Clear(); }

		void* Data;

	};
}