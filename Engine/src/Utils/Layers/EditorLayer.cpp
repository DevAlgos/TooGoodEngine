#include <pch.h>
#include "EditorLayer.h"
#include <Graphics/Texture.h>
#include <Graphics/Buffers.h>

namespace 
{
	static bool opt_fullscreen = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static bool p_open = true;

	TGE::LightSource* source;
	TGE::Material* TestMaterial;
	std::unique_ptr<TGE::ParticleScene> PScene;
	std::random_device dev;

	std::unique_ptr<TGE::PhysicsScene> PhysicsScene;
	int NumbOfEntites = 0;

	float* TextureData;
	
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
		TGE::TextureData data;
		data.Width = 500;
		data.Height = 500;
		data.InternalFormat = TGE::TextureFormat::RGBA16F;
		TextureData = new float[500 * 500 * 4];
		for (uint32_t i = 0; i < 500 * 500 * 4; i += 4) {
			TextureData[i] =  1.0f;     // Red component
			TextureData[i + 1] = 1.0f; // Green component
			TextureData[i + 2] = 10.0f; // Blue component
			TextureData[i + 3] = 1.0f; // Alpha component
		}

		TestTexture = std::make_unique<TGE::Texture>(TextureData, data);

		TestTexture->SetData(TextureData);

		PhysicsScene = std::make_unique<TGE::PhysicsScene>();
		
		glm::vec4 ObjectColor = { 0.5f, 0.4f, 0.2f, 0.1f };
		glm::vec3 ObjectPosition = { 0.4f, 0.5f, 0.4f };
		float     ObjectRotation = 0.0f;
		glm::vec2 ObjectScale = { 1.0f, 1.0f };

		/*PhysicsScene->CreateGameObject<Test::TestObject>();
		PhysicsScene->CreateGameObject<Test::OtherObject>();
		NumbOfEntites += 2;*/

		std::vector<TGE::Attachment> Attachments = {
		{TGE::AttachmentType::Color, false, Application::GetMainWindow().GetWidth(), 
		Application::GetMainWindow().GetHeight()},
		};

		std::map<GLenum, const char*> FramebufferShaderList = {
		{GL_VERTEX_SHADER, "Engine/src/resources/shaders/Defaults/post_process_vert.glsl"},
		{GL_FRAGMENT_SHADER, "Engine/src/resources/shaders/Defaults/post_process_frag.glsl"},
		};

		ViewFrame = std::make_unique<TGE::Framebuffer>(Attachments, FramebufferShaderList);
		PScene = std::make_unique<TGE::ParticleScene>();

		OrthoCameraData CameraData;
		CameraData.Position = glm::vec3(0.0f, 0.0f, 0.0f);
		CameraData.Front = glm::vec3(0.0f, 0.0f, -1.0f);
		CameraData.Up = glm::vec3(0.0f, 1.0f, 0.0f);

		CameraData.AspectRatio = (float)Application::GetMainWindow().GetWidth() / 
			(float)Application::GetMainWindow().GetHeight();

		CameraData.ZoomLevel = 5.0f;
		CameraData.CameraSpeed = 30.0f;

		m_OrthoCam.SetCam(CameraData);

		TGE::TextureData BackGroundData;
		BackGroundData.MipmapLevels = 0;
		BackGroundData.TextureParamaters = {
			{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
			{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
		};

		BackGround = std::make_unique<TGE::Texture>("Engine/src/resources/Background.png", 
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

		m_OrthoCam.Update(Application::GetCurrentDeltaSecond());
		TGE::Renderer2D::ClearColor({ 0.4f, 0.2f, 0.8f });
		TGE::Renderer2D::BeginScene(m_OrthoCam);

		if (InputManager::IsMouseButtonDown(RIGHT_MOUSE))
		{
			glm::vec4 ObjectColor = { Utils::GenFloat(0.0f, 1.0f), Utils::GenFloat(0.0f, 1.0f), Utils::GenFloat(0.0f, 1.0f), 1.0f};
			glm::vec3 ObjectPosition = { 0.4f, 0.5f, 0.4f };
			float     ObjectRotation = 0.0f;
			glm::vec2 ObjectScale = { 0.1f, 0.1f };

			PhysicsScene->CreateGameObject<Test::TestObject>(ObjectColor, ObjectPosition, ObjectRotation, ObjectScale);
			NumbOfEntites++;
		}

		if (InputManager::IsMouseButtonDown(LEFT_MOUSE))
		{
			//TGE::Particle p;
			//p.Position = glm::vec3(m_OrthoCam.GetMousePressCoordinates(), 0.0);
			//p.xVelocity = Utils::GenFloat(-0.05f, 0.05f);
			//p.yVelocity = Utils::GenFloat(-0.05f, 0.05f);
			//p.DecaySpeed = 0.005f;
			//p.Color = { 0.5f, 0.4f, 0.2f };
			//p.EndColor = { 1.0f, 1.0f, 1.0f };
			//p.Rotation = Utils::GenFloat(0, 360.0f);
			//PScene->PushParticle(p);

			for (uint32_t i = 0; i < 500 * 500 * 4; i += 4) {
				TextureData[i] = Utils::GenFloat(0.0f, 1.0f);     // Red component
				TextureData[i + 1] = Utils::GenFloat(0.0f, 1.0f); // Green component
				TextureData[i + 2] = Utils::GenFloat(0.0f, 1.0f); // Blue component
				TextureData[i + 3] = 1.0f; // Alpha component
			}

			TestTexture->SetData(TextureData);

		}

		for (float i = -10.0f; i < 10.0f; i++)
		{
			for (float j = -10.0f; j < 10.0f; j++)
			{
				TGE::Renderer2D::PushQuad({ i,j,0.0f }, { 1.0f,1.0f }, 0.0f, TestTexture->Get());
			}

		}

		

		TGE::Renderer2D::PushCircle({ 2.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f,
			{0.8f, 0.4f, 0.5f, 1.0f}, *TestMaterial);

		TGE::Renderer2D::PushCircle({ 9.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f,
			{ 0.8f, 0.4f, 0.5f, 1.0f });
		
		TGE::Renderer2D::PushCircle({ 9.0f, -5.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f, 1.0f,
			{ 0.0f, 0.4f, 0.4, 1.0f });
		
		PScene->Update(m_OrthoCam, Application::GetCurrentDeltaSecond());
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
			ImGui::Image((void*)(intptr_t)ViewFrame->GetTexture(0), {(float)Application::GetMainWindow().GetWidth(),
			(float)Application::GetMainWindow().GetHeight() },
				{ 0,1 }, { 1,0 });

		ImGui::End();

	}
	void EditorLayer::OnShutdown()
	{
		delete TextureData;
		delete source;
	}
}