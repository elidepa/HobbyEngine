#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <GLFW/glfw3.h>

class InputSystem
{
public:
	void init(GLFWwindow* window);

	bool isPressed(int key);
	void getMousePos(double& xpos, double& ypos);

private:
	GLFWwindow* m_window;
};

#endif // !INPUT_SYSTEM_H
