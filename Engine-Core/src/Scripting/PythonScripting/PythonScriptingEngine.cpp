#include "pch.h"
#include "PythonScriptingEngine.h"

#include "Bindings.h"



namespace Scripting
{
	static PythonScriptingData s_Data;

	PythonFileInfo::PythonFileInfo(const std::filesystem::path& fileLocation)
		: m_File(nullptr), m_Location(fileLocation)
	{
		TGE_ASSERT(fileLocation.extension() == ".py", "not a valid python file");

		fopen_s(&m_File, fileLocation.string().c_str(), "r");
		TGE_ASSERT(m_File, "not a valid file");
	}

	PythonFileInfo::PythonFileInfo()
		: m_File(nullptr)
	{
	}

	void PythonFileInfo::Reload()
	{
		if (m_File)
			fclose(m_File);

		m_File = nullptr;

		fopen_s(&m_File, m_Location.string().c_str(), "r");
		TGE_ASSERT(m_File, "File could not be opened or does not exist");
	}

	void PythonFileInfo::Reload(const std::filesystem::path& newPath)
	{
		TGE_ASSERT(newPath.extension() == ".py", "not a valid python file");

		m_Location = newPath;

		if (m_File)
			fclose(m_File);

		m_File = nullptr;

		fopen_s(&m_File, m_Location.string().c_str(), "r");
		TGE_ASSERT(m_File, "not a valid file");
	}

	PythonFileInfo::~PythonFileInfo()
	{
		if (m_File)
			fclose(m_File);

		m_File = nullptr;
	}



	void PythonScriptingEngine::Init(const std::filesystem::path& enviromentPath)
	{
		Py_Initialize();

		s_Data.FilesToExecute = new PythonFileInfo[PythonScriptingData::MaxFiles];
		s_Data.FilesToExecutePtr = s_Data.FilesToExecute;

		std::filesystem::path PythonPath = "../Engine-Core/src/Scripting/PythonScripting/";

		PyObject* sys_module = PyImport_ImportModule("sys");
		PyObject* sys_path = PyObject_GetAttrString(sys_module, "path");
		PyList_Append(sys_path, PyUnicode_FromString(enviromentPath.string().c_str()));
		PyList_Append(sys_path, PyUnicode_FromString(PythonPath.string().c_str()));

		//src\Scripting\PythonScripting\Py_Libs

		Py_DECREF(sys_module);
		Py_DECREF(sys_path);

		s_Data.TooGoodEngineModule = PyModule_Create(&Bindings::TooGoodEngineModule);


		s_Data.Bindings.EntityClass = PyCapsule_New((void*)(&PyType_Type), nullptr, Bindings::CleanUpEntity);
		PyModule_AddObject(s_Data.TooGoodEngineModule, "Entity", s_Data.Bindings.EntityClass);

		PyDict_SetItemString(PyImport_GetModuleDict(), "TooGoodEngine", s_Data.TooGoodEngineModule);

	}
	void PythonScriptingEngine::Execute(const std::filesystem::path& PythonFile)
	{
		FILE* file = nullptr;
		fopen_s(&file, PythonFile.string().c_str(), "r");
		
		if (file)
		{
			PyRun_AnyFileEx(file, PythonFile.string().c_str(), 1);
			fclose(file);
		}
		else
			TGE_LOG_WARN("Python script failed to load at location: ", PythonFile.string());
		
	}
	void PythonScriptingEngine::Shutdown()
	{
		delete[] s_Data.FilesToExecute;
		Py_Finalize();
	}
	void PythonScriptingEngine::ChangePath(const std::filesystem::path& enviormentPath)
	{
		PyObject* sys_module = PyImport_ImportModule("sys");
		PyObject* sys_path = PyObject_GetAttrString(sys_module, "path");
		PyList_Append(sys_path, PyUnicode_FromString(enviormentPath.string().c_str()));
		Py_DECREF(sys_module);
	}
	PythonScriptWrapper PythonScriptingEngine::LoadScript(const std::string& PythonScript)
	{		
		PyObject* pyModule = PyImport_ImportModule(PythonScript.c_str());

		PyObject* onInitFunc = nullptr;
		PyObject* onUpdateFunc = nullptr;

		if (pyModule) {
			PyObject* pyFunc = PyObject_GetAttrString(pyModule, "OnInit");
			if (pyFunc && PyCallable_Check(pyFunc)) {
				//PyObject_CallObject(pyFunc, NULL);
				onInitFunc = pyFunc;
			}

			pyFunc = PyObject_GetAttrString(pyModule, "OnUpdate");

			if (pyFunc && PyCallable_Check(pyFunc)) {

				//double deltaTimeValue = 0.1;  // Example value
				//PyObject* args = Py_BuildValue("(d)", deltaTimeValue);

				//PyObject_CallObject(pyFunc, args);

				//// Don't forget to decref the args object
				//Py_DECREF(args);
				onUpdateFunc = pyFunc;
			}

		}

		if (!onInitFunc || !onUpdateFunc)
		{
			TGE_CLIENT_ERROR("Those functions were not in script!");
			__debugbreak();
		}

		return PythonScriptWrapper(onInitFunc, onUpdateFunc);

	}
	void PythonScriptingEngine::Reload(uint32_t Index)
	{
		if (Index >= s_Data.CurrentSize)
		{
			TGE_LOG_WARN("Index out of range file not reloaded");
			return;
		}

		s_Data.FilesToExecute[Index].Reload();
	}
	uint32_t PythonScriptingEngine::Load(const std::filesystem::path& PythonFile)
	{
		if (s_Data.CurrentSize >= PythonScriptingData::MaxFiles)
		{
			TGE_LOG_WARN("Max files have been reached script not loaded");
			return PythonScriptingData::NullFile;
		}
		s_Data.FilesToExecutePtr->Reload(PythonFile);
		s_Data.FilesToExecutePtr++;
		return s_Data.CurrentSize++;
	}
	void PythonScriptingEngine::ExecuteAll()
	{
		for (uint32_t i = 0; i < s_Data.CurrentSize; i++)
		{
			FILE* file = s_Data.FilesToExecute[i].Get();

			if (!file)
				TGE_HALT();
			
			PyRun_AnyFile(file, s_Data.FilesToExecute[i].GetLocation().c_str());
			fseek(file, 0, 0);
		}
	}
}