
#include <pch.h>
#include <Application.h>

#include <Game.h>

namespace
{
	static Graphics::Window s_MainWindow;
	static std::unique_ptr<Utils::Clock> ApplicationClock;

	static float deltaTime = 0.0f;
	static float end = 0.0f;
	static float start = 0.0f;
	static float elapsedTime;

	
}

Application::Application()
	: Manager()
{
	Utils::Logger::Init(Utils::Logger::Platform::Windows);
	Utils::JobManager::InitalizeManager();
	ApplicationClock = std::make_unique<Utils::Clock>();

	

	s_MainWindow.Create(1600, 900, "Engine");
	s_MainWindow.Init();

	Graphics::Renderer2D::Init();

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

	//glfwSwapInterval(1);
	Manager.PushLayer(std::make_unique<Utils::DebuggingLayer>());
	Manager.PushLayer(std::make_unique<Utils::EditorLayer>());
	//Manager.PushLayer(std::make_unique<Game>());
	
}

Application::~Application()
{
	Graphics::Renderer2D::ShutDown();
	Utils::JobManager::ShutdownManager();
	Utils::Logger::ShutDown();
	
}

void Application::MainLoop()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!s_MainWindow.WindowClosed())
	{
		start = glfwGetTime();

		s_MainWindow.SwapBuffers();
		s_MainWindow.PollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		

		Manager.UpdateLayers();

	
		ImGui::GetIO().DisplaySize = ImVec2(s_MainWindow.GetWidth(), s_MainWindow.GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backupContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupContext);
		}

		end = glfwGetTime();
		deltaTime = end - start;
	}



}

Graphics::Window& Application::GetMainWindow()
{
	return s_MainWindow;
}

long long Application::GetCurrentTime()
{
	return ApplicationClock->GetCurrentTime(Utils::TimeUnit::second);
}

float Application::GetCurrentDelta()
{
	return deltaTime;
}