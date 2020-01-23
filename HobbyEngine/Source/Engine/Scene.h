#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <tinyxml2/tinyxml2.h>

#include "../GameObjects/GameObject.h"
#include "../Renderer/Camera.h"
#include "../Renderer/Skybox.h"
#include "../UI/UIElement.h"

struct SceneLighting {
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

class Scene
{
public:
	Scene() = default;
	~Scene();

	bool init(tinyxml2::XMLElement* data, GLFWwindow* window);

	bool update(int deltaTime);

	std::vector<SharedGOPtr>& gameObjects() { return m_gameObjects; }
	std::vector<std::shared_ptr<UIElement>>& uiElements() { return m_uiElements; }

	Camera& camera() { return m_camera; }
	SceneLighting& lighting() { return m_lighting; }
	std::shared_ptr<Skybox> skybox() { return m_skybox; }

	glm::mat4 lightSpaceMatrix();

private:
	std::vector<SharedGOPtr> m_gameObjects;
	std::vector<std::shared_ptr<UIElement>> m_uiElements;

	Camera m_camera;
	SceneLighting m_lighting;
	std::shared_ptr<Skybox> m_skybox;
};

#endif // !SCENE_H
