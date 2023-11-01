#include <pch.h>
#include "EditorLayer.h"
#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>

namespace {
	static bool opt_fullscreen = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static bool p_open = true;
}
namespace Utils
{
	EditorLayer::EditorLayer()
	{
	}
	EditorLayer::~EditorLayer()
	{
	}
	void EditorLayer::OnInit()
	{;
		std::vector<Graphics::Attachment> Attachments = {
		{Graphics::AttachmentType::Color, false, Application::GetMainWindow().GetWidth(), 
		Application::GetMainWindow().GetHeight()},
		};

		std::map<GLenum, const char*> FramebufferShaderList = {
		{GL_VERTEX_SHADER, "Engine/src/resources/shaders/Defaults/post_process_vert.glsl"},
		{GL_FRAGMENT_SHADER, "Engine/src/resources/shaders/Defaults/post_process_frag.glsl"},
		};

		ViewFrame = std::make_unique<Graphics::Framebuffer>(Attachments, FramebufferShaderList);
	}
	void EditorLayer::OnUpdate()
	{
		ViewFrame->Bind();

	}
	void EditorLayer::OnGUIUpdate()
	{
		ViewFrame->Unbind();
		

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::Begin("Editor", &p_open, ImGuiWindowFlags_MenuBar);
		ImGui::ShowDemoWindow();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				ImGui::MenuItem("Show Rendering", nullptr, &opt_fullscreen);
				
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();

		}

		if (opt_fullscreen)
			ImGui::Image((void*)(intptr_t)ViewFrame->GetTexture(0), { (float)Application::GetMainWindow().GetWidth(),
			(float)Application::GetMainWindow().GetHeight() },
				{ 0,1 }, { 1,0 });

		ImGui::End();

	}
	void EditorLayer::OnShutdown()
	{
	}
}