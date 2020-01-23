#include "GLApplication.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../GameObjects/TransformComponent.h"
#include "../ResourceCache/FontLoader.h"
#include "../ResourceCache/ImageLoader.h"
#include "../ResourceCache/LuaLoader.h"
#include "../ResourceCache/ModelLoader.h"
#include "../ResourceCache/TextLoader.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/XMLUtils.h"

Game Game::gameInstance;

void Game::updateResolution(int width, int height)
{
	m_screenWidth = width;
	m_screenHeight = height;
	m_renderer->updateScreenSize(m_screenWidth, m_screenHeight);
}

Game::Game()
{
	m_resCache = new ResCache;
	m_goFactory = new GOFactory;
	m_inputSystem = new InputSystem;
	m_renderer = new Renderer;

	m_deltaTime = 0;
	m_lastFrame = 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
	game->updateResolution(width, height);
	glViewport(0, 0, width, height);
}

Game::~Game()
{
	if (m_resCache != nullptr) {
		delete m_resCache;
		m_resCache = nullptr;
	}

	if (m_goFactory != nullptr) {
		delete m_goFactory;
		m_goFactory = nullptr;
	}

	if (m_inputSystem != nullptr) {
		delete m_inputSystem;
		m_inputSystem = nullptr;
	}

	if (m_uiElementFactory != nullptr) {
		delete m_uiElementFactory;
		m_uiElementFactory = nullptr;
	}

	if (m_renderer != nullptr) {
		delete m_renderer;
		m_renderer = nullptr;
	}

	if (m_window != nullptr) {
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
}

bool Game::init()
{
	// Init resource cache
	std::shared_ptr<IResLoader> fontLoader(new FontLoader());
	std::shared_ptr<IResLoader> imageLoader(new ImageLoader());
	std::shared_ptr<IResLoader> luaLoader(new LuaLoader());
	std::shared_ptr<IResLoader> objLoader(new ObjLoader());
	std::shared_ptr<IResLoader> textLoader(new TextLoader());

	m_resCache->registerLoader(fontLoader);
	m_resCache->registerLoader(imageLoader);
	m_resCache->registerLoader(luaLoader);
	m_resCache->registerLoader(objLoader);
	m_resCache->registerLoader(textLoader);

	// Load game config
	tinyxml2::XMLDocument configDoc;
	if (!XMLUtils::loadXMLFile("GameConfig.xml", configDoc)) {
		LOG_DEBUG("Game::init: Could not load scene XML file.");
		return false;
	}
	auto configRoot = configDoc.FirstChildElement();

	// Init GLFW and create window

	auto resolutionElem = configRoot->FirstChildElement("ScreenSize");
	// Check if resolution element exists in config, if it does try to read width and height attributes, if any of these fail, use fallback resolution
	if (resolutionElem) {
		if (!XMLUtils::xmlAttribToInt(resolutionElem, "width", m_screenWidth) || !XMLUtils::xmlAttribToInt(resolutionElem, "height", m_screenHeight)) {
			LOG_DEBUG("Game::init: found ScreenSize element in game config but could not get width or height attributes")
		}
	}
	else {
		m_screenWidth = 1600;
		m_screenHeight = 1200;
	}

	if (glfwInit() == GLFW_FALSE) {
		LOG_DEBUG("Could not initialize GLFW");
		return false;
	}

	if (!initWindow()) {
		return false;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		LOG_DEBUG("Failed to initialize GLAD");
		glfwTerminate();
		return false;
	}

	glViewport(0, 0, m_screenWidth, m_screenHeight);

	glfwSetWindowUserPointer(m_window, this);

	glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

	m_inputSystem->init(m_window);

	// Init UI element factory

	m_uiElementFactory = new UIElementFactory;

	// Init renderer
	m_renderer->init(m_screenWidth, m_screenHeight);

	// Load scene
	tinyxml2::XMLDocument sceneDoc;
	if (!XMLUtils::loadXMLFile("Scenes/scene_1.xml", sceneDoc)) {
		LOG_DEBUG("Game::init: Could not load scene XML file.");
		return false;
	}
	auto sceneRoot = sceneDoc.FirstChildElement();

	return m_scene.init(sceneRoot, m_window);;
}

bool gamma = false;

void Game::processInput()
{
	if (m_inputSystem->isPressed(GLFW_KEY_ESCAPE)) {
		glfwSetWindowShouldClose(m_window, true);
	}
}

int Game::run()
{
	m_lastFrame = glfwGetTime();
	while (!glfwWindowShouldClose(m_window)) {
		double currentFrame = glfwGetTime();
		m_deltaTime = currentFrame - m_lastFrame;
		m_lastFrame = currentFrame;

		int deltaTimeMillis = (m_deltaTime * 1000);

		processInput();

		m_scene.update(deltaTimeMillis);

		m_renderer->renderScene(m_scene);

		glfwPollEvents();
	}

	glfwTerminate();

	return 1;
}

int Game::initWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	m_window = glfwCreateWindow(m_screenWidth, m_screenHeight, "HobbyEngine", nullptr, nullptr);

	if (m_window == nullptr) {
		LOG_DEBUG("Failed to create window");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(m_window);

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return 1;
}
