#include "pch.h"
#include "Game.h"



namespace //These are all for sandbox not going to be used in real engine (only testing purposes)
{
	static Graphics::SpriteSheet Sprite;
	std::array <Graphics::Material, 1000> particleMaterials;
	uint32_t particleIndex = 0;

	static std::array<Graphics::Particle, 4> testParticles;
	std::random_device dev;

	static std::array<uint32_t, 4> Bens;
	static Graphics::Material mat;

	Graphics::LightSource LightTest;
	Graphics::LightSource LightTest2;


	std::vector<uint32_t> xValues;
	std::vector<uint32_t> yValues;



}

namespace RandomNumberGenerator
{
	static std::mt19937 gen(dev());
	static float RandomNumber(float min, float max)
	{
		
		std::uniform_real_distribution dist6(min, max);

		return dist6(gen);
	}
}



void Game::OnInit()
{
		
	pScene = std::make_unique<Graphics::ParticleScene>();

	LightTest.Position = glm::vec3(1.0f, 1.0f, 1.0f);
	LightTest.Color = glm::vec3(0.4f, 0.2f, 0.9f);
	LightTest.ID = 0;

	LightTest2.Position = glm::vec3(4.0f, 1.0f, 1.0f);
	LightTest2.Color = glm::vec3(0.4f, 0.2f, 0.9f);
	LightTest2.ID = 1;


	for (int i = 0; i < 1000; i++)
	{
		Graphics::Material ParticleMaterial;
		ParticleMaterial.ambient = { 0.2f, 0.2f, 0.2f };
		ParticleMaterial.diffuse = { 0.8f, 0.2f, 0.9f };
		ParticleMaterial.specular = { 1.0f, 1.0f, 1.0f };
		//ParticleMaterial.shininess = 2.0f;

		particleMaterials[i] = ParticleMaterial;
	}


	Sprite.TotalWidth = 2037; //px2037x513
	Sprite.TotalHeight = 513; //px

	Sprite.CellWidth = 2037 / 8; //Sprite.TotalWidth / 8;
	Sprite.CellHeight = Sprite.TotalHeight / 1;
	

	
	
	OrthoCameraData CameraData;
	CameraData.Position = glm::vec3(0.0f, 0.0f, 0.0f);
	CameraData.Front = glm::vec3(0.0f, 0.0f, -1.0f);
	CameraData.Up = glm::vec3(0.0f, 1.0f, 0.0f);

	CameraData.AspectRatio = 16.0f / 9.0f;
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

	Graphics::TextureData Ben1Data;
	Ben1Data.FileLocation = "Engine/src/resources/mrbennoart/sim1.png";
	Ben1Data.InternalFormat = GL_RGBA;
	Ben1Data.Type = Graphics::TextureType::Texture2D;
	Ben1Data.MipmapLevels = 0;
	Ben1Data.TextureParamaters = {
		{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
		{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
	};

	Ben1 = std::make_unique<Graphics::Texture>(Ben1Data);
	Bens[0] = Ben1->Get();
	


	Graphics::TextureData Ben2Data;
	Ben2Data.FileLocation = "Engine/src/resources/mrbennoart/sim2.png";
	Ben2Data.InternalFormat = GL_RGBA;
	Ben2Data.Type = Graphics::TextureType::Texture2D;
	Ben2Data.MipmapLevels = 0;
	Ben2Data.TextureParamaters = {
		{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
		{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
	};

	Ben2 = std::make_unique<Graphics::Texture>(Ben2Data);
	Bens[1] = Ben2->Get();


	Graphics::TextureData Ben3Data;
	Ben3Data.FileLocation = "Engine/src/resources/mrbennoart/sim3.png";
	Ben3Data.InternalFormat = GL_RGBA;
	Ben3Data.Type = Graphics::TextureType::Texture2D;
	Ben3Data.MipmapLevels = 0;
	Ben3Data.TextureParamaters = {
		{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
		{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
	};

	Ben3 = std::make_unique<Graphics::Texture>(Ben3Data);
	Bens[2] = Ben3->Get();

	

	Graphics::TextureData Ben4Data;
	Ben4Data.FileLocation = "Engine/src/resources/mrbennoart/sim4.png";
	Ben4Data.InternalFormat = GL_RGBA;
	Ben4Data.Type = Graphics::TextureType::Texture2D;
	Ben4Data.MipmapLevels = 0;
	Ben4Data.TextureParamaters = {
		{GL_TEXTURE_MIN_FILTER, GL_NEAREST},
		{GL_TEXTURE_MAG_FILTER, GL_NEAREST},
	};

	Ben4 = std::make_unique<Graphics::Texture>(Ben4Data);
	Bens[3] = Ben4->Get();


	std::vector<Graphics::Attachment> Attachments = {
		{Graphics::AttachmentType::Color, false, 1280, 720}
	};

	std::map<GLenum, const char*> FramebufferShaderList = {
		{GL_VERTEX_SHADER, "Engine/src/resources/shaders/Defaults/post_process_vert.glsl"},
		{GL_FRAGMENT_SHADER, "Engine/src/resources/shaders/Defaults/post_process_frag.glsl"},
	};

	TestFrameBuffer = std::make_unique<Graphics::Framebuffer>(Attachments, FramebufferShaderList);




	std::map<GLenum, const char*> ComputeShaderList;
	ComputeShaderList[GL_COMPUTE_SHADER] = "Engine/src/resources/shaders/Defaults/testCompute.glsl";

	ComputeShader = std::make_unique<Graphics::Shader>(ComputeShaderList);

}

float CurrrentTimeS = 0.0f;
int index = 0;

float x, y = 1.0; 



void Game::OnUpdate(float delta)
{

	m_OrthoCam.Update(delta);

	TestFrameBuffer->Bind();
	Graphics::Renderer2D::ClearColor({ 1.0f, 1.0f, 1.0f });
	Graphics::Renderer2D::BeginScene(m_OrthoCam);


	for (int i = -50; i < 50; i++)
	{
		for (int j = -50; j < 50; j++)
		{

			Graphics::Renderer2D::PushQuad(glm::vec3(i, j, 0.0f), 1.0f, BackGround->Get(), glm::mat4(1.0f));
			numbOfQuads += 1;

		}
	}


	Graphics::Renderer2D::PushLight(LightTest);
	Graphics::Renderer2D::PushLight(LightTest2);

	Graphics::Renderer2D::PushQuad(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, Bens[index], glm::mat4(1.0f));
	
	
		
	if (InputManager::IsMouseButtonDown(LEFT_MOUSE))
	{

		Graphics::Particle test;
		test.DecaySpeed = 0.005f;

		double x, y;
		InputManager::GetMousePos(x, y);
		std::string s = std::to_string(x) + " " + std::to_string(y);
		LOG(s);

		int viewport[4];

		glGetIntegerv(GL_VIEWPORT, viewport);

		float normalizedX = ((x * 2.0f) / viewport[2]) - 1.0f;
		float normalizedY = (1.0f - (y * 2.0f) / viewport[3]) - 0.0f;

		glm::mat4 inverserOrthoMatrix = glm::inverse(m_OrthoCam.GetProjection());
		glm::vec4 mousePositionInNDC(normalizedX, normalizedY, 0.0f, 1.0f);
		glm::vec4 mousePositionInWorld = inverserOrthoMatrix * mousePositionInNDC;

		test.Position = glm::vec3(mousePositionInWorld.x - 0.5f, mousePositionInWorld.y - 0.5f, 0.0f);
		test.xVelocity = RandomNumberGenerator::RandomNumber(-0.05f, 0.05f) * abs(delta) * 100;
		test.yVelocity = RandomNumberGenerator::RandomNumber(-0.01f, 0.01f) * abs(delta) * 100;
		test.Rotation = RandomNumberGenerator::RandomNumber(0.0f, 360.0f) * delta; //*In degrees
		test.Color = glm::vec3(0.4f, 0.2f, 1.0f);
		test.EndColor = glm::vec3(0.0f, 0.4f, 0.0f);



		if (particleIndex > 999)
			particleIndex = 0;

		test.ParticleMaterial = particleMaterials[0];

		pScene->PushParticle(test);

		numbOfQuads += 1;
	}
	
	
	LightTest.Position = { x, y, 1.0f };
	LightTest.Color = { x,y,1.0f };
	if (InputManager::IsKeyDown(KEY_RIGHT))
	{
		x += 0.01f;
		//m_ModelForModel = glm::translate(m_ModelForModel, glm::vec3(0.01f, 0.0f, 0.0f));
	}
	if (InputManager::IsKeyDown(KEY_LEFT))
	{
		x -= 0.01f;
		//m_ModelForModel = glm::translate(m_ModelForModel, glm::vec3(-0.01f, 0.0f, 0.0f));

	}
	if (InputManager::IsKeyDown(KEY_UP))
	{
		y += 0.01f;
		//m_ModelForModel = glm::translate(m_ModelForModel, glm::vec3(0.0f, 0.01f, 0.0f));

	}
	if (InputManager::IsKeyDown(KEY_DOWN))
	{
		y -= 0.01f;
		//m_ModelForModel = glm::translate(m_ModelForModel, glm::vec3(0.0f, -0.01f, 0.0f));

	}
		
	CurrrentTimeS += abs(delta);
	
	if (CurrrentTimeS > 1.0f / 6.0f)
	{
		index += 1;
		if (index > 3)
			index = 0;

		CurrrentTimeS = 0;

	}
	
	pScene->Update(m_OrthoCam, delta);

	Graphics::Renderer2D::EndScene();

	ComputeShader->Use();
	glBindImageTexture(0, TestFrameBuffer->GetTexture(0), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	ComputeShader->Compute(1280 / 8, 720 / 4, 1);


	TestFrameBuffer->Unbind();
	TestFrameBuffer->UseFramebuffer();
	TestFrameBuffer->BindTexture(0);
	TestFrameBuffer->Draw();


	ImGui::Begin("Debug Window");
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Text("Number of quads drawn: %i", numbOfQuads);
	ImGui::End();	

	numbOfQuads = 0;

}

void Game::ShutDown()
{
}

