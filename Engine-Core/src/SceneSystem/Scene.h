#pragma once

#include "ECS/Registry.h"
#include "Graphics/Renderer.h"
#include "Graphics/ModeImporting/AssimpImporter.h"

#include <string>
#include <unordered_map>

namespace TooGoodEngine
{
	enum class ComponentType 
	{
		Transform = 0, Material, Quad
	};

	struct ComponentDiscription
	{
		ComponentType ComponentType;
		Ecs::EntityID EntityID; //will change to UUID in the future
	};

	struct SceneData
	{
		std::string_view SceneName;
		std::vector<Ecs::Entity> SceneEntities;
		std::vector<ComponentDiscription> ComponentsList;
	};

	class Scene
	{
	public:
		Scene();
		Scene(const std::string_view& Name);
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
		AssimpImporter m_SourceImporter;
		std::unordered_map<std::filesystem::path, InstanceID> m_AddedModels;

		char m_EntityNameBuffer[30]{};
		char m_SourceFileBuffer[100]{};

		const float labelWidth = 100.0f;
		const float labelOffset = -10.0f;

		std::string_view m_DebugName;
		Ecs::Registry m_SceneRegistry;

		std::vector<Ecs::Entity> m_SceneEntites;
		Camera m_SceneCamera;
		bool click = false;
		
		bool m_ShowModelPopup = false;
		Ecs::Entity m_CurrentPopupEntity;

		bool m_ShowChangeEntityNamePopup = false;
		Ecs::Entity m_CurrentChangePopupEntity;

		friend class Impl_DisplayComponent;
	};

	class Impl_DisplayComponent
	{
	public:
		//template specialzed in Scene.cpp
		template<class Type>
		static void Call(Ecs::Entity entity, Scene* self)
		{
			TGE_LOG_WARN("Not a valid component");
		}
	};

	static std::shared_ptr<Scene> CurrentScene;
	
}