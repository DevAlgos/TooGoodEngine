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

			renderable = nullptr;
			return;
		}

		LOGWARNING("Object is not renderable! no entity has been created!");
	}
	
}