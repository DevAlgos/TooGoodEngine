#pragma once

#include <iostream>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <Utils/Logger.h>
#include <glm/glm.hpp>

#include "Entity.h"

namespace Ecs
{
	template<typename T, typename... Args>
	std::enable_if_t<std::is_constructible_v<T, Args...>, void*> ConstructObject(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	std::enable_if_t<!std::is_constructible_v<T, Args...>, void*> ConstructObject(Args&&...)
	{
		LOG_CORE_ERROR("Warning no suitable constructor for this object, nullptr returned!");
		__debugbreak();
		return nullptr; // Object cannot be constructed
	}

	struct Component
	{
		inline Component(const Component& other)
			: m_Data(other.m_Data)
		{
		}

		Component& operator=(const Component& other)
		{
			if (this != &other)
			{
				Clear();
				m_Data = other.m_Data;
			}
			return *this;
		}

		inline Component(Component&& other) noexcept
			: m_Data(std::move(other.m_Data))
		{
			other.m_Data = nullptr;
		}

		Component& operator=(Component&& other) noexcept
		{
			if (this != &other)
			{
				Clear();
				m_Data = std::move(other.m_Data);
				other.m_Data = nullptr;
			}
			return *this;
		}

		inline Component() noexcept : m_Data(nullptr) {}

		template<typename T, typename ...Args>
		inline void Construct(Args&&... args)
		{
			m_Data = ConstructObject<T>(std::forward<Args>(args)...);
		}

		inline void Clear() 
		{ 
			delete m_Data;
			m_Data = nullptr;
		}

		inline void* GetRaw() { return m_Data; }
		template<typename T>
		inline T* Get() {  return static_cast<T*>(m_Data); }

		template<typename T>
		inline T& GetRef() { return *(static_cast<T*>(m_Data)); }	

		inline ~Component() noexcept { Clear(); }

	private:
		void* m_Data;
	};

#pragma region Base Components List


	enum class RenderType
	{
		Quad = 0,
		Circle
	};

	struct PhysicsBehaviour //Behaviour Component
	{
		constexpr PhysicsBehaviour() : m_Acceleration(0.0f)
		{}
		inline PhysicsBehaviour(float InitialAcceleration) :
			m_Acceleration(InitialAcceleration)
		{}

		inline const glm::vec2 GetAcceleration() const { return m_Acceleration; }

		inline void SetAcceleration(const glm::vec2& other) { m_Acceleration = other; }

	private:
		glm::vec2 m_Acceleration;
	};

	struct QuadCollider //Quad Collider Component
	{
		inline QuadCollider() : m_Left(0), m_Right(0), m_Top(0), m_Down(0) {}
		inline QuadCollider(const glm::vec2& Position, const glm::vec2& Size) 
			: m_Left(Position.x), m_Right(Position.x + 2.0f * Size.x), 
			  m_Top(Position.y), m_Down(Position.y - 2.0f * Size.y)
		{}

		virtual ~QuadCollider() {}

		inline bool CheckCollision(const QuadCollider& other) const
		{
			bool xOverlap = (m_Right >= other.m_Left && m_Left <= other.m_Right);
			bool yOverlap = (m_Top >= other.m_Down && m_Down <= other.m_Top);

			return xOverlap && yOverlap;
		}

		inline void UpdateBounds(const glm::vec2& NewPosition, 
						         const glm::vec2& NewSize)
		{
			m_Left = NewPosition.x;
			m_Right = NewPosition.x + 2.0f * NewSize.x;
			m_Top = NewPosition.y;
			m_Down = NewPosition.y - 2.0f * NewSize.y;
		}

		inline const float GetLeft() const { return   m_Left; }
		inline const float GetRight() const { return  m_Right; }
		inline const float GetTop() const { return    m_Top; }
		inline const float GetDown() const { return   m_Down; }


	private:
		float m_Left, m_Right, m_Top, m_Down;
	};

	struct Renderable
	{
		inline Renderable() : m_Type(RenderType::Quad), m_Position(0.0f, 0.0f, 0.0f), m_OldPosition(0.0f, 0.0f, 0.0f),
			m_Rotation(0.0f), m_Scale(1.0f), m_Color(1.0f)
		{}
		inline Renderable(RenderType type, glm::vec4 Color, glm::vec3 position, float rotation, glm::vec2 scale) 
			: m_Type(type), m_Position(position), m_OldPosition(position), m_Rotation(rotation), m_Scale(scale), m_Color(Color)
		{}

		inline Renderable(const Renderable& other) 
			: m_Position(other.m_Position),
			  m_OldPosition(other.m_OldPosition),
		 	  m_Rotation(other.m_Rotation),
			  m_Type(other.m_Type),
			  m_Scale(other.m_Scale),
			  m_Color(other.m_Color)
			{}

		inline Renderable(Renderable&& other) noexcept
			: m_Position(std::move(other.m_Position)), 
			  m_OldPosition(std::move(other.m_OldPosition)),
			  m_Rotation(std::move(other.m_Rotation)), 
			  m_Scale(std::move(other.m_Scale)),
			  m_Type(std::move(other.m_Type)),
			  m_Color(std::move(other.m_Color))
			{}

		Renderable& operator=(const Renderable& other) 
		{
			if (this != &other) 
			{
				m_Type = other.m_Type;
				m_Position = other.m_Position;
				m_OldPosition = other.m_OldPosition;
				m_Rotation = other.m_Rotation;
				m_Scale = other.m_Scale;
				m_Color = other.m_Color;
			}
			return *this;
		}

		Renderable& operator=(Renderable&& other) noexcept 
		{
			if (this != &other) 
			{
				m_Type =     std::move(other.m_Type);
				m_Position = std::move(other.m_Position);
				m_OldPosition = std::move(other.m_OldPosition);
				m_Rotation = std::move(other.m_Rotation);
				m_Scale =    std::move(other.m_Scale);
				m_Color =    std::move(other.m_Color);
			}
			return *this;
		}

		virtual ~Renderable() {}

		inline void TransformColor(const glm::vec4& NewColor) { m_Color = NewColor; }
		inline void TransformPosition(const glm::vec3& Offset) { m_Position += Offset; };
		
		inline void SetPosition(const glm::vec3& OtherPosition) { m_Position = OtherPosition; }
		inline void SetOldPosition(const glm::vec3& OtherPosition) { m_OldPosition = OtherPosition; }

		inline void TransformScale(const glm::vec2& Offset) { m_Scale += Offset; };
		inline void TransformRotation(float Offset) { m_Rotation += Offset; };

		inline const RenderType GetType()       const { return m_Type; }
		inline const glm::vec4 GetColor()       const { return m_Color; }
		inline const glm::vec3 GetPosition()    const { return m_Position; }
		inline const glm::vec3 GetOldPosition() const { return m_OldPosition; }
		inline const glm::vec2 GetScale()       const { return m_Scale; }
		inline const float GetRotation()        const { return m_Rotation; }

	private:
		RenderType m_Type;
		
		glm::vec4 m_Color;
		glm::vec3 m_Position;
		glm::vec3 m_OldPosition;
		glm::vec2 m_Scale;
		float m_Rotation;
	}; 

#pragma endregion Base Components List


}