#include <pch.h>
#include "EditorLayer.h"
#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>

namespace {
	static bool opt_fullscreen = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static bool p_open = true;

	Graphics::LightSource* source;
	Graphics::Material* TestMaterial;

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
	

		OrthoCameraData CameraData;
		CameraData.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		CameraData.Front = glm::vec3(0.0f, 0.0f, -1.0f);
		CameraData.Up = glm::vec3(0.0f, 1.0f, 0.0f);

		CameraData.AspectRatio = (float)Application::GetMainWindow().GetWidth() / 
			(float)Application::GetMainWindow().GetHeight();

		CameraData.ZoomLevel = 5.0f;
		CameraData.CameraSpeed = 30.0f;

		m_OrthoCam.SetCam(CameraData);

		Graphics::TextureData BackGroundData;
		BackGroundData.FileLocation = "Engine/src/resources/Background.png";
		BackGroundData.InternalFormat = GL_RGB;
		BackGroundData.Type = Graphics::TextureType::Texture2D;
		BackGroundData.MipmapLevels = 0;
		BackGroundData.TextureParamaters = {
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
		};

		BackGround = std::make_unique<Graphics::Texture>(BackGroundData);

		source = new Graphics::LightSource();

		source->Color = { 1.0f, 0.0f, 0.5f };
		source->Position = { 1.0f, 1.0f, 1.0f };
		source->ID = 0;

		TestMaterial = new Graphics::Material();
		TestMaterial->ambient = { 0.2f, 0.2f, 0.2f };
		TestMaterial->diffuse = { 0.8f, 0.9f, 0.9f };
		TestMaterial->specular = { 1.0f, 1.0f, 1.0f };
		TestMaterial->shininess = 12.0f;
	}
	void EditorLayer::OnUpdate()
	{
		ViewFrame->Bind();

		m_OrthoCam.Update(Application::GetCurrentDeltaSecond());
		Graphics::Renderer2D::ClearColor({ 0.4f, 0.2f, 0.8f });
		Graphics::Renderer2D::BeginScene(m_OrthoCam);


		for (float i = -10.0f; i < 10.0f; i++)
		{
			for (float j = -10.0f; j < 10.0f; j++)
			{
				Graphics::Renderer2D::PushQuad({ i,j,0.0f }, { 1.0f,1.0f }, 0.0f, BackGround->Get());
			}

		}

		Graphics::Renderer2D::PushCircle({ 2.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f, 
			{0.8f, 0.4f, 0.5f, 1.0f}, *TestMaterial);

		Graphics::Renderer2D::PushCircle({ 9.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f,
			{ 0.8f, 0.4f, 0.5f, 1.0f });
		

		Graphics::Renderer2D::PushLight(*source);

		Graphics::Renderer2D::EndScene();

		ViewFrame->Unbind();


		if (InputManager::IsKeyPressed(KEY_UP))
			source->Position.y += 0.005f;
		if (InputManager::IsKeyPressed(KEY_DOWN))
			source->Position.y -= 0.005f;
		if (InputManager::IsKeyPressed(KEY_LEFT))
			source->Position.x -= 0.005f;
		if (InputManager::IsKeyPressed(KEY_RIGHT))
			source->Position.x += 0.005f;


	}
	void EditorLayer::OnGUIUpdate()
	{	
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::Begin("Editor", &p_open, ImGuiWindowFlags_MenuBar);
		ImGui::ShowDemoWindow();

		ImGui::ColorEdit3("Light Color", glm::value_ptr(source->Color));

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
		delete source;
	}
}