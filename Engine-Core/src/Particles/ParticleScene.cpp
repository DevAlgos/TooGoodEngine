#include <pch.h>
#include "ParticleScene.h"
#include <Graphics/Renderer2D.h>

namespace TooGoodEngine
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
					Renderer2D::PushQuad(glm::vec3(particle.Position.x, particle.Position.y, 0.0f), { InterpolatedSize,InterpolatedSize }, particle.Rotation, glm::vec4(InterpolatedColor, particle.Alpha), particle.ParticleMaterial);
				else
					Renderer2D::PushQuad(glm::vec3(particle.Position.x, particle.Position.y, 0.0f), { InterpolatedSize, InterpolatedSize }, particle.Rotation, particle.Texture, particle.ParticleMaterial);


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
}