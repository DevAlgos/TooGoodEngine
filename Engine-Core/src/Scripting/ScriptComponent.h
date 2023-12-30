#pragma once

#include <functional>
#include <memory>


namespace Scripting
{
	struct Programmable
	{
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(float deltaTime) {}
	};

	struct ScriptWrapper
	{
		std::shared_ptr<Programmable> s_Programmable;

		std::function<void()> OnCreate;
		std::function<void()> OnDestroy;
		std::function<void(float Timestep)> OnUpdate;
 
		ScriptWrapper(std::shared_ptr<Programmable> programmable) : s_Programmable(programmable) {};

		void CreateScript()
		{
			OnCreate = [this]() {this->s_Programmable->OnCreate(); };
			OnDestroy = [this]() {this->s_Programmable->OnDestroy(); };
			OnUpdate = [this](float deltaTime) {this->s_Programmable->OnUpdate(deltaTime); };

			OnCreate();
		}

		void ScriptOnUpdate(float deltaTime)
		{
			OnUpdate(deltaTime);
		}

		void ScriptOnDestroy()
		{
			OnDestroy();
			s_Programmable = nullptr;
		}
	};
}