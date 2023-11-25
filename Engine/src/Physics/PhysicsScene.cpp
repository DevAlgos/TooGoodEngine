#include <pch.h>
#include "PhysicsScene.h"
#include <Graphics/Renderer2D.h>

namespace TGE
{
	PhysicsScene::PhysicsScene()
		: LastEntity(0)
	{
	}

	PhysicsScene::~PhysicsScene()
	{
	}

	void PhysicsScene::UpdateScene()
	{
		for (uint32_t i = 0; i < LastEntity; i++)
		{
			Ecs::Renderable* Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(i, 0);
			Ecs::QuadCollider* Collider = EntityRegistry.GetComponent<Ecs::QuadCollider>(i, 1);
			Ecs::PhysicsBehaviour* Physics = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(i, 2);

			Physics->SetAcceleration({ 0.0f, -10.0f });


			for (Ecs::Entity j = 0; j < LastEntity; j++)
			{
				if (i != j)
				{
					Ecs::Renderable* Other = EntityRegistry.GetComponent<Ecs::Renderable>(j, 0);
					Ecs::QuadCollider* OtherCollider = EntityRegistry.GetComponent<Ecs::QuadCollider>(j, 1);
					Ecs::PhysicsBehaviour* OtherPhysics = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(j, 2);


					if (Collider->CheckCollision(std::move(*OtherCollider)))
						ApplyForce(Renderable, Collider, Physics, Other, OtherCollider);

				}

			}

			ApplyGravity(Renderable, Collider, Physics);
			ApplyContraint(Renderable, Collider);

			TGE::Renderer2D::PushCircle(Renderable->GetPosition(),
				Renderable->GetScale(), Renderable->GetRotation(), 1.0f, Renderable->GetColor());

		}
	}

	void PhysicsScene::ApplyGravity(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider,
		Ecs::PhysicsBehaviour* Behaviour)
	{
		const glm::vec2 Velocity = Renderable->GetPosition() - Renderable->GetOldPosition();
		Renderable->SetOldPosition(Renderable->GetPosition());

		glm::vec3 NewPosition =
			Renderable->GetPosition() +
			glm::vec3(Velocity, 0.0f) +
			glm::vec3(Behaviour->GetAcceleration(), 0.0f) *
			Application::GetCurrentDeltaSecond() *
			Application::GetCurrentDeltaSecond();

		Renderable->SetPosition(NewPosition);
		Collider->UpdateBounds(glm::vec2(NewPosition), Renderable->GetScale());


	}
	void PhysicsScene::ApplyContraint(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider)
	{
		const glm::vec3 Constraint = { 2.0f, 0.0f,0.0f }; //arbitrary for now, so can test easier
		const float Radius = { 6.0f }; //same here

		const glm::vec3 Pos = Renderable->GetPosition() - Constraint;
		float Distance = glm::length(Pos);

		if (Distance > Radius - 1.0f)
		{
			const glm::vec3 Back = Pos / Distance;
			glm::vec3 ConstraintedPosition = Constraint + Back * (Radius - 1.0f);
			Renderable->SetPosition(ConstraintedPosition);
			Collider->UpdateBounds(glm::vec2(Renderable->GetPosition()), Renderable->GetScale());
		}
	}
	void PhysicsScene::ApplyForce(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider, Ecs::PhysicsBehaviour* Behaviour, Ecs::Renderable* Other, Ecs::QuadCollider* OtherCollider)
	{
		glm::vec2 CollisionAxis = Renderable->GetPosition() - Other->GetPosition();
		float Distance = glm::length(CollisionAxis);

		const glm::vec2 Back = CollisionAxis / Distance;


		Renderable->SetOldPosition(Renderable->GetPosition());


		glm::vec3 NewPosition = Renderable->GetPosition() + (0.2f *
			Application::GetCurrentDeltaSecond() *
			glm::vec3(Back, 0.0f));

		glm::vec3 OtherNewPosition = Other->GetPosition() - (0.2f *
			Application::GetCurrentDeltaSecond() *
			glm::vec3(Back, 0.0f));

		Renderable->SetPosition(NewPosition);
		Collider->UpdateBounds(glm::vec2(NewPosition), Renderable->GetScale());

		Other->SetPosition(OtherNewPosition);
		OtherCollider->UpdateBounds(glm::vec2(OtherNewPosition), Other->GetScale());

	}


}