#include <pch.h>
#include "EditorLayer.h"
#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>
#include <UI/UIManager.h>

namespace 
{
	static bool opt_fullscreen = true;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static bool p_open = true;

	TGE::LightSource* source;
	TGE::Material* TestMaterial;
	std::unique_ptr<TGE::ParticleScene> PScene;
	std::random_device dev;

	std::unique_ptr<TGE::PhysicsScene> PhysicsScene;
	int NumbOfEntites = 0;

	uint32_t* TextureData;

	TGE::UIManager manager;
	uint32_t font;
}

namespace Test
{
	struct TestObject : public Ecs::Renderable
	{
		TestObject(const glm::vec4& Color, const glm::vec3 Position, float Rotation, glm::vec2 Scale) :
			Ecs::Renderable(Ecs::RenderType::Quad, Color, Position, Rotation, Scale) {}

		TestObject() :
			Ecs::Renderable(Ecs::RenderType::Quad, { Utils::GenFloat(0.0,1.0f), 
				Utils::GenFloat(0.0,1.0f), Utils::GenFloat(0.0f,1.0f), 1.0f},
				{ Utils::GenFloat(-2.0f,20.0f), Utils::GenFloat(-2.0f,2.0f), 0.0f}, 0.0f, {1.0f, 1.0f}) {}

		~TestObject() {}

		
	};

	struct OtherObject : public Ecs::Renderable
	{
		OtherObject() :
			Ecs::Renderable(Ecs::RenderType::Quad, { 1.0f, 1.0f, 1.0f, 1.0f },
				{ 4.0f, -2.0f, 0.4f }, 0.0f, { 1.0f, 1.0f }) {}

		~OtherObject() {}
	};
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
	{
		TGE::Renderer2D::LoadInFont("../Resources/fonts/Anonymous.ttf");
		

		TGE::TextureData data;
		data.Width = 500;
		data.Height = 500;
		data.InternalFormat = TGE::TextureFormat::RGBA8;
		TextureData = new uint32_t[500 * 500];
		for (uint32_t i = 0; i < 500 * 500; i++) {
			TextureData[i] = 0xFFFF00FF;
		}

		TestTexture = std::make_unique<TGE::Texture>(TextureData, data);

		TestTexture->SetData(TextureData);

		TGE::PhysicsData PhysicsData;
		PhysicsData.Width = 50;
		PhysicsData.Height = 50;
		PhysicsData.BoundaryRadius = 20.0f;
		PhysicsData.BoundaryCoordinates = { 2.0f, 0.0f, 0.0f };

		PhysicsScene = std::make_unique<TGE::PhysicsScene>(PhysicsData);

		std::vector<TGE::Attachment> Attachments = {
		{TGE::AttachmentType::Color, false, (int)TGE::Application::GetMainWindow().GetWidth(), 
		(int)TGE::Application::GetMainWindow().GetHeight()},
		};

		std::map<GLenum, std::string_view> FramebufferShaderList = {
		{GL_VERTEX_SHADER, "../Resources/shaders/Defaults/post_process_vert.glsl"},
		{GL_FRAGMENT_SHADER, "../Resources/shaders/Defaults/post_process_frag.glsl"},
		};

		ViewFrame = std::make_unique<TGE::Framebuffer>(Attachments, FramebufferShaderList);
		PScene = std::make_unique<TGE::ParticleScene>();

		OrthoCameraData CameraData;
		CameraData.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		CameraData.Front = glm::vec3(0.0f, 0.0f, -1.0f);
		CameraData.Up = glm::vec3(0.0f, 1.0f, 0.0f);

		CameraData.AspectRatio = (float)TGE::Application::GetMainWindow().GetWidth() / 
			(float)TGE::Application::GetMainWindow().GetHeight();

		CameraData.ZoomLevel = 5.0f;
		CameraData.CameraSpeed = 30.0f;

		m_OrthoCam.SetCam(CameraData);

		TGE::TextureData BackGroundData;
		BackGround = std::make_unique<TGE::Texture>("../Resources/Background.png", 
			BackGroundData, TGE::Format::RGB);

		source = new TGE::LightSource();

		source->Color = { 1.0f, 0.0f, 0.5f };
		source->Position = { 1.0f, 1.0f, 1.0f };
		source->ID = 0;

		TestMaterial = new TGE::Material();
		TestMaterial->ambient = { 0.7f, 0.8f, 0.7f };
		TestMaterial->diffuse = { 1.0f, 1.0f, 1.0f };
		TestMaterial->specular = { 1.0f, 1.0f, 1.0f };
		TestMaterial->shininess = 32.0f;
	}
	void EditorLayer::OnUpdate()
	{
		ViewFrame->Bind();

		m_OrthoCam.Update(TGE::Application::GetCurrentDeltaSecond());
		TGE::Renderer2D::ClearColor({ 0.4f, 0.2f, 0.8f });
		TGE::Renderer2D::BeginScene(m_OrthoCam);

		if (InputManager::IsMouseButtonDown(RIGHT_MOUSE))
		{
			glm::vec4 ObjectColor = { Utils::GenFloat(0.0f, 1.0f), Utils::GenFloat(0.0f, 1.0f), Utils::GenFloat(0.0f, 1.0f), 1.0f};
			glm::vec3 ObjectPosition = { 0.0f, 10.0f, 0.0f };
			float     ObjectRotation = 0.0f;
			glm::vec2 ObjectScale = { 1.0f, 1.0f };

			PhysicsScene->CreateGameObject<Test::TestObject>(ObjectColor, ObjectPosition, ObjectRotation, ObjectScale);
			NumbOfEntites++;
		}

		if (InputManager::IsMouseButtonDown(LEFT_MOUSE))
		{
			TGE::Particle p;
			p.Position = glm::vec3(m_OrthoCam.GetMousePressCoordinates(), 0.0);
			p.xVelocity = Utils::GenFloat(-0.05f, 0.05f);
			p.yVelocity = Utils::GenFloat(-0.05f, 0.05f);
			p.DecaySpeed = 0.005f;
			p.Color = { 0.5f, 0.4f, 0.2f };
			p.EndColor = { 1.0f, 1.0f, 1.0f };
			p.Rotation = Utils::GenFloat(0, 360.0f);
			PScene->PushParticle(p);

		}

		for (float i = -10.0f; i < 10.0f; i++)
		{
			for (float j = -10.0f; j < 10.0f; j++)
			{
				TGE::Renderer2D::PushQuad({ i,j,0.0f }, { 1.0f,1.0f }, 0.0f, BackGround->Get());
			}

		}

		TGE::Renderer2D::PushCircle({ 2.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f,
			{0.8f, 0.4f, 0.5f, 1.0f}, *TestMaterial);

		TGE::Renderer2D::PushCircle({ 9.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f,
			{ 0.8f, 0.4f, 0.5f, 1.0f });
		
		TGE::Renderer2D::PushCircle({ 9.0f, -5.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f,
			{ 0.0f, 0.4f, 0.4, 1.0f });

		TGE::Renderer2D::PushUIText("have you ever wondered", 0, { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		TGE::Renderer2D::PushUIText("why octopuses have 8 legs ????", 0, { 1.0f, -0.5f, 0.0f }, { 1.0f, 1.0f }, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		TGE::Renderer2D::PushUIText("well now you know the elot p", 0, { 1.0f, -2.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f });


		PScene->Update(m_OrthoCam, TGE::Application::GetCurrentDeltaSecond());
		PhysicsScene->UpdateScene();


		TGE::Renderer2D::PushLight(*source);

		TGE::Renderer2D::EndScene();

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
		ImGui::Text("%i", NumbOfEntites);

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
		{
			ImGui::Image((void*)(intptr_t)TGE::Renderer2D::GetUIManager().GetFont(0).CharacterSheet.Get(), {800.0f,
			400.0f },
				{ 0,1 }, { 1,0 });
			ImGui::Image((void*)(intptr_t)ViewFrame->GetTexture(0), {1280.0f,
			720.0f },
				{ 0,1 }, { 1,0 });
		}
			

		ImGui::End();

	}
	void EditorLayer::OnShutdown()
	{
		delete TextureData;
		delete source;
	}
}