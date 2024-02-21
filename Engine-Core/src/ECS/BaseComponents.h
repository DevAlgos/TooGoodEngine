#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Graphics/Texture.h>

namespace Ecs
{
	struct TransformComponent
	{
	public:
		TransformComponent() : Transform(0.0f), RotationAxis(0.0f), Rotation(0.0f) {}
		TransformComponent(const glm::vec3& Position, const glm::vec3& Scale, const glm::vec3& RotationAxis, float Rotation)
			: RotationAxis(RotationAxis), Rotation(0.0f)
		{
			Transform = glm::identity<glm::mat4>();
			Transform = glm::translate(Transform, Position)
				* glm::rotate(Transform, glm::radians(Rotation), RotationAxis)
				* glm::scale(Transform, Scale);
		}


		float Rotation;
		glm::vec3 RotationAxis;
		glm::mat4 Transform;
	};

	struct MaterialComponent
	{
	public:
		MaterialComponent(const glm::vec4& Albedo, const glm::vec3& Reflectivity,
						 const glm::vec3& EmissionColor, float EmissionPower, 
						 float Roughness, const std::shared_ptr<TooGoodEngine::Texture>& TextureRef)
			: Albedo(Albedo), Reflectivity(Reflectivity), 
			  EmissionColor(EmissionColor), EmissionPower(EmissionPower),
			  Roughness(Roughness), MaterialTexture(TextureRef), Metallic(0.0)
		{
		}



		glm::vec4 Albedo;
		glm::vec3 Reflectivity;
		glm::vec3 EmissionColor;
		float     EmissionPower;
		float	  Roughness;
		float     Metallic;

		std::shared_ptr<TooGoodEngine::Texture> MaterialTexture;
	};

	//tag saying its a quad component
	struct QuadComponent
	{
		QuadComponent() = default;
	};
}