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

#pragma region Base Components List

/*
	Note, for all of these base components they are not a MUST to use, they simply
	provide an interface with existing systems such as the physics system. You 
	are able to make your own base components from scratch and you use them however
	you like with the ECS system, and have them be interactable with the engine.
	And if you would like to make your own systems you could entirly ignore these
	if you wish. These can simply be used as an example to base your own system off.

*/


	enum class RenderType
	{
		Quad = 0,
		Circle
	};

	struct PhysicsBehaviour //Behaviour Component
	{
		constexpr PhysicsBehaviour() : m_GravityForce(0.01f), m_Acceleration(0.0f), m_Velocity(0.0f),
									   m_Mass(1.0f), m_CurrentDirection({1.0f, 0.0f}) 
		{}
		inline PhysicsBehaviour(float GravityForce, float InitialAcceleration, float InitialVelocity, float Mass, 
								const glm::vec2& CurrentDirection) :
			m_GravityForce(GravityForce), m_Acceleration(InitialAcceleration),m_Velocity(InitialVelocity), m_Mass(Mass),
			m_CurrentDirection(CurrentDirection)
		{}


		inline const float GetGravityForce() const { return m_GravityForce; }
		inline const float GetVelocity()	const  { return m_Velocity; }
		inline const float GetMass()         const {		 return m_Mass; }
		inline const glm::vec2 GetAcceleration() const { return m_Acceleration; }
		inline const glm::vec2 GetCurrentDirection() const { return m_CurrentDirection; }

		inline void SetGravityForce(float other) { m_GravityForce = other; }
		inline void SetAcceleration(const glm::vec2& other) { m_Acceleration = other; }
		inline void SetVelocity(float other) { m_Velocity = other; }
		inline void SetMass(float other) { m_Mass = other; }
		inline void SetDirection(const glm::vec2& other) { m_CurrentDirection = other; }

	private:
		float m_GravityForce;
		float m_Velocity;
		float m_Mass;

		glm::vec2 m_Acceleration;
		glm::vec2 m_CurrentDirection;
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


}