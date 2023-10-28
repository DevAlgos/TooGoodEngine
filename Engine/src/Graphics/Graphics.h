#pragma once

#include "GraphicsUtils.h"
#include "Shader.h"

namespace Graphics
{


	class ParticleScene //calculations needs to be on the GPU instead of the CPU
	{
	public:
		ParticleScene(); 
		~ParticleScene();

		void Update(OrthoGraphicCamera& Camera, float deltaTime);
		void PushParticle(const Particle& p);

	private:
		static const int ParticleCount = 1000;
		Utils::Job TestJob;
		std::array<Particle, ParticleCount> m_Particles;
		uint16_t m_ParticleIndex = ParticleCount - 1;
		glm::mat4 m_ParticleModel;

		
	};
}