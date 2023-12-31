
#include <pch.h>
#include <Application.h>
#include <Audio/Audio.h>

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
		Utils::Logger::Init(Utils::Logger::Platform::Windows);
		Audio::Init();
		Scripting::PythonScriptingEngine::Init(App.GetDetails().PythonEnviromentPath);
		
		ApplicationClock = std::make_unique<Utils::Clock>();

		s_MainWindow.Create((uint32_t)App.GetDetails().Width, (uint32_t)App.GetDetails().Height,
			App.GetDetails().ApplicationName.data());

		s_MainWindow.Init();

		Renderer2D::Init();
		Raytracing2D::Init();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Manager.PushLayer(std::make_shared<Utils::EditorLayer>());
		for (auto& layer : App.GetLayers())
			Manager.PushLayer(layer);
		
		
	}

	Application::~Application()
	{
		Scripting::PythonScriptingEngine::Shutdown();
		Audio::Shutdown();
		Renderer2D::ShutDown();
		Utils::Logger::ShutDown();

	}

	void Application::Run()
	{
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		
		Scripting::PythonScriptWrapper TestScript = 
			Scripting::PythonScriptingEngine::LoadScript("Test");
		TestScript.CreateScript();

		while (!s_MainWindow.WindowClosed())
		{
			start = (float)ApplicationClock->TimeElapsed(Utils::TimeUnit::mili);
			
			s_MainWindow.SwapBuffers();
			s_MainWindow.PollEvents();

			Manager.UpdateLayers();


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

		TestScript.ScriptOnDestroy();
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