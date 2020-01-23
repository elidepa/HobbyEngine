#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../Engine/GLApplication.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/XMLUtils.h"

bool Camera::init(tinyxml2::XMLElement* data, GLFWwindow* window)
{
	m_window = window;

	auto positionElem = data->FirstChildElement("Position");
	auto rotationElem = data->FirstChildElement("Rotation");

	if (!positionElem || !rotationElem) {
		LOG_DEBUG("Camera::init: Position or rotation element missing");
		return false;
	}

	float x, y, z;
	bool success = XMLUtils::xmlXYZAttribsToFloat(positionElem, x, y, z);
	if (!success) {
		LOG_DEBUG("Camera::init: Could not convert position element data to floats");
		return false;
	}
	position = glm::vec3(x, y, z);

	success = XMLUtils::xmlAttribToFloat(rotationElem, "yaw", m_yaw);
	if (!success) {
		return false;
	}

	success = XMLUtils::xmlAttribToFloat(rotationElem, "pitch", m_pitch);
	if (!success) {
		return false;
	}

	glm::vec3 front;
	front.x = glm::cos(glm::radians(m_pitch)) * glm::cos(glm::radians(m_yaw));
	front.y = glm::sin(glm::radians(m_pitch));
	front.z = glm::cos(glm::radians(m_pitch)) * glm::sin(glm::radians(m_yaw));
	m_front = glm::normalize(front);

	return true;
}

void Camera::update(int deltaTime)
{
	InputSystem& input = Game::instance().inputSystem();

	float cameraSpeed = deltaTime * m_cameraSpeed;
	float lookSpeed = deltaTime * m_cameraSensitivity;

	// Camera direction on the XZ plane
	glm::vec2 cameraDirection = glm::normalize(glm::vec2(m_front.x, m_front.z));

	if (input.isPressed(GLFW_KEY_W)) {
		position.x += cameraSpeed * cameraDirection.x;
		position.z += cameraSpeed * cameraDirection.y;
	}
	if (input.isPressed(GLFW_KEY_S)) {
		position.x -= cameraSpeed * cameraDirection.x;
		position.z -= cameraSpeed * cameraDirection.y;
	}
	if (input.isPressed(GLFW_KEY_A)) {
		position -= glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
	}
	if (input.isPressed(GLFW_KEY_D)) {
		position += glm::normalize(glm::cross(m_front, glm::vec3(0.0f, 1.0f, 0.0f))) * cameraSpeed;
	}

	double mouseX, mouseY;

	input.getMousePos(mouseX, mouseY);

	double xoffset = mouseX - m_lastMouseX;
	double yoffset = -(mouseY - m_lastMouseY);

	m_yaw += xoffset * lookSpeed;
	m_pitch += yoffset * lookSpeed;

	if (m_pitch > 89.0f) {
		m_pitch = 89.0f;
	}
	if (m_pitch < -89.0f) {
		m_pitch = -89.0f;
	}

	glm::vec3 front;
	front.x = glm::cos(glm::radians(m_pitch)) * glm::cos(glm::radians(m_yaw));
	front.y = glm::sin(glm::radians(m_pitch));
	front.z = glm::cos(glm::radians(m_pitch)) * glm::sin(glm::radians(m_yaw));
	m_front = glm::normalize(front);

	m_lastMouseX = mouseX;
	m_lastMouseY = mouseY;
}

glm::mat4 Camera::viewMatrix()
{
	glm::vec3 direction = position + m_front;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	return glm::lookAt(position, direction, up);
}
