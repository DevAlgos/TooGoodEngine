#pragma once

#include <glm/glm.hpp>
#include "InputManager.h"

#pragma region ortho
struct OrthoCameraData
{
		//Positional Data
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;

		//Projecion Data
		float AspectRatio = 1.0f; //default 1:1 Aspect Ratio
		float ZoomLevel = 2.5f;  //default Zoom
		float CameraSpeed = 2.0f; //default CamerSpeed
		float ZoomSpeed = 1.0f; //default Zoom Speed
};

class OrthoGraphicCamera
{
public:
	OrthoGraphicCamera(const OrthoCameraData& CameraData);
	OrthoGraphicCamera(const OrthoGraphicCamera& camera);
	OrthoGraphicCamera() = default;
	~OrthoGraphicCamera() = default;

	OrthoGraphicCamera operator=(const OrthoGraphicCamera& camera);

	void SetCam(const OrthoCameraData& CameraData);

	void Update(float dt);

	glm::vec2 GetMousePressCoordinates();

	inline glm::vec3 GetPosition() { return m_CameraData.Position; }
	inline glm::vec3 GetFront() { return m_CameraData.Front; }
	inline glm::vec3 GetUp() { return m_CameraData.Up; }

	inline glm::mat4 GetProjection() { return m_Proj; }
	inline glm::mat4 GetView() { return m_View; }

	inline glm::mat4 GetInverseProjection() { return m_InverseProjection; }
	inline glm::mat4 GetInverseView() { return m_InverseView; }


private:
	OrthoCameraData m_CameraData;

	glm::mat4 m_Proj;
	glm::mat4 m_View;

	glm::mat4 m_InverseProjection;
	glm::mat4 m_InverseView;

	bool keyWPressed = false;
	bool keySPressed = false;
	bool keyAPressed = false;
	bool keyDPressed = false;

	float ZoomFactor;

};
#pragma endregion ortho

struct CameraData
{
	float Fov = 60.0f;
	float AspectRatio = 16.0f / 9.0f;
	float Near = 0.1f;
	float Far = 100.0f;

	glm::vec3 Position = {0.0f, 0.0f, 0.0f};
	glm::vec3 Front = {0.0f, 0.0f, -1.0f};
	glm::vec3 Up = {0.0f, 1.0f, 0.0f};

	float CameraSpeed = 1.0f;
	float ZoomSpeed = 1.0f;
	float Sensitivity = 0.1f;
};

class Camera
{
public:
	Camera(const CameraData& data);
	Camera();
	~Camera();

	void Update(float dt);

	void SetCam(const CameraData& data);

	inline glm::vec3 GetPosition() { return m_CameraData.Position; }
	inline glm::vec3 GetFront() { return m_CameraData.Front; }
	inline glm::vec3 GetUp() { return m_CameraData.Up; }

	inline glm::mat4 GetProjection() { return m_Proj; }
	inline glm::mat4 GetView() { return m_View; }

	inline glm::mat4 GetInverseProjection() { return m_InverseProjection; }
	inline glm::mat4 GetInverseView() { return m_InverseView; }

private:
	CameraData m_CameraData;

	glm::mat4 m_Proj;
	glm::mat4 m_View;

	glm::mat4 m_InverseProjection;
	glm::mat4 m_InverseView;

	bool keyWPressed = false;
	bool keySPressed = false;
	bool keyAPressed = false;
	bool keyDPressed = false;
	bool keyQPressed = false;
	bool keyEPressed = false;
	bool keyVPressed = false;

	bool CursorEnabled = true;

	double LastX = 0.0;
	double LastY = 0.0;

	float Pitch = 0.0f;
	float Yaw = 0.0f;
};