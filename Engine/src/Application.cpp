
#include <pch.h>
#include <Application.h>

#include <Game.h>

namespace
{
	static Game SandBox;
	static Graphics::Window s_MainWindow;

}

Application::Application()
{
	Utils::Logger::Init(Utils::Logger::Platform::Windows);
	Utils::JobManager::InitalizeManager();


	s_MainWindow.Create(1280, 720, "Engine");
	s_MainWindow.Init();

	Graphics::Renderer2D::Init();
	SandBox.OnInit();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(s_MainWindow.GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//glfwSwapInterval(1);
}

Application::~Application()
{
	SandBox.ShutDown();
	Graphics::Renderer2D::ShutDown();
	Utils::JobManager::ShutdownManager();
	Utils::Logger::ShutDown();

}

void Application::MainLoop()
{
	float deltaTime = 0.0f;
	float end = 0.0f;
	float start = 0.0f;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!s_MainWindow.WindowClosed())
	{
		s_MainWindow.SwapBuffers();
		s_MainWindow.PollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		start = glfwGetTime();
		deltaTime = end - start;
		end = start;

		SandBox.OnUpdate(deltaTime);

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(s_MainWindow.GetWindow(), &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}



}

void Application::MainLoop(MemoryData stats)
{
	float deltaTime = 0.0f;
	float end = 0.0f;
	float start = 0.0f;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	
	while (!s_MainWindow.WindowClosed())
	{
		s_MainWindow.SwapBuffers();
		s_MainWindow.PollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DisplayDebugInfo(stats);

		start = glfwGetTime();
		deltaTime = end - start;
		end = start;

		SandBox.OnUpdate(deltaTime);


		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(s_MainWindow.GetWindow(), &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	}
}

Graphics::Window& Application::GetMainWindow()
{
	return s_MainWindow;
}

void Application::DisplayDebugInfo(MemoryData stats)
{
	ImGui::Begin("Memory Usage");
	ImGui::Text("Memory Allocated %i", stats.AllocatedMemory);
	ImGui::Text("Memory Freed %i", stats.FreedMemory);
	ImGui::Text("Memory Using: %i", stats.CurrentlyUsingMemory());
	ImGui::End();
}