#ifndef GL_APPLICATION_H
#define GL_APPLICATION_H

#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../GameObjects/GameObject.h"
#include "InputSystem.h"
#include "../ResourceCache/ResourceCache.h"
#include "../Renderer/Renderer.h"
#include "Scene.h"
#include "../UI/UIElement.h"

class Game
{
public:
	static Game& instance() { return gameInstance; }
	~Game();

	bool init();
	int run();

	ResCache& resourceCache() { return *m_resCache; }
	GOFactory& goFactory() { return *m_goFactory; }
	InputSystem& inputSystem() { return *m_inputSystem; }
	UIElementFactory& uiElementFactory() { return *m_uiElementFactory; }

	void updateResolution(int width, int height);

	void swapBuffers() { glfwSwapBuffers(m_window); }

private:
	Game();
	int initWindow();
	void processInput();

	static Game gameInstance;

	ResCache* m_resCache;
	GOFactory* m_goFactory;
	InputSystem* m_inputSystem;
	UIElementFactory* m_uiElementFactory;

	Renderer* m_renderer;

	double m_deltaTime;
	double m_lastFrame;

	Scene m_scene;

	GLFWwindow* m_window;

	int m_screenWidth;
	int m_screenHeight;
};

#endif // !GL_APPLICATION_H

