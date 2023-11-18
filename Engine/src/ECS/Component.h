#pragma once

#include <iostream>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <Utils/Logger.h>

namespace Ecs
{
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
			if (std::is_constructible_v<T>)
				Data = ConstructRawObject<T>(std::forward<Args>(args)...);
			else
				LOGERROR("Object constructor has been deleted!");
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

	enum class RenderType
	{
		Quad = 0,
		Circle
	};

	struct QuadCollider
	{
		QuadCollider() {}
		virtual ~QuadCollider() {}

		float left, right, top, down;
	};

	struct Renderable
	{
		inline Renderable() : m_Type(RenderType::Quad), m_Position(0.0f, 0.0f, 0.0f), 
			m_Rotation(0.0f), m_Scale(1.0f)
		{}
		inline Renderable(RenderType type, glm::vec3 position, float rotation, float scale) 
			: m_Type(type), m_Position(position), m_Rotation(rotation), m_Scale(scale)
		{}

		inline Renderable(const Renderable& other) 
			: m_Position(other.m_Position),
		 	  m_Rotation(other.m_Rotation),
			  m_Scale(other.m_Scale)
			{}

		inline Renderable(Renderable&& other) noexcept
			: m_Position(std::move(other.m_Position)), 
			  m_Rotation(std::move(other.m_Rotation)), 
			  m_Scale(std::move(other.m_Scale))
			{}

		Renderable& operator=(const Renderable& other) 
		{
			if (this != &other) 
			{
				m_Type = other.m_Type;
				m_Position = other.m_Position;
				m_Rotation = other.m_Rotation;
				m_Scale = other.m_Scale;
			}
			return *this;
		}

		Renderable& operator=(Renderable&& other) noexcept 
		{
			if (this != &other) 
			{
				m_Type =     std::move(other.m_Type);
				m_Position = std::move(other.m_Position);
				m_Rotation = std::move(other.m_Rotation);
				m_Scale =    std::move(other.m_Scale);
			}
			return *this;
		}

		virtual ~Renderable() {}

		inline void TransformPosition(const glm::vec3& Offset) { m_Position += Offset; };
		inline void TransformRotation(float Offset) { m_Rotation += Offset; };
		inline void TransformScale(float Offset) { m_Scale += Offset; };

		inline glm::vec3 GetPosition() { return m_Position; }
		inline float GetRotation() { return m_Rotation; }
		inline float GetScale() { return m_Scale; }

	private:
		RenderType m_Type;
		
		glm::vec3 m_Position;
		float m_Rotation;
		float m_Scale;
	}; 
}