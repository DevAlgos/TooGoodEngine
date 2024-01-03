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
	}
	void Scene::AddEntity()
	{
	}
}