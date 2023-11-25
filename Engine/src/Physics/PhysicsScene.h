#pragma once

#include <Utils/Logger.h>
#include <ECS/Component.h>
#include <ECS/Entity.h>

namespace TGE
{
	class PhysicsScene
	{
	public:
		PhysicsScene();
		~PhysicsScene();

		template<typename T, typename ...Args>
		void CreateGameObject(Args&&... args);
		void UpdateScene();
	private:
		/*
			These equations used here are the Verlet integration equation
			Next Position = Current Position + Velocity * Time Step
		*/


		void ApplyGravity(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider,
			Ecs::PhysicsBehaviour* Behaviour);
		void ApplyContraint(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider);
		void ApplyForce(
			Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider, Ecs::PhysicsBehaviour* Behaviour,
			Ecs::Renderable* Other, Ecs::QuadCollider* OtherCollider);
	private:
		Ecs::Registry EntityRegistry;
		Ecs::Entity LastEntity;
	};

	template<typename T, typename ...Args>
	inline void PhysicsScene::CreateGameObject(Args&&... args)
	{
		if (std::is_base_of_v<Ecs::Renderable, T>)
		{
			LastEntity++;
			Ecs::Entity current = EntityRegistry.Create();
			EntityRegistry.PushBackComponent<T>(current, std::forward<Args>(args)...);
			Ecs::Renderable* renderable = EntityRegistry.GetComponent<T>(current, 0);
			Ecs::RenderType type = renderable->GetType();

			switch (type)
			{
			case Ecs::RenderType::Quad:
			{
				glm::vec3 position = renderable->GetPosition();
				glm::vec2 scale = renderable->GetScale();
				EntityRegistry.PushBackComponent<Ecs::QuadCollider>(current, position, scale);
				break;
			}
			case Ecs::RenderType::Circle: //haven't made circle collider yet
				break;
			default:
				break;
			}
			EntityRegistry.PushBackComponent<Ecs::PhysicsBehaviour>(current);


			renderable = nullptr;
			return;
		}

		LOGWARNING("Object is not renderable! no entity has been created!");
	}

}