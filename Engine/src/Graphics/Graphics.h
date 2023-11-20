#pragma once

#include "GraphicsUtils.h"
#include "Shader.h"
#include "ECS/Component.h"
#include "ECS/Entity.h"

namespace Graphics
{
	class ParticleScene 
	{
	public:
		ParticleScene(); 
		~ParticleScene();

		void Update(OrthoGraphicCamera& Camera, float deltaTime);
		void PushParticle(const Particle& p);

	private:
		static const int ParticleCount = 1000;
		std::array<Particle, ParticleCount> m_Particles;
		uint16_t m_ParticleIndex = ParticleCount - 1;
		glm::mat4 m_ParticleModel;

		
	};


	class PhysicsScene
	{
	public:
		PhysicsScene();
		~PhysicsScene();

		template<typename T, typename ...Args>
		void CreateGameObject(Args&&... args);
		void UpdateScene();
	private:
		void ApplyGravity(Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider);
		void ApplyForce(int XForce, int YForce, Ecs::Renderable* Renderable, Ecs::QuadCollider* Collider);
	private:
		Ecs::Registry EntityRegistry;
		uint32_t LastEntity;
	};
	
}