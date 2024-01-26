#pragma once

#include <filesystem>

#include "PythonBase.h"

namespace Scripting {

	struct PythonScriptWrapper
	{
		std::function<void()> OnCreate;
		std::function<void(float Timestep)> OnUpdate;

		PyObject* PyOnCreate = nullptr;
		PyObject* PyOnUpdate = nullptr;

		PythonScriptWrapper(PyObject* pyOnCreate, PyObject* pyOnUpdate) :
			PyOnCreate(pyOnCreate),  PyOnUpdate(pyOnUpdate)
		{};

		void CreateScript()
		{
			OnCreate = [&]() {PyObject_CallObject(PyOnCreate, NULL); };
			OnUpdate = [&](float deltaTime) 
				{
					PyObject* args = Py_BuildValue("(d)", static_cast<double>(deltaTime));
					PyObject_CallObject(PyOnUpdate, args);
					Py_DECREF(args);
				};

			OnCreate();
		}

		void ScriptOnUpdate(float deltaTime)
		{
			OnUpdate(deltaTime);
		}

		void ScriptOnDestroy()
		{
			Py_DECREF(PyOnCreate);
			Py_DECREF(PyOnUpdate);
		}
	};

	class PythonFileInfo
	{
	public:
		PythonFileInfo(const std::filesystem::path& fileLocation);
		PythonFileInfo();
		~PythonFileInfo();

		void Reload();
		void Reload(const std::filesystem::path& newPath);

		inline FILE* Get() { return m_File; }
		inline std::string GetLocation() const { return m_Location.string(); }

	private:
		FILE* m_File;
		std::filesystem::path m_Location;
	};

	struct ClassBindings
	{
		PyObject* EntityClass = nullptr;
	};

	struct PythonScriptingData
	{
		static constexpr uint32_t MaxFiles = 1000;
		static constexpr uint32_t NullFile = std::numeric_limits<uint32_t>::max();
		
		uint32_t CurrentSize = 0;

		PythonFileInfo* FilesToExecute = nullptr;
		PythonFileInfo* FilesToExecutePtr = nullptr;

		PyObject* TooGoodEngineModule = nullptr;

		ClassBindings Bindings;
	};

	class PythonScriptingEngine
	{
	public:
		PythonScriptingEngine() = delete;
		~PythonScriptingEngine() = delete;

		static void Init(const std::filesystem::path& enviromentPath);
		static void Execute(const std::filesystem::path& PythonFile);
		/*TODO: instead of running python file extract OnCreate() and OnUpdate() methods from file*/
		static uint32_t Load(const std::filesystem::path& PythonFile);

		static PythonScriptWrapper LoadScript(const std::string& PythonScript);
		static void Reload(uint32_t Index);
		static void ExecuteAll();

		static void Shutdown();

		static void ChangePath(const std::filesystem::path& enviormentPath);
	};

}