#include "InputSystem.h"

void InputSystem::init(GLFWwindow* window)
{
	m_window = window;
}

bool InputSystem::isPressed(int key)
{
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

void InputSystem::getMousePos(double& xpos, double& ypos)
{
	glfwGetCursorPos(m_window, &xpos, &ypos);
}
