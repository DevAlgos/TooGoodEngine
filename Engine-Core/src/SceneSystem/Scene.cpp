#include <pch.h>
#include "Scene.h"

namespace TooGoodEngine
{
	Scene::Scene()
		: m_DebugName("unamed scene"), m_SceneRegistry()
	{
		CameraData CamData;
		CamData.Sensitivity = 0.05f;
		m_SceneCamera.SetCam(CamData);
	}
	Scene::Scene(std::string_view Name)
		: m_DebugName(Name), m_SceneRegistry()
	{
		CameraData CamData;
		CamData.Sensitivity = 0.05f;
		m_SceneCamera.SetCam(CamData);

		glm::vec3 Pos = { 0.0f, 0.0f, -1.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 0.0f };
		glm::vec3 RotationAxis = { 0.0f, 0.0f, 1.0f };

		/*
			const glm::vec4& Albedo, const glm::vec3& Reflectivity,
						 const glm::vec3& EmissionColor, float EmissionPower, 
						 float Roughness, std::shared_ptr<TooGoodEngine::Texture> TextureRef
		*/

		glm::vec4 Albedo = { 1.0f, 0.0f, 1.0f, 1.0f };


		Ecs::Entity testentity = m_SceneRegistry.CreateEntity("test entity");
		//m_SceneRegistry.Insert<Ecs::TransformComponent>(testentity, Pos, Scale, RotationAxis, 0.0f);
		m_SceneRegistry.Insert<Ecs::MaterialComponent>(testentity, Albedo, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.0f, nullptr);

		m_SceneEntites.push_back(testentity);


	}
	Scene::~Scene()
	{
	}
	void Scene::SceneUpdate()
	{
		m_SceneCamera.Update(Application::GetCurrentDeltaSecond());
	}
	void Scene::SceneDisplay()
	{
		Renderer2D::BeginScene(m_SceneCamera);
		Renderer2D::ClearColor({ 0.2f, 0.4f, 0.5f});

		{
			auto MaterialIt = m_SceneRegistry.Begin<Ecs::MaterialComponent>();
			auto TransformIt = m_SceneRegistry.Begin<Ecs::TransformComponent>();

			if (MaterialIt != nullptr && TransformIt != nullptr)
			{
				do
				{
					Renderer2D::PushQuad(*TransformIt, *MaterialIt);
					MaterialIt++;
					TransformIt++;

				} while (MaterialIt != m_SceneRegistry.End<Ecs::MaterialComponent>() &&
					     TransformIt != m_SceneRegistry.End<Ecs::TransformComponent>());
			}
		
		}

		Renderer2D::EndScene();
	}
	void Scene::DisplayEntites()
	{
		ImGui::Begin("Entities");

		for (const Ecs::Entity& entity : m_SceneEntites)
		{
			ImGui::PushID(entity.GetID());

			ImGui::NextColumn();
			ImGui::AlignTextToFramePadding();
			bool node_open = ImGui::TreeNode("%s", entity.GetName().data());

			if (node_open)
			{
				if (ImGui::BeginPopupContextWindow())
				{
					
					if (ImGui::MenuItem("Add Transform Component"))
					{
						if (!m_SceneRegistry.HasComponent<Ecs::TransformComponent>(entity))
						{
							m_SceneRegistry.Insert<Ecs::TransformComponent>(entity,
								glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
						}
					}
					ImGui::EndPopup();
				}

				DisplayComponent<Ecs::TransformComponent>(entity);

				ImGui::TreePop();
			}

	
			ImGui::PopID();
		}

		
		ImGui::End();
	}
	void Scene::AddEntity()
	{
	}

	template<>
	static void Impl_DisplayComponent::Call<Ecs::TransformComponent>(Ecs::Entity entity, Scene* self)
	{
		if (self->m_SceneRegistry.HasComponent<Ecs::TransformComponent>(entity))
		{
			if (ImGui::TreeNodeEx("Transform Component"))
			{
				auto& Transform = self->m_SceneRegistry.Get<Ecs::TransformComponent>(entity);

				glm::vec3 Position = Transform.s_Transform[3];

				glm::vec3 Size{};
				Size.x = glm::length(glm::vec3(Transform.s_Transform[0]));
				Size.y = glm::length(glm::vec3(Transform.s_Transform[1]));
				Size.z = glm::length(glm::vec3(Transform.s_Transform[2]));

				float Rotation = 0.0f; /*Need to find a good solution to extract from matrix*/

				const float labelWidth = 80.0f;
				const float labelOffset = 10.0f;

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + labelOffset);

				ImGui::Text("Position");
				float CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + labelWidth);
				float changed = ImGui::SliderFloat3("##PositionSlider", &Position.x, -100.0f, 100.0f);
				
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + labelOffset);

				ImGui::Text("Scale");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + labelWidth);
				float changed1 = ImGui::SliderFloat3("##ScaleSlider", &Size.x, -100.0f, 100.0f);



				if (changed || changed1)
				{
					Transform.s_Transform = glm::mat4(1.0f);
					Transform.s_Transform = glm::translate(Transform.s_Transform, Position)
						* glm::rotate(Transform.s_Transform, glm::radians(Rotation), { 0.0f, 0.0f, -1.0f })
						* glm::scale(Transform.s_Transform, Size);
				}

				ImGui::TreePop();
			}
		}
	}
}