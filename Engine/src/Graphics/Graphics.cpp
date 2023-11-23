#include "pch.h"
#include "Graphics.h"


namespace Graphics
{

	ParticleScene::ParticleScene()
	{
		m_ParticleModel = glm::mat4(1.0f);

		for (Particle& particle : m_Particles)
			particle.IsActive = false;

		
	}
	ParticleScene::~ParticleScene()
	{
	}
	void ParticleScene::Update(OrthoGraphicCamera& Camera, float deltaTime)
	{	

		for (Particle& particle : m_Particles)
		{

			if (particle.IsActive)
			{
				m_ParticleModel = glm::mat4(1.0f);

				particle.Alpha -= particle.DecaySpeed;
				if (particle.Size <= 0) //If it is a texture then this will make sure that it dissapears properly and size diminishes
				{
					particle.IsActive = false;
					particle.Alpha = 0.0f;
				}


				if (particle.Alpha <= 0) //incase alpha becomes 0 before size
				{
					particle.IsActive = false;
					particle.Alpha = 0;
				}


				glm::vec3 InterpolatedColor = particle.EndColor + (particle.Color - particle.EndColor) * particle.Alpha;
				float InterpolatedSize = particle.Size * 2.0f * particle.Alpha;

				particle.Position.x += particle.xVelocity;
				particle.Position.y += particle.yVelocity;

				if (particle.Texture == -1)
					Renderer2D::PushQuad(glm::vec3(particle.Position.x, particle.Position.y, 0.0f), { InterpolatedSize,InterpolatedSize },particle.Rotation, glm::vec4(InterpolatedColor, particle.Alpha), particle.ParticleMaterial);
				else
					Renderer2D::PushQuad(glm::vec3(particle.Position.x, particle.Position.y, 0.0f), { InterpolatedSize, InterpolatedSize },particle.Rotation, particle.Texture, particle.ParticleMaterial);

					
			}


		}
		
	}
	void ParticleScene::PushParticle(const Particle& p)
	{
		Particle& particle = m_Particles[m_ParticleIndex];
		particle = p;
		particle.IsActive = true;
		particle.Alpha = 1.0f;
		
		if (m_ParticleIndex <= 0)
			m_ParticleIndex = ParticleCount - 1;
		else
			m_ParticleIndex--;
	}


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
			Ecs::Renderable* Renderable =    EntityRegistry.GetComponent<Ecs::Renderable>(i, 0);
			Ecs::QuadCollider* Collider =    EntityRegistry.GetComponent<Ecs::QuadCollider>(i, 1);
			Ecs::PhysicsBehaviour* Physics = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(i, 2);
			
			Physics->SetAcceleration({ 0.0f, -1.0f });


			/*for (Ecs::Entity j = 0; j < LastEntity; j++)
			{
				if (i != j)
				{
					Ecs::Renderable*   Other         = EntityRegistry.GetComponent<Ecs::Renderable>(j, 0);
					Ecs::QuadCollider* OtherCollider = EntityRegistry.GetComponent<Ecs::QuadCollider>(j, 1);
					Ecs::PhysicsBehaviour* OtherPhysics = EntityRegistry.GetComponent<Ecs::PhysicsBehaviour>(j, 2);


					if (Collider->CheckCollision(*OtherCollider))
						ApplyForce(Renderable, Collider, Physics, Other);
					
				}
				
			}*/

			ApplyGravity(Renderable, Collider, Physics);
			ApplyContraint(Renderable, Collider);

			Graphics::Renderer2D::PushQuad(Renderable->GetPosition(),
				Renderable->GetScale(), Renderable->GetRotation(), Renderable->GetColor());
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
	void PhysicsScene::ApplyForce(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider, Ecs::PhysicsBehaviour* Behaviour, Ecs::Renderable* Other)
	{
		glm::vec2 Velocity = (Renderable->GetPosition() - Renderable->GetOldPosition());
		Velocity = glm::vec2(-Velocity);

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

	
}
