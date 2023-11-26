
#include <pch.h>
#include <Application.h>

namespace
{
	static TGE::Window s_MainWindow;
	static std::unique_ptr<Utils::Clock> ApplicationClock;

	static float deltaTime = 0.0f;
	static float end = 0.0f;
	static float start = 0.0f;
	static float elapsedTime;
	
}

namespace TGE
{



	Application::Application(const TGE::UserApplication& App)
		: Manager()
	{
		Utils::Logger::Init(Utils::Logger::Platform::Windows);
		Utils::JobManager::InitalizeManager();

		ApplicationClock = std::make_unique<Utils::Clock>();

		s_MainWindow.Create((uint16_t)App.GetDetails().Width, (uint16_t)App.GetDetails().Height,
			App.GetDetails().ApplicationName.data());

		s_MainWindow.Init();

		TGE::Renderer2D::Init();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();


		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows


		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(s_MainWindow.GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460");

		Manager.PushLayer(std::make_shared<Utils::DebuggingLayer>());
		Manager.PushLayer(std::make_shared<Utils::EditorLayer>());
		for (auto& layer : App.GetLayers())
			Manager.PushLayer(layer);
		
	}

	Application::~Application()
	{
		TGE::Renderer2D::ShutDown();
		Utils::JobManager::ShutdownManager();
		Utils::Logger::ShutDown();

	}

	void Application::Run()
	{
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		while (!s_MainWindow.WindowClosed())
		{
			start = (float)ApplicationClock->TimeElapsed(Utils::TimeUnit::mili);

			s_MainWindow.SwapBuffers();
			s_MainWindow.PollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();


			Manager.UpdateLayers();


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

	TGE::Window& Application::GetMainWindow()
	{
		return s_MainWindow;
	}

	long long Application::GetCurrentTime()
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