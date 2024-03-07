#include "pch.h"
#include "Scene.h"

#include "Graphics/ModeImporting/AssimpImporter.h"


namespace TooGoodEngine
{
	Scene::Scene()
		: m_DebugName("unamed scene"), m_SceneRegistry()
	{
		CameraData CamData;
		CamData.Sensitivity = 0.05f;
		m_SceneCamera.SetCam(CamData);
	}
	Scene::Scene(const std::string_view& Name)
		: m_DebugName(Name), m_SceneRegistry()
	{
		CameraData CamData;
		CamData.Sensitivity = 0.05f;
		CamData.CameraSpeed = 10.0f;
		m_SceneCamera.SetCam(CamData);

		Ecs::Entity testentity = m_SceneRegistry.CreateEntity("test entity");
		//m_SceneRegistry.Insert<Ecs::TransformComponent>(testentity, Pos, Scale, RotationAxis, 0.0f);
		//m_SceneRegistry.Insert<Ecs::MaterialComponent>(testentity, Albedo, glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.0f, nullptr);

		m_SceneEntites.push_back(testentity);
	}
	Scene::~Scene()
	{
	}
	void Scene::SceneUpdate()
	{
		if (Input::IsKeyDown(KEY_V) && !ImGui::IsAnyItemActive())
			Input::EnableCursor();

		if (Input::IsKeyDown(KEY_B) && !ImGui::IsAnyItemActive())
			Input::DisableCursor();


		if (!Input::IsCursorEnabled())
			m_SceneCamera.Update(Application::GetCurrentDeltaSecond());
	}
	void Scene::SceneDisplay()
	{
		Renderer::Begin(m_SceneCamera);

		m_SceneRegistry.View<Ecs::DirectionalLightComponent>([&](Ecs::DirectionalLightComponent& Component) 
			{
				DirectionalLightSource LightSrc{};
				memcpy(&LightSrc, &Component, sizeof(DirectionalLightSource));
				Renderer::AddDirectionalLight(LightSrc);
			});

		uint64_t Index = 0;
		m_SceneRegistry.View<Ecs::ModelComponent>([&](Ecs::ModelComponent& Component)
			{
				Ecs::Entity CurrentEntity("loop entity", 
				m_SceneRegistry.GetEntityFromComponent<Ecs::ModelComponent>(Index));

				if (!m_SceneRegistry.HasComponent<Ecs::MaterialComponent>(CurrentEntity) ||
					!m_SceneRegistry.HasComponent<Ecs::TransformComponent>(CurrentEntity))
				{
					return;
				}

				auto& MaterialComponent = m_SceneRegistry.Get<Ecs::MaterialComponent>(CurrentEntity);
				auto& TransformComponent = m_SceneRegistry.Get<Ecs::TransformComponent>(CurrentEntity);

				Renderer::DrawModelInstance(Component.InstanceID, TransformComponent.Transform, MaterialComponent);

				Index++;

			});

		Renderer::End();
	}
	void Scene::DisplayEntites()
	{
		ImGui::Begin("Entities");
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

		if (m_ShowChangeEntityNamePopup)
		{
			if (ImGui::Begin("Change entity name"))
			{
				ImGui::Text("Entity name: ");
				ImGui::SameLine();
				ImGui::InputText("##EntityNameTag", m_EntityNameBuffer, 100);

				if (ImGui::Button("Enter"))
				{
					auto it = std::find(m_SceneEntites.begin(), m_SceneEntites.end(), m_CurrentChangePopupEntity);
					size_t Index = std::distance(m_SceneEntites.begin(), it);
					m_SceneEntites[Index].SetName(std::string_view(m_EntityNameBuffer));
					m_ShowChangeEntityNamePopup = false;

				}

				ImGui::End();
			}
		}

		if (m_ShowModelPopup)
		{
			if (ImGui::Begin("Model source"))
			{
				ImGui::Text("Source input: ");
				ImGui::SameLine();
				ImGui::InputText("##SourceInputTag", m_SourceFileBuffer, 100);

				if (ImGui::Button("Enter"))
				{
					std::filesystem::path SourcePath = m_SourceFileBuffer;

					if (m_AddedModels.contains(SourcePath))
					{
						m_SceneRegistry.Insert<Ecs::ModelComponent>(m_CurrentPopupEntity, 
																	m_AddedModels[SourcePath], 
																	SourcePath.string());
					}
					else
					{
						Model base = m_SourceImporter.ImportModel(SourcePath);
						InstanceID id = Renderer::AddUniqueModel(base);

						m_AddedModels[SourcePath] = id;

						m_SceneRegistry.Insert<Ecs::ModelComponent>(m_CurrentPopupEntity, id, SourcePath.string());
					}

					
					m_ShowModelPopup = false;
				}

				ImGui::End();
			}
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
					if (ImGui::MenuItem("Change Name"))
					{
						m_ShowChangeEntityNamePopup = true;
						m_CurrentChangePopupEntity = entity;
					}

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

					if (ImGui::MenuItem("Add Model Component"))
					{
						m_ShowModelPopup = true;
						m_CurrentPopupEntity = entity;
					}

					if (ImGui::MenuItem("Add Directional Light Component"))
					{
						m_SceneRegistry.Insert<Ecs::DirectionalLightComponent>(entity, glm::vec3(0.0f, -1.0f, -1.0f), 
							glm::vec3(1.0f, 1.0f, 1.0f), 
							3.0f);
					}

					ImGui::EndPopup();
				}

				ImGui::PushID(23432423);
				DisplayComponent<Ecs::QuadComponent>(entity);
				ImGui::PopID();

				ImGui::PushID(20540121);
				DisplayComponent<Ecs::TransformComponent>(entity);
				ImGui::PopID();
				
				ImGui::PushID(14013);
				DisplayComponent<Ecs::MaterialComponent>(entity);
				ImGui::PopID();

				ImGui::PushID(1053245);
				DisplayComponent<Ecs::ModelComponent>(entity);
				ImGui::PopID();

				ImGui::PushID(3420786);
				DisplayComponent<Ecs::DirectionalLightComponent>(entity);
				ImGui::PopID();

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
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Remove"))
					{
						self->m_SceneRegistry.Delete<Ecs::TransformComponent>(entity);
						ImGui::EndPopup();
						ImGui::TreePop();
						return;
					}
					
					ImGui::EndPopup();
				}

				

				auto& Transform = self->m_SceneRegistry.Get<Ecs::TransformComponent>(entity);

				glm::vec3 Position = Transform.Transform[3];

				glm::vec3 Size{};
				Size.x = glm::length(glm::vec3(Transform.Transform[0]));
				Size.y = glm::length(glm::vec3(Transform.Transform[1]));
				Size.z = glm::length(glm::vec3(Transform.Transform[2]));				

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
				float ChangedAxis = ImGui::SliderFloat3("##RotationAxisSlider", &Transform.RotationAxis.x, 0.0f, 1.0f);
			
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Rotation");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				float ChangedRotation = ImGui::SliderFloat("##RotationSlider", &Transform.Rotation, -1000.0f, 1000.0f, "%.3f", 1.0f);


				if (ChangedPos || ChangedScale || ChangedAxis || ChangedRotation)
				{
					Transform.Transform = glm::mat4(1.0f);
					Transform.Transform = glm::translate(Transform.Transform, Position)
						* glm::rotate(Transform.Transform, glm::radians(Transform.Rotation), Transform.RotationAxis)
						* glm::scale(Transform.Transform, Size);
				}

				ImGui::TreePop();
				
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
				ImGui::SliderFloat4("##AlbedoSlider", &Material.Albedo.x, 0.0f, 1.0f, "%.3f", 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Reflectivity");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat3("##ReflectivitySlider", &Material.Reflectivity.x, 0.0f, 1.0f, "%.3f", 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Emission Color");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat3("##EmissionColorSlider", &Material.EmissionColor.x, 0.0f, 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Emission Power");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat("##EmissionPowerSlider", &Material.EmissionPower, 0, 1.0f, "%.3f", 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Roughness");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat("##RoughnessSlider", &Material.Roughness, 0, 1.0f, "%.3f", 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Metalic");
				CursorPosX = ImGui::GetCursorPosX();
				ImGui::SameLine(CursorPosX + self->labelWidth);
				ImGui::SliderFloat("##MetalicSlider", &Material.Metallic, 0, 1.0f, "%.3f", 1.0f);

				
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
				if (ImGui::BeginPopupContextWindow())
				{

					if (ImGui::MenuItem("Remove"))
					{

						self->m_SceneRegistry.Delete<Ecs::QuadComponent>(entity);
						ImGui::EndPopup();
						ImGui::TreePop();
						return;
					}

					ImGui::EndPopup();

				}
				ImGui::TreePop();
			}
		}
	}

	template<>
	static void Impl_DisplayComponent::Call<Ecs::MeshComponent>(Ecs::Entity entity, Scene* self)
	{
		if (self->m_SceneRegistry.HasComponent<Ecs::MeshComponent>(entity))
		{
			if (ImGui::TreeNodeEx("Mesh Component"))
			{
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Remove"))
					{
						self->m_SceneRegistry.Delete<Ecs::MeshComponent>(entity);
						ImGui::EndPopup();
						ImGui::TreePop();
						return;
					}

					ImGui::EndPopup();
				}

				Ecs::MeshComponent& Component = self->m_SceneRegistry.Get<Ecs::MeshComponent>(entity);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Mesh ID %i", Component.InstanceID);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Mesh source file %s", Component.MeshSourceFile.c_str());

				ImGui::TreePop();
			}
		}
	}

	template<>
	static void Impl_DisplayComponent::Call<Ecs::ModelComponent>(Ecs::Entity entity, Scene* self)
	{
		if (self->m_SceneRegistry.HasComponent<Ecs::ModelComponent>(entity))
		{
			if (ImGui::TreeNodeEx("Model Component"))
			{
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Remove"))
					{
						self->m_SceneRegistry.Delete<Ecs::ModelComponent>(entity);
						ImGui::EndPopup();
						ImGui::TreePop();
						return;
					}

					ImGui::EndPopup();
				}

				Ecs::ModelComponent& Component = self->m_SceneRegistry.Get<Ecs::ModelComponent>(entity);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Model ID %i", Component.InstanceID);
				
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Model source file %s", Component.ModelSourceFile.c_str());

				ImGui::TreePop();
			}
		}
	}


	template<>
	static void Impl_DisplayComponent::Call<Ecs::DirectionalLightComponent>(Ecs::Entity entity, Scene* self)
	{
		if (self->m_SceneRegistry.HasComponent<Ecs::DirectionalLightComponent>(entity))
		{
			if (ImGui::TreeNodeEx("Directional Light"))
			{
				if (ImGui::BeginPopupContextWindow())
				{
					if (ImGui::MenuItem("Remove"))
					{
						self->m_SceneRegistry.Delete<Ecs::DirectionalLightComponent>(entity);
						ImGui::EndPopup();
						ImGui::TreePop();
						return;
					}
					ImGui::EndPopup();
				}

				Ecs::DirectionalLightComponent& Component = self->m_SceneRegistry.Get<Ecs::DirectionalLightComponent>(entity);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Direction");
				ImGui::SameLine(ImGui::GetCursorPosX() + self->labelWidth);
				ImGui::SliderFloat3("##DirectionTag", glm::value_ptr(Component.Direction), -1.0f, 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Color");
				ImGui::SameLine(ImGui::GetCursorPosX() + self->labelWidth);
				ImGui::SliderFloat3("##ColorTag", glm::value_ptr(Component.Color), 0.0f, 1.0f);

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + self->labelOffset);

				ImGui::Text("Intensity");
				ImGui::SameLine(ImGui::GetCursorPosX() + self->labelWidth);
				ImGui::SliderFloat("##IntensityTag", &Component.Intensity, 0.0f, 10.0f);

				ImGui::TreePop();
			}
		}
	}
}