#include "GameObject.h"

#include "LuaComponent.h"
#include "ParticleSystemComponent.h"
#include "RenderComponent.h"
#include "TransformComponent.h"
#include "../Utils/DebugLogger.h"

GameObject::~GameObject()
{
}

int GameObject::init(tinyxml2::XMLElement* data)
{
	return 1;
}

void GameObject::postInit()
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		auto component = it->second;
		component->postInit();
	}
}

void GameObject::destroy()
{
	m_components.clear();
}

void GameObject::update(int deltaTime)
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it) {
		it->second->update(deltaTime);
	}
}

void GameObject::addComponent(std::shared_ptr<IGOComponent> component)
{
	m_components.insert(std::make_pair(component->componentId(), component));
}

GOFactory::GOFactory()
{
	m_GOComponentCreators["TransformComponent"] = createTransformComponent;
	m_GOComponentCreators["LuaComponent"] = createLuaComponent; 
	m_GOComponentCreators["RenderComponent"] = createRenderComponent;
	m_GOComponentCreators["ParticleSystemComponent"] = createParticleSystemComponent;
}

GOFactory::~GOFactory()
{
}

std::shared_ptr<GameObject> GOFactory::createGameObject(const char* xml)
{
	tinyxml2::XMLError err;
	tinyxml2::XMLDocument doc;
	if ((err = doc.Parse(xml)) != tinyxml2::XML_SUCCESS) {
		LOG_DEBUG("GOFactory::createGameObject: Could not parse XML, error: " + err + std::string("\n") + xml);
		return std::shared_ptr<GameObject>();
	}
	tinyxml2::XMLElement* root = doc.FirstChildElement();

	std::shared_ptr<GameObject> gameObject(new GameObject(getNextGOId()));
	if (!gameObject->init(root)) {
		LOG_DEBUG("GOFactory::createGameObject: Could not initialize GameObject.")
		return std::shared_ptr<GameObject>();
	}

	auto components = root->FirstChildElement("Components");

	if (!components) {
		LOG_DEBUG("GOFactory::createGameObject: Couldn't find any components.");
		return std::shared_ptr<GameObject>();
	}

	for (tinyxml2::XMLElement* node = components->FirstChildElement(); node; node = node->NextSiblingElement()) {
		std::shared_ptr<IGOComponent> component(createComponent(node));
		if (component) {
			gameObject->addComponent(component);
			component->setOwner(gameObject);
		} else {
			LOG_DEBUG("GOFactory::createGameObject: Could not initialize component.");
			return std::shared_ptr<GameObject>();
		}
	}

	gameObject->postInit();

	return gameObject;
}

std::shared_ptr<IGOComponent> GOFactory::createComponent(tinyxml2::XMLElement* data)
{
	std::string name(data->Value());

	std::shared_ptr<IGOComponent> component;

	auto creatorIt = m_GOComponentCreators.find(name);
	if (creatorIt != m_GOComponentCreators.end()) {
		GOComponentCreator creator = creatorIt->second;
		component.reset(creator());
	}
	else {
		LOG_DEBUG("Could not find component: " + name);
		return std::shared_ptr<IGOComponent>();
	}

	if (!component->init(data)) {
		LOG_DEBUG("Could not initialize component: " + name);
		return std::shared_ptr<IGOComponent>();
	}

	return component;
}
