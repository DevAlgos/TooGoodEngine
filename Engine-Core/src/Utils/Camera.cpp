#include <pch.h>
#include "Camera.h"
#include "Utils.h"

namespace
{
	template<typename T>
	static T LinearInterpolation(T a, T b, T t)
	{
		t = std::max((T)0, std::min((T)1, (T)t));
		return ((T)1 - t) * a + t * b;
	}
}

#pragma region ortho

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
	
	m_InverseProjection = glm::inverse(m_Proj);
}

void OrthoGraphicCamera::Update(float dt)
{
	keyWPressed = Input::IsKeyDown(GLFW_KEY_W);
	keySPressed = Input::IsKeyDown(GLFW_KEY_S);
	keyAPressed = Input::IsKeyDown(GLFW_KEY_A);
	keyDPressed = Input::IsKeyDown(GLFW_KEY_D);

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

	double yOffset = Input::GetScrollWheel().second;
	if (Input::IsScrolled())
	{ 
		ZoomFactor += LinearInterpolation<float>(ZoomFactor, (float)yOffset * 0.25f, m_CameraData.ZoomSpeed);
		

		ZoomFactor = std::max(ZoomFactor, -10.0f);
		ZoomFactor = std::min(ZoomFactor, 10.0f);

		Input::SetScrolledToFalse();
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

	m_InverseView =		  glm::inverse(m_View);
	m_InverseProjection = glm::inverse(m_Proj);
}

glm::vec2 OrthoGraphicCamera::GetMousePressCoordinates()
{
	double mouseX, mouseY;
	Input::GetMousePos(mouseX, mouseY);

	glm::vec4 VecCoords = glm::vec4(
		((float)mouseX * 2.0f / tge::Application::GetMainWindow().GetWidth())  - 1.0f,
		1.0f - ((float)mouseY * 2.0f / tge::Application::GetMainWindow().GetHeight()), 0.0f, 1.0f);

	glm::vec4 Coords = m_InverseView * m_InverseProjection * VecCoords;

	return Coords;
}
#pragma endregion ortho

Camera::Camera(const CameraData& data)
	: m_CameraData()
{
	SetCam(data);
}

Camera::Camera()
	: m_CameraData()
{
}

Camera::~Camera()
{
}

void Camera::Update(float dt)
{
	keyWPressed = Input::IsKeyDown(KEY_W);
	keySPressed = Input::IsKeyDown(KEY_S);
	keyAPressed = Input::IsKeyDown(KEY_A);
	keyDPressed = Input::IsKeyDown(KEY_D);
	keyQPressed = Input::IsKeyDown(KEY_Q);
	keyEPressed = Input::IsKeyDown(KEY_E);
	keyVPressed = Input::IsKeyPressed(KEY_V);

	if (keyVPressed)
	{
		if (CursorEnabled)
		{
			Input::EnableCursor();
			CursorEnabled = false;
		}
		else
		{
			Input::DisableCursor();
			CursorEnabled = true;
		}
	}

	if (!CursorEnabled)
		return;


	glm::vec3 movement(0.0f);

	if (keyWPressed)
		movement += m_CameraData.CameraSpeed * m_CameraData.Up * dt;
	if (keySPressed)
		movement -= m_CameraData.CameraSpeed * m_CameraData.Up * dt;
	if (keyAPressed)
		movement -= glm::normalize(glm::cross(m_CameraData.Front, m_CameraData.Up)) * m_CameraData.CameraSpeed * dt;
	if (keyDPressed)
		movement += glm::normalize(glm::cross(m_CameraData.Front, m_CameraData.Up)) * m_CameraData.CameraSpeed * dt;
	if (keyQPressed)
		movement += m_CameraData.CameraSpeed * m_CameraData.Front * dt;
	if (keyEPressed)
		movement -= m_CameraData.CameraSpeed * m_CameraData.Front * dt;
	

	double xPos = 0.0;
	double yPos = 0.0;

	Input::GetMousePos(xPos, yPos);

	float xOffset = (xPos - LastX) * m_CameraData.Sensitivity;
	float yOffset = (LastY - yPos) * m_CameraData.Sensitivity;

	LastX = xPos;
	LastY = yPos;

	Yaw += xOffset;
	Pitch += yOffset;

	Pitch = std::min(Pitch, 89.0f);
	Pitch = std::max(Pitch, -89.0f);

	glm::vec3 direction;
	direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	direction.y = sin(glm::radians(Pitch));
	direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	m_CameraData.Front = glm::normalize(direction);

	m_CameraData.Position += movement;

	m_Proj = glm::perspective(glm::radians(m_CameraData.Fov), m_CameraData.AspectRatio, m_CameraData.Near, m_CameraData.Far);
	m_View = glm::lookAt(m_CameraData.Position, m_CameraData.Position + m_CameraData.Front, m_CameraData.Up);

	m_InverseProjection = glm::inverse(m_Proj);
	m_InverseView = glm::inverse(m_View);
}

void Camera::SetCam(const CameraData& data)
{
	m_CameraData = data;

	m_Proj = glm::perspective(glm::radians(m_CameraData.Fov), m_CameraData.AspectRatio, m_CameraData.Near, m_CameraData.Far);
	m_View = glm::lookAt(m_CameraData.Position, m_CameraData.Position + m_CameraData.Front, m_CameraData.Up);

	m_InverseProjection = glm::inverse(m_Proj);
	m_InverseView = glm::inverse(m_View);
}
