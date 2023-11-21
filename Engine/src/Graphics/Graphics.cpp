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
			Ecs::Renderable* Renderable = EntityRegistry.GetComponent<Ecs::Renderable>(i, 0);
			Ecs::QuadCollider* Collider = EntityRegistry.GetComponent<Ecs::QuadCollider>(i, 1);

			for (Ecs::Entity j = 0; j < LastEntity; j++)
			{
				if (i != j)
				{
					Ecs::Renderable*   Other         = EntityRegistry.GetComponent<Ecs::Renderable>(j, 0);
					Ecs::QuadCollider* OtherCollider = EntityRegistry.GetComponent<Ecs::QuadCollider>(j, 1);
					
					if (Collider->CheckCollision(*OtherCollider))
						ApplyForce(10, 10, Renderable, Collider);
					
				}
				
			}

			ApplyGravity(Renderable, Collider);
			Graphics::Renderer2D::PushQuad(Renderable->GetPosition(),
				Renderable->GetScale(), Renderable->GetRotation(), Renderable->GetColor());
		}
	}

	void PhysicsScene::ApplyGravity(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider)
	{
	}

	void PhysicsScene::ApplyForce(int XForce, int YForce, Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider)
	{
	}

	
}
