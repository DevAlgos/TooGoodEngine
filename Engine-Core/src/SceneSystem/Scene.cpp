#include <pch.h>
#include "Scene.h"

namespace TooGoodEngine
{
	static glm::vec2 TestScale = {1.0f, 1.0f};

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
		//m_SceneRegistry.Insert<Ecs::MaterialComponent>(testentity, Albedo, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.0f, nullptr);

		m_SceneEntites.push_back(testentity);

		Renderer2D::LoadInFont("../Resources/fonts/JetBrainsMono-Italic.ttf");


	}
	Scene::~Scene()
	{
	}
	void Scene::SceneUpdate()
	{
		if (Input::IsKeyDown(KEY_V))
			Input::EnableCursor();
		
		if (Input::IsKeyDown(KEY_B))
			Input::DisableCursor();


		if(Input::IsCursorEnabled())
			m_SceneCamera.Update(Application::GetCurrentDeltaSecond());
	}
	void Scene::SceneDisplay()
	{
		Renderer2D::BeginScene(m_SceneCamera);
		Renderer2D::ClearColor({ 0.2f, 0.4f, 0.5f});


		Renderer2D::PushUIText("some testing", 0, { 1.0f, 0.0f, 0.0f }, TestScale, 0.0f, { 1.0f, 0.0f, 1.0f, 1.0f });

		{
			uint64_t Index = 0;
			m_SceneRegistry.View<Ecs::QuadComponent>([&](auto& Component)
				{
					Ecs::Entity CurrentEntity("loop entity", m_SceneRegistry.GetEntityFromComponent<Ecs::QuadComponent>(Index));

					if (!m_SceneRegistry.HasComponent<Ecs::MaterialComponent>(CurrentEntity) ||
						!m_SceneRegistry.HasComponent<Ecs::QuadComponent>(CurrentEntity))
						return; //Needs both to render

					auto& MaterialComponent = m_SceneRegistry.Get<Ecs::MaterialComponent>(CurrentEntity);
					auto& TransformComponent = m_SceneRegistry.Get<Ecs::TransformComponent>(CurrentEntity);

					Renderer2D::PushQuad(TransformComponent, MaterialComponent);

					Index++;
				});
		
		}

		

		Renderer2D::EndScene();
	}
	void Scene::DisplayEntites()
	{
		ImGui::Begin("Entities");
		ImGui::SliderFloat2("Scale", &TestScale.x, 0.0f, 3.0f);

		ImGui::PushID(0604502);

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Add Entity"))
			{
				Ecs::Entity Temp = m_SceneRegistry.CreateEntity("unamed entity");
				m_SceneEntites.push_back(Temp);
			}

			ImGui::EndPopup();
		}

		for (Ecs::Entity& entity : m_SceneEntites)
		{
			ImGui::PushID((int)entity.GetID());

			ImGui::NextColumn();
			ImGui::AlignTextToFramePadding();
			bool node_open = ImGui::TreeNode("%s", entity.GetName().data());

			if (node_open)
			{
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Add Quad Component"))
					{
						if (!m_SceneRegistry.HasComponent<Ecs::QuadComponent>(entity))
							m_SceneRegistry.Insert<Ecs::QuadComponent>(entity);
						
					}

					if (ImGui::MenuItem("Add Transform Component"))
					{
						if (!m_SceneRegistry.HasComponent<Ecs::TransformComponent>(entity))
						{
							m_SceneRegistry.Insert<Ecs::TransformComponent>(entity,
								glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
								glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
						}
					}

					if (ImGui::MenuItem("Add Material Component"))
					{
						if (!m_SceneRegistry.HasComponent<Ecs::MaterialComponent>(entity))
						{
							/*const glm::vec4& Albedo, const glm::vec3& Reflectivity,
								const glm::vec3& EmissionColor, float EmissionPower,
								float Roughness, std::shared_ptr<TooGoodEngine::Texture> TextureRef*/

							m_SceneRegistry.Insert<Ecs::MaterialComponent>(entity, 
								glm::vec4(1.0f), glm::vec3(0.0f), glm::vec3(1.0f), 1.0f, 0.0f, nullptr);
						}
					}

					ImGui::EndPopup();
				}

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);
				DisplayComponent<Ecs::QuadComponent>(entity);

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);
				DisplayComponent<Ecs::TransformComponent>(entity);

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 10.0f);
				DisplayComponent<Ecs::MaterialComponent>(entity);

				

				ImGui::TreePop();
			}

	
			ImGui::PopID();
		}
		
		ImGui::PopID();
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
				ImGui::PushID(5030);

				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Remove"))
					{
						self->m_SceneRegistry.Delete<Ecs::TransformComponent>(entity);
						ImGui::EndPopup();
						ImGui::TreePop();
						ImGui::PopID();
						return;
					}
					
					ImGui::EndPopup();
				}

				

				auto& Transform = self->m_SceneRegistry.Get<Ecs::TransformComponent>(entity);

				glm::vec3 Position = Transform.s_Transform[3];

				glm::vec3 Size{};
				Size.x = glm::length(glm::vec3(Transform.s_Transform[0]));
				Size.y = glm::length(glm::vec3(Transform.s_Transform[1]));
				Size.z = glm::length(glm::vec3(Transform.s_Transform[2]));				

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Position");
				float CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				float ChangedPos = ImGui::SliderFloat3("##PositionSlider", &Position.x, -100.0f, 100.0f, "%.3f", 5.0f);
				
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Scale");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				float ChangedScale = ImGui::SliderFloat3("##ScaleSlider", &Size.x, 0.0f, 100.0f, "%.3f", 5.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Axis");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				float ChangedAxis = ImGui::SliderFloat3("##RotationAxisSlider", &Transform.s_RotationAxis.x, 0.0f, 1.0f);
			
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Rotation");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				float ChangedRotation = ImGui::SliderFloat("##RotationSlider", &Transform.s_Rotation, -1000.0f, 1000.0f, "%.3f", 1.0f);


				if (ChangedPos || ChangedScale || ChangedAxis || ChangedRotation)
				{
					Transform.s_Transform = glm::mat4(1.0f);
					Transform.s_Transform = glm::translate(Transform.s_Transform, Position)
						* glm::rotate(Transform.s_Transform, glm::radians(Transform.s_Rotation), Transform.s_RotationAxis)
						* glm::scale(Transform.s_Transform, Size);
				}

				ImGui::TreePop();
				ImGui::PopID();
			}

		}
	}

	template<>
	static void Impl_DisplayComponent::Call<Ecs::MaterialComponent>(Ecs::Entity entity, Scene* self)
	{
		if (self->m_SceneRegistry.HasComponent<Ecs::MaterialComponent>(entity))
		{
			if (ImGui::TreeNodeEx("Material Component"))
			{
				ImGui::PushID(59037238);
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Remove"))
					{
						self->m_SceneRegistry.Delete<Ecs::MaterialComponent>(entity);
						ImGui::EndPopup();
						ImGui::TreePop();
						ImGui::PopID();
						return;
					}

					ImGui::EndPopup();
				}

				

				auto& Material = self->m_SceneRegistry.Get<Ecs::MaterialComponent>(entity);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);
#
				/*const glm::vec4& Albedo, const glm::vec3& Reflectivity,
							 const glm::vec3& EmissionColor, float EmissionPower,
							 float Roughness, std::shared_ptr<TooGoodEngine::Texture> TextureRef*/

				ImGui::Text("Albdeo");
				float CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat4("##AlbedoSlider", &Material.s_Albedo.x, 0.0f, 1.0f, "%.3f", 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Reflectivity");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat3("##ReflectivitySlider", &Material.s_Reflectivity.x, 0.0f, 1.0f, "%.3f", 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Emission Color");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat3("##EmissionColorSlider", &Material.s_EmissionColor.x, 0.0f, 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Emission Power");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat("##EmissionPowerSlider", &Material.s_EmissionPower, 0, 1.0f, "%.3f", 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Roughness");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat("##RoughnessSlider", &Material.s_Roughness, 0, 1.0f, "%.3f", 1.0f);

				
				//TODO: Add image of texture here
				
				ImGui::TreePop();
				ImGui::PopID();
			}

		
		}
	}

	template<>
	static void Impl_DisplayComponent::Call<Ecs::QuadComponent>(Ecs::Entity entity, Scene* self)
	{
		if (self->m_SceneRegistry.HasComponent<Ecs::QuadComponent>(entity))
		{
			if (ImGui::TreeNodeEx("Quad Tag")) 
			{
				ImGui::PushID(10101414);
				if (ImGui::BeginPopupContextWindow())
				{

					if (ImGui::MenuItem("Remove"))
					{

						self->m_SceneRegistry.Delete<Ecs::QuadComponent>(entity);
						ImGui::EndPopup();
						ImGui::PopID();
						ImGui::TreePop();
						return;
					}

					ImGui::EndPopup();

				}

				ImGui::PopID();
				ImGui::TreePop();
			}
		}
	}
}