#include "Scene.h"

#include <string>

#include <glm/gtc/matrix_transform.hpp>

#include "GLApplication.h"
#include "../ResourceCache/ResourceCache.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/XMLUtils.h"

Scene::~Scene()
{
	m_gameObjects.clear();
}

bool Scene::init(tinyxml2::XMLElement* data, GLFWwindow* window)
{
	auto cameraElement = data->FirstChildElement("Camera");

	if (!cameraElement) {
		LOG_DEBUG("Scene::init: could not init scene - no camera element");
		return false;
	}

	if (!m_camera.init(cameraElement, window)) {
		return false;
	}

	auto skyboxElement = data->FirstChildElement("Skybox");

	if (skyboxElement) {
		Skybox* skybox = new Skybox();
		m_skybox = std::shared_ptr<Skybox>(skybox);
		if (!m_skybox->init(skyboxElement)) {
			return false;
		}
	}

	auto goFactory = Game::instance().goFactory();

	auto goElements = data->FirstChildElement("GameObjects");

	if (!goElements) {
		LOG_DEBUG("Scene::init: could not init scene - no GameObjects element");
		return false;
	}

	for (auto goElem = goElements->FirstChildElement("GameObject"); goElem; goElem = goElem->NextSiblingElement()) {
		auto goFile = goElem->Attribute("file");
		if (!goFile) {
			LOG_DEBUG("Scene::init: could not init scene - GameObject missing file attribute");
			return false;
		}

		Resource goXmlResource(goFile);
		auto goXmlHandle = Game::instance().resourceCache().getHandle(goXmlResource);

		if (!goXmlHandle) {
			LOG_DEBUG("Scene::init: could not init scene - could not load game object file " + std::string(goFile));
			return false;
		}

		auto xml = goXmlHandle->buffer;
		auto gameObject = goFactory.createGameObject(xml);

		if (!gameObject) {
			LOG_DEBUG("Scene::init: could not init scene - could not create game object " + std::string(goFile));
			return false;
		}

		gameObject->scene = std::shared_ptr<Scene>(this);

		m_gameObjects.push_back(gameObject);
	}

	auto uiElementFactory = Game::instance().uiElementFactory();

	auto uiElements = data->FirstChildElement("UIElements");
	if (uiElements) {
		for (auto uiElem = uiElements->FirstChildElement(); uiElem; uiElem = uiElem->NextSiblingElement()) {
			auto element = uiElementFactory.createUIElement(uiElem);
			if (element) {
				m_uiElements.push_back(element); 
			}
		}
	}

	auto lightingElement = data->FirstChildElement("Lighting");

	if (lightingElement) {
		auto directionXml = lightingElement->FirstChildElement("Direction");
		auto ambientXml = lightingElement->FirstChildElement("Ambient");
		auto diffuseXml = lightingElement->FirstChildElement("Diffuse");
		auto specularXml = lightingElement->FirstChildElement("Specular");

		if (!directionXml || !ambientXml || !diffuseXml || !specularXml) {
			LOG_DEBUG("Scene::init: could not init scene - lighting data missing an element");
			return false;
		}
		
		float x, y, z;
		bool success = XMLUtils::xmlXYZAttribsToFloat(directionXml, x, y, z);
		if (!success) {
			return false;
		}
		m_lighting.direction = glm::vec3(x, y, z);

		success = XMLUtils::xmlXYZAttribsToFloat(ambientXml, x, y, z);
		if (!success) {
			return false;
		}
		m_lighting.ambient = glm::vec3(x, y, z);

		success = XMLUtils::xmlXYZAttribsToFloat(diffuseXml, x, y, z);
		if (!success) {
			return false;
		}
		m_lighting.diffuse = glm::vec3(x, y, z);

		success = XMLUtils::xmlXYZAttribsToFloat(specularXml, x, y, z);
		if (!success) {
			return false;
		}
		m_lighting.specular = glm::vec3(x, y, z);
	}

	return true;
}

bool Scene::update(int deltaTime)
{
	m_camera.update(deltaTime);

	for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); ++it) {
		(*it)->update(deltaTime);
	}
	
	for (auto it = m_uiElements.begin(); it != m_uiElements.end(); ++it) {
		(*it)->update(deltaTime);
	}

	return true;
}

glm::mat4 Scene::lightSpaceMatrix()
{
	glm::vec3 cameraPosition = glm::vec3(m_camera.position.x, 0.0f, m_camera.position.z);
	glm::mat4 view = glm::lookAt(cameraPosition + m_lighting.direction * -1.0f, cameraPosition, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 projection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, -10.0f, 20.0f);

	return projection * view;
}
