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
		template<class Type>
		void DisplayComponent(Ecs::Entity entity)
		{
			Impl_DisplayComponent::Call<Type>(entity, this);
		}
	private:
		std::string_view m_DebugName;
		Ecs::Registry m_SceneRegistry;

		std::vector<Ecs::Entity> m_SceneEntites;
		Camera m_SceneCamera;
		bool click = false;

		friend class Impl_DisplayComponent;
	};

	struct Impl_DisplayComponent
	{
		//template specialzed in Scene.cpp
		template<class Type>
		static void Call(Ecs::Entity entity, Scene* self)
		{
			LOG_CORE_WARNING("Not a valid component");
		}
	};


	
		


	static std::shared_ptr<Scene> CurrentScene;
	
}