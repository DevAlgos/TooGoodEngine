#pragma once

#include <string>
#include <ECS/Registry.h>

namespace TooGoodEngine
{
	class Scene
	{
	public:
		Scene();
		Scene(std::string_view Name);
		~Scene();

		//TODO: will include updating physics and other systems
		void SceneUpdate(); 

		//Rendering the scene
		void SceneDisplay();

		//TODO: need to implement this
		void DisplayEntites();

		//TODO: This is just for testing purposes not perminant
		void AddEntity();



	private:
		std::string_view m_DebugName;
		Ecs::Registry m_SceneRegistry;

		std::vector<Ecs::Entity> m_SceneEntites;
		Camera m_SceneCamera;
	};

	static std::shared_ptr<Scene> CurrentScene;
}