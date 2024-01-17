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
		TransformComponent() : s_Transform(0.0f), s_RotationAxis(0.0f), s_Rotation(0.0f) {}
		TransformComponent(const glm::vec3& Position, const glm::vec3& Scale, const glm::vec3& RotationAxis, float Rotation)
			: s_RotationAxis(RotationAxis), s_Rotation(0.0f)
		{
			s_Transform = glm::mat4(1.0f);
			s_Transform = glm::translate(s_Transform, Position)
				* glm::rotate(s_Transform, glm::radians(Rotation), RotationAxis)
				* glm::scale(s_Transform, Scale);
		}


		float s_Rotation;
		glm::vec3 s_RotationAxis;
		glm::mat4 s_Transform;
	};

	struct MaterialComponent
	{
	public:
		MaterialComponent(const glm::vec4& Albedo, const glm::vec3& Reflectivity,
						 const glm::vec3& EmissionColor, float EmissionPower, 
						 float Roughness, std::shared_ptr<TooGoodEngine::Texture> TextureRef)
			: s_Albedo(Albedo), s_Reflectivity(Reflectivity), 
			  s_EmissionColor(EmissionColor), s_EmissionPower(EmissionPower),
			  s_Roughness(Roughness), s_MaterialTexture(TextureRef)
		{
		}



		glm::vec4 s_Albedo;
		glm::vec3 s_Reflectivity;
		glm::vec3 s_EmissionColor;
		float     s_EmissionPower;
		float	  s_Roughness;

		std::shared_ptr<TooGoodEngine::Texture> s_MaterialTexture;
	};

	//tag saying its a quad component
	struct QuadComponent
	{
		QuadComponent() = default;
	};
}