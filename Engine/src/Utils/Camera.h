#pragma once

#include <glm/glm.hpp>
#include "InputManager.h"


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

	inline glm::vec3 GetPosition() { return m_CameraData.Position; }
	inline glm::vec3 GetFront() { return m_CameraData.Front; }
	inline glm::vec3 GetUp() { return m_CameraData.Up; }

	inline glm::mat4 GetProjection() { return m_Proj; }
	inline glm::mat4 GetView() { return m_View; }


private:
	OrthoCameraData m_CameraData;

	glm::mat4 m_Proj;
	glm::mat4 m_View;

};
