#include <pch.h>
#include "Camera.h"
#include "Utils.h"

namespace
{
	static bool keyWPressed = false;
	static bool keySPressed = false;
	static bool keyAPressed = false;
	static bool keyDPressed = false;

	static float ZoomFactor;


	template<typename T>
	static T LinearInterpolation(T a, T b, T t)
	{
		t = std::max((T)0, std::min((T)1, (T)t));
		return ((T)1 - t) * a + t * b;
	}
}

OrthoGraphicCamera::OrthoGraphicCamera(const OrthoCameraData& CameraData)
{
	SetCam(CameraData);
}

OrthoGraphicCamera::OrthoGraphicCamera(const OrthoGraphicCamera& camera)
{
	m_CameraData = camera.m_CameraData;
}


OrthoGraphicCamera OrthoGraphicCamera::operator=(const OrthoGraphicCamera& camera)
{
	return OrthoGraphicCamera(camera);
}

void OrthoGraphicCamera::SetCam(const OrthoCameraData& CameraData)
{
	m_CameraData = CameraData;

	m_Proj = glm::ortho(-m_CameraData.AspectRatio * m_CameraData.ZoomLevel, 
		m_CameraData.AspectRatio * m_CameraData.ZoomLevel, -m_CameraData.ZoomLevel, m_CameraData.ZoomLevel, -1.0f, 1.0f);
}

void OrthoGraphicCamera::Update(float dt)
{
	keyWPressed = InputManager::IsKeyDown(GLFW_KEY_W);
	keySPressed = InputManager::IsKeyDown(GLFW_KEY_S);
	keyAPressed = InputManager::IsKeyDown(GLFW_KEY_A);
	keyDPressed = InputManager::IsKeyDown(GLFW_KEY_D);

	glm::vec3 movement(0.0f);

	if (keyWPressed) 
		movement += m_CameraData.CameraSpeed * m_CameraData.Up * dt;
	if (keySPressed)
		movement -= m_CameraData.CameraSpeed * m_CameraData.Up * dt;
	if (keyAPressed)
		movement -= glm::normalize(glm::cross(m_CameraData.Front, m_CameraData.Up)) * m_CameraData.CameraSpeed * dt;
	if (keyDPressed)
		movement += glm::normalize(glm::cross(m_CameraData.Front, m_CameraData.Up)) * m_CameraData.CameraSpeed * dt;


	m_CameraData.Position += movement;

	double yOffset = InputManager::GetScrollWheel().second;
	if (InputManager::IsScrolled())
	{ 
		ZoomFactor += LinearInterpolation<float>(ZoomFactor, (float)yOffset * 0.25f, m_CameraData.ZoomSpeed);
		

		ZoomFactor = std::max(ZoomFactor, -10.0f);
		ZoomFactor = std::min(ZoomFactor, 10.0f);

		InputManager::SetScrolledToFalse();
	}
	else
	{
		ZoomFactor = 0;
	}

	m_CameraData.ZoomLevel -= ZoomFactor;

	m_CameraData.ZoomLevel = std::max(m_CameraData.ZoomLevel, 0.1f);
	m_CameraData.ZoomLevel = std::min(m_CameraData.ZoomLevel, 10.0f);
	

	m_View = glm::lookAt(m_CameraData.Position, m_CameraData.Position + m_CameraData.Front, m_CameraData.Up);
	m_Proj = glm::ortho(-m_CameraData.AspectRatio * m_CameraData.ZoomLevel, m_CameraData.AspectRatio * m_CameraData.ZoomLevel, -m_CameraData.ZoomLevel, m_CameraData.ZoomLevel, -1.0f, 1.0f);
}
