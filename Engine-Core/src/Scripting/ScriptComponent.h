#pragma once

#include <functional>
#include <memory>


namespace Scripting
{
	struct Programmable
	{
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(float Timestep) {}
	};

	struct ScriptComponent
	{
		std::shared_ptr<Programmable> s_Programmable;

		std::function<void()> OnCreate;
		std::function<void()> OnDestroy;
		std::function<void(float Timestep)> OnUpdate;
 
		ScriptComponent(std::shared_ptr<Programmable> programmable) : s_Programmable(programmable) {};

		void CreateScript()
		{
			OnCreate = [this]() {this->s_Programmable->OnCreate(); };
			OnDestroy = [this]() {this->s_Programmable->OnDestroy(); };
			OnUpdate = [this](float Timestep) {this->s_Programmable->OnUpdate(Timestep); };

			OnCreate();
		}

		void ScriptOnUpdate(float Timestep)
		{
			OnUpdate(Timestep);
		}

		void ScriptOnDestroy()
		{
			OnDestroy();
			s_Programmable = nullptr;
		}
	};
}