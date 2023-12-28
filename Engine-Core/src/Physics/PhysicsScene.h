#pragma once

#include <Utils/RandomNumbers.h>
#include <Utils/Logger.h>
#include <ECS/Component.h>
#include <ECS/Entity.h>
#include <map>

#include "CollisionMap.h"
//
//namespace tge
//{
//	struct PhysicsObject
//	{
//		Ecs::Renderable* Renderable = nullptr;
//		Ecs::QuadCollider* Collider = nullptr;
//		Ecs::PhysicsBehaviour* PhysicsBehaviour = nullptr;
//
//		Ecs::EntityID Entity;
//	};
//
//	struct PhysicsData
//	{
//		int Width = 0;
//		int Height = 0;
//
//		float BoundaryRadius = 0.0f;
//		glm::vec3 BoundaryCoordinates = { 0.0f, 0.0f, 0.0f };
//
//		int SubStepCount = 1;
//		float CellSize = 1.0f;
//	};
//
//
//	class PhysicsScene
//	{
//	public:
//		PhysicsScene(const PhysicsData& data);
//		~PhysicsScene();
//
//		template<typename T, typename ...Args>
//		void CreateGameObject(Args&&... args);
//
//		template<typename T, typename ...Args>
//		void CreateGameObject(const Ecs::PhysicsBehaviour& Behaviour, Args&&... args);
//
//		void DestroyObject(Ecs::EntityID GameObject);
//		void UpdateScene();
//	private:
//		/*
//			These equations used here are the Verlet integration equation
//			Next Position = Current Position + Velocity * Time Step
//		*/
//
//		void UpdateObjects();
//		void SolveCollision();
//
//		void SolveCollisionInCell(Cell& cell, Cell& other);
//		void AddObjectsToGrid();
//
//		void ApplyGravity();
//		void ApplyConstraint();
//		
//		void ApplyForce(const PhysicsObject& GameObject0, const PhysicsObject& GameObject1);
//	private:
//		Ecs::Registry EntityRegistry;
//		Ecs::EntityID LastEntity;
//
//		CollisionMap PhysicsMap;
//		PhysicsData m_PhysicsData;
//		//Each coordinate will contain a list of entites that may be in that cell.
//	};
//
//	template<typename T, typename ...Args>
//	inline void PhysicsScene::CreateGameObject(Args&&... args)
//	{
//		if (std::is_base_of_v<Ecs::Renderable, T>)
//		{
//			LastEntity++;
//			Ecs::Entity current = EntityRegistry.Create();
//			EntityRegistry.PushBackComponent<T>(current, std::forward<Args>(args)...);
//			Ecs::Renderable* renderable = EntityRegistry.GetComponent<T>(current, 0);
//			Ecs::RenderType type = renderable->GetType();
//
//			glm::vec3 position = renderable->GetPosition();
//			glm::vec2 scale = renderable->GetScale();
//			
//
//			switch (type)
//			{
//			case Ecs::RenderType::Quad:
//			{
//				EntityRegistry.PushBackComponent<Ecs::QuadCollider>(current, position, scale);
//				break;
//			}
//			case Ecs::RenderType::Circle: //haven't made circle collider yet
//				EntityRegistry.PushBackComponent<Ecs::QuadCollider>(current, position, scale);
//				break;
//			default:
//				break;
//			}
//			EntityRegistry.PushBackComponent<Ecs::PhysicsBehaviour>(current);
//
//			Ecs::PhysicsBehaviour* Behaviour = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(current, 2);
//			return;
//		}
//
//		LOGWARNING("Object is not renderable! no Entity has been created!");
//	}
//
//	template<typename T, typename ...Args>
//	inline void PhysicsScene::CreateGameObject(const Ecs::PhysicsBehaviour& Behaviour, Args&& ...args)
//	{
//		if (std::is_base_of_v<Ecs::Renderable, T>)
//		{
//			LastEntity++;
//			Ecs::Entity current = EntityRegistry.Create();
//			EntityRegistry.PushBackComponent<T>(current, std::forward<Args>(args)...);
//			Ecs::Renderable* renderable = EntityRegistry.GetComponent<T>(current, 0);
//			Ecs::RenderType type = renderable->GetType();
//
//			glm::vec3 position = renderable->GetPosition();
//			glm::vec2 scale = renderable->GetScale();
//
//			switch (type)
//			{
//			case Ecs::RenderType::Quad:
//			{
//				EntityRegistry.PushBackComponent<Ecs::QuadCollider>(current, position, scale);
//				break;
//			}
//			case Ecs::RenderType::Circle: //haven't made circle collider yet
//				EntityRegistry.PushBackComponent<Ecs::QuadCollider>(current, position, scale);
//				break;
//			default:
//				break;
//			}
//
//			EntityRegistry.PushBackComponent<Ecs::PhysicsBehaviour>(current, Behaviour->GetAcceleration());
//
//			renderable = nullptr;
//			return;
//		}
//
//		LOGWARNING("Object is not renderable! no Entity has been created!");
//	}
//
//}