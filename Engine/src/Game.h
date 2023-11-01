#pragma once
#include <pch.h>


//Sandbox Game for testing

class Game : public Utils::BaseLayer
{
public:
	Game() = default;
	~Game() = default;

	virtual void OnInit()       override;
	virtual void OnUpdate()		override;
	virtual void OnGUIUpdate()  override;
	virtual void OnShutdown()   override;

private:


	float fov = 45.0f;
	OrthoGraphicCamera m_OrthoCam;
	std::unique_ptr<Graphics::ParticleScene> pScene;
	std::unique_ptr<Graphics::Framebuffer> TestFrameBuffer;

	std::unique_ptr<Graphics::Shader> ComputeShader;

	Graphics::VertexArrayObject m_LightVAO;

	Graphics::Shader modelShader;

	int numbOfQuads = 1;

	float x, y;
	uint32_t indicies[6];

	glm::vec3 lightColor;
	glm::vec3 lightPos;


	glm::mat4 m_Proj;
	glm::mat4 m_View;
	glm::mat4 m_Model;


	std::unique_ptr<Graphics::Texture> BackGround;

	std::unique_ptr<Graphics::Texture> Ben1;
	std::unique_ptr<Graphics::Texture> Ben2;
	std::unique_ptr<Graphics::Texture> Ben3;
	std::unique_ptr<Graphics::Texture> Ben4;

};

