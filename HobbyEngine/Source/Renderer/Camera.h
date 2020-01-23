#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <tinyxml2/tinyxml2.h>

struct GLFWwindow;

// TODO: Camera should probably be a game object component to make it more flexible. This would especially
// enable writing the camera movement code in a LUA script instead of hard coding it into this class and
// thus make it easier to alter camera behaviour.
class Camera
{
public:
	bool init(tinyxml2::XMLElement* data, GLFWwindow* window);
	void update(int deltaTime);

	glm::mat4 viewMatrix();

	glm::vec3 front() { return m_front; }
	glm::vec3 position;
private:
	glm::vec3 m_front;

	GLFWwindow* m_window;

	double m_lastMouseX;
	double m_lastMouseY;
	float m_pitch;
	float m_yaw;

	// TODO: These should be defined in a config file OR in a LUA script for camera movement
	const float m_cameraSpeed = 0.01f;
	const float m_cameraSensitivity = 0.01f;
};

#endif // !CAMERA_H
