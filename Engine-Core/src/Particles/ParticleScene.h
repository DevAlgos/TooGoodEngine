#pragma once

#include <Graphics/GraphicsUtils.h>
#include <Utils/Camera.h>

namespace tge
{
	struct Particle
	{
		glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);

		Material ParticleMaterial;
		uint32_t Texture = -1;

		glm::vec3 Color = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 EndColor = glm::vec3(1.0f, 1.0f, 1.0f);

		float Alpha = 0.0f;
		float Rotation = 0.005f * glm::pi<float>();
		glm::vec2 Scale = { 1.0f, 1.0f };

		float DecaySpeed = 0.05f;
		float Size = 1.0f;

		float xVelocity = 0.05f;
		float yVelocity = 0.05f;
		bool IsActive = false;

		bool operator==(const Particle& p)
		{
			return Position == p.Position && Color == p.Color && Rotation == p.Rotation && DecaySpeed == p.DecaySpeed ? true : false;
		}
	};

	class ParticleScene
	{
	public:
		ParticleScene();
		~ParticleScene();

		void Update(OrthoGraphicCamera& Camera, float deltaTime);
		void PushParticle(const Particle& p);

	private:
		static const uint16_t ParticleCount = 1000;
		std::array<Particle, ParticleCount> m_Particles;
		uint16_t m_ParticleIndex = ParticleCount - 1;
		glm::mat4 m_ParticleModel;


	};
}