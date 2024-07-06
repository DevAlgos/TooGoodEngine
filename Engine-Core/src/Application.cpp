
#include "pch.h"
#include "Application.h"
#include "Audio/Audio.h"

#include "Utils/Log.h"
#include "Utils/UUID.h"
#include "Graphics/Renderer.h"

namespace
{
	static TooGoodEngine::Window s_MainWindow;
	static std::unique_ptr<Utils::Clock> ApplicationClock;

	static float deltaTime = 0.0f;
	static float end = 0.0f;
	static float start = 0.0f;
	static float elapsedTime;
}

namespace TooGoodEngine
{
	Application::Application(const UserApplication& App)
		: Manager()
	{
		Log::Init();

		Audio::Init();
		Scripting::PythonScriptingEngine::Init(App.GetDetails().PythonEnviromentPath);
		
		ApplicationClock = std::make_unique<Utils::Clock>();

		s_MainWindow.Create((uint32_t)App.GetDetails().Width, (uint32_t)App.GetDetails().Height,
			App.GetDetails().ApplicationName.data());

		s_MainWindow.Init();

		Renderer::Init();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Manager.PushLayer(std::make_shared<Utils::EditorLayer>());
		for (auto& layer : App.GetLayers())
			Manager.PushLayer(layer);
	}

	Application::~Application()
	{
		Renderer::Shutdown();
		Scripting::PythonScriptingEngine::Shutdown();
		Audio::Shutdown();
	}

	void Application::Run()
	{
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		
		//Scripting::PythonScriptWrapper TestScript = 
		//	Scripting::PythonScriptingEngine::LoadScript("Test");
		//TestScript.CreateScript();

		while (!s_MainWindow.WindowClosed())
		{
			{
				start = (float)ApplicationClock->TimeElapsed(Utils::TimeUnit::mili);
				//TestScript.ScriptOnUpdate(deltaTime);

				{
					Utils::TimedScope s("Update GUI");
					s_MainWindow.SwapBuffers();
					s_MainWindow.PollEvents();

					if (s_MainWindow.WindowClosed())
						break;
				}

				{
					Utils::TimedScope layerScope("Update Layers");
					Manager.UpdateLayers();
				}
			
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				
				Manager.UpdateGUI();

				ImGui::GetIO().DisplaySize = ImVec2((float)s_MainWindow.GetWidth(), (float)s_MainWindow.GetHeight());

				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

				if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					GLFWwindow* backupContext = glfwGetCurrentContext();
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
					glfwMakeContextCurrent(backupContext);
				}
				

				end = (float)ApplicationClock->TimeElapsed(Utils::TimeUnit::mili);
				deltaTime = end - start;
			}
			
		}

	}

	Window& Application::GetMainWindow()
	{
		return s_MainWindow;
	}

	int64_t Application::GetCurrentTime()
	{
		return ApplicationClock->TimeElapsed(Utils::TimeUnit::mili);
	}

	float Application::GetCurrentDelta()
	{
		return deltaTime;
	}

	float Application::GetCurrentDeltaSecond()
	{
		return deltaTime / 1000.0f;
	}
}