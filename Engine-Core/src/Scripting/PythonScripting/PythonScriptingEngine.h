#pragma once

#include <filesystem>
#include <Python.h>

namespace Scripting {

	struct PythonFileInfo
	{
	public:
		PythonFileInfo(const std::filesystem::path& fileLocation);
		PythonFileInfo();
		~PythonFileInfo();

		void Reload();
		void Reload(const std::filesystem::path& newPath);

		inline FILE* Get() { return m_File; }
		inline const char* GetLocation() const { return m_Location.string().c_str(); }

	private:
		FILE* m_File;
		std::filesystem::path m_Location;
	};

	struct PythonScriptingData
	{
		static constexpr uint32_t MaxFiles = 1000;
		static constexpr uint32_t NullFile = std::numeric_limits<uint32_t>::max();
		
		uint32_t CurrentSize = 0;

		PythonFileInfo* FilesToExecute = nullptr;
		PythonFileInfo* FilesToExecutePtr = nullptr;
	};

	class PythonScriptingEngine
	{
	public:
		PythonScriptingEngine() = delete;
		~PythonScriptingEngine() = delete;

		static void Init(const std::filesystem::path& enviromentPath);
		static void Execute(const std::filesystem::path& PythonFile);
		static uint32_t Load(const std::filesystem::path& PythonFile);
		static void Reload(uint32_t Index);
		static void ExecuteAll();

		static void Shutdown();

		static void ChangePath(const std::filesystem::path& enviormentPath);
	};

}