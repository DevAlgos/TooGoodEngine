#include <pch.h>
#include "PhysicsScene.h"
#include <Graphics/Renderer2D.h>

namespace tge
{
	PhysicsScene::PhysicsScene(const PhysicsData& data)
		: LastEntity(0), PhysicsMap(data.Width, data.Height), m_PhysicsData(data)
	{
	}

	PhysicsScene::~PhysicsScene()
	{
	}

	void PhysicsScene::DestroyObject(Ecs::Entity GameObject)
	{
		if (GameObject >= LastEntity)
			return;

		Ecs::Renderable* Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(GameObject, 0);

		EntityRegistry.DeleteEntity(GameObject);

		LastEntity--;
	}

	void PhysicsScene::UpdateScene()
	{
		AddObjectsToGrid();
		ApplyGravity();
		SolveCollision();
		ApplyConstraint();
		UpdateObjects();
	}

	void PhysicsScene::UpdateObjects()
	{
		PhysicsObject GameObject;

		for (Ecs::Entity Entity = 0; Entity < LastEntity; Entity++)
		{	
			GameObject.Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(Entity, 0);
			GameObject.Collider = EntityRegistry.GetComponent<Ecs::QuadCollider>(Entity, 1);
			GameObject.PhysicsBehaviour = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(Entity, 2);

			glm::vec2 Velocity = GameObject.Renderable->GetPosition() - GameObject.Renderable->GetOldPosition();
			GameObject.Renderable->SetOldPosition(GameObject.Renderable->GetPosition());
			GameObject.Renderable->SetPosition(GameObject.Renderable->GetPosition() +
				glm::vec3(Velocity.x, Velocity.y, 0.0f) +
				glm::vec3(GameObject.PhysicsBehaviour->GetAcceleration().x, GameObject.PhysicsBehaviour->GetAcceleration().y, 0.0f)
				* Application::GetCurrentDeltaSecond() * Application::GetCurrentDeltaSecond()
			);

			GameObject.Collider->UpdateBounds(glm::vec2(GameObject.Renderable->GetPosition()), GameObject.Renderable->GetScale());

			GameObject.PhysicsBehaviour->SetAcceleration(glm::vec2(0.0f));

			tge::Renderer2D::PushQuad(GameObject.Renderable->GetPosition(),
				GameObject.Renderable->GetScale(), GameObject.Renderable->GetRotation(), GameObject.Renderable->GetColor());

		}
	}

	void PhysicsScene::SolveCollision()
	{
		for (int x = 1; x <= m_PhysicsData.Width - 1; x++)
		{
			for (int y = 1; y <= m_PhysicsData.Height - 1; y++)
			{
				auto& CurrentCell = PhysicsMap.GetCell(x, y);
				if (CurrentCell.EntitiesInCell.size() == 0)
					continue;

				for (int ChangeX = -1; ChangeX <= 1; ChangeX++)
				{
					for (int ChangeY = -1; ChangeY <= 1; ChangeY++)
					{
						auto& OtherCell = PhysicsMap.GetCell(x + ChangeX, y + ChangeY);

						if (OtherCell.EntitiesInCell.size() == 0)
							continue;

						SolveCollisionInCell(CurrentCell, OtherCell);
					}
				}
			}
		}
	}

	void PhysicsScene::SolveCollisionInCell(Cell& cell, Cell& other)
	{
		PhysicsObject GameObject;
		PhysicsObject OtherGameObject;

		for (int i = 0; i < cell.EntitiesInCell.size(); i++)
		{
			Ecs::Entity Current = cell.EntitiesInCell[i];
			
			GameObject.Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(Current, 0);
			GameObject.Collider = EntityRegistry.GetComponent<Ecs::QuadCollider>(Current, 1);
			GameObject.PhysicsBehaviour = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(Current, 2);

			for (int j = 0; j < other.EntitiesInCell.size(); j++)
			{
				Ecs::Entity Other = other.EntitiesInCell[j];

				if (Current != Other)
				{
					OtherGameObject.Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(Other, 0);
					OtherGameObject.Collider = EntityRegistry.GetComponent<Ecs::QuadCollider>(Other, 1);
					OtherGameObject.PhysicsBehaviour = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(Other, 2);

					for (int step = m_PhysicsData.SubStepCount; step >= 0; step--)
					{						
						if (GameObject.Collider->CheckCollision(*OtherGameObject.Collider))
							ApplyForce(GameObject, OtherGameObject);
						else
							break;
					}
				
				}
			}


			
		}
	}

	void PhysicsScene::AddObjectsToGrid()
	{
		PhysicsMap.Clear();

		Ecs::Renderable* Renderable;

		for (Ecs::Entity Current = 0; Current < LastEntity; Current++)
		{
			Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(Current, 0);
			float length = glm::length(Renderable->GetScale());

			if (length > m_PhysicsData.CellSize)
			{
				glm::vec2 ObjectPosition = Renderable->GetPosition();
				glm::vec2 ObjectScale = Renderable->GetScale();

				float XStart = ObjectPosition.x - ObjectScale.x / 2.0f;
				float YStart = ObjectPosition.y - ObjectScale.y / 2.0f;
				float XEnd = ObjectPosition.x + ObjectScale.x / 2.0f;
				float YEnd = ObjectPosition.y + ObjectScale.y / 2.0f;

				float Jump = m_PhysicsData.CellSize / length;

				for (float x = XStart; x <= XEnd; x+=Jump)
				{
					for (float y = YStart; y <= YEnd; y+=Jump)
					{
						PhysicsMap.AddEntity(static_cast<int>(std::floor(x)), 
											 static_cast<int>(std::floor(y)), Current);
					}
				}
			}
			else
			{
				int X = static_cast<int>(std::floor(Renderable->GetPosition().x));
				int Y = static_cast<int>(std::floor(Renderable->GetPosition().y));

				PhysicsMap.AddEntity(X, Y, Current);
			}
		}
	}

	void PhysicsScene::ApplyGravity()
	{
		Ecs::PhysicsBehaviour* PhysicsBehaviour;

		for (Ecs::Entity Current = 0; Current < LastEntity; Current++)
		{
			PhysicsBehaviour = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(Current, 2);
			PhysicsBehaviour->SetAcceleration(PhysicsBehaviour->GetAcceleration() + glm::vec2(0.0f, -1.0f));
		}
		

	}
	void PhysicsScene::ApplyConstraint()
	{
		Ecs::Renderable* Renderable;
		Ecs::QuadCollider* Collider;

		for (Ecs::Entity Current = 0; Current < LastEntity; Current++)
		{
			Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(Current, 0);
			Collider = EntityRegistry.GetComponent<Ecs::QuadCollider>(Current, 1);

			const glm::vec3 Pos = Renderable->GetPosition() - m_PhysicsData.BoundaryCoordinates;
			float Distance = glm::length(Pos);

			if (Distance > m_PhysicsData.BoundaryRadius - Renderable->GetScale().x)
			{
				const glm::vec3 Back = Pos / Distance;
				glm::vec3 ConstraintedPosition = m_PhysicsData.BoundaryCoordinates + Back * (m_PhysicsData.BoundaryRadius - Renderable->GetScale().x);
				Renderable->SetPosition(ConstraintedPosition);
				Collider->UpdateBounds(glm::vec2(Renderable->GetPosition()), Renderable->GetScale());
			}
		}
		
	}
	void PhysicsScene::ApplyForce(const PhysicsObject& GameObject0, const PhysicsObject& GameObject1)
	{
		glm::vec2 CollisionAxis = GameObject0.Renderable->GetPosition() - GameObject1.Renderable->GetPosition();
		float Distance = glm::length(CollisionAxis);

		glm::vec2 Back = CollisionAxis / Distance;

		glm::vec3 NewPosition = GameObject0.Renderable->GetPosition() + (0.2f *
			Application::GetCurrentDeltaSecond() *
			glm::vec3(Back, 0.0f));

		glm::vec3 OtherNewPosition = GameObject1.Renderable->GetPosition() - (0.2f *
			Application::GetCurrentDeltaSecond() *
			glm::vec3(Back, 0.0f));

		GameObject0.Renderable->SetPosition(NewPosition);
		GameObject0.Collider->UpdateBounds(glm::vec2(NewPosition), GameObject0.Renderable->GetScale());

		GameObject1.Renderable->SetPosition(OtherNewPosition);
		GameObject1.Collider->UpdateBounds(glm::vec2(OtherNewPosition), GameObject1.Renderable->GetScale());
	}


}