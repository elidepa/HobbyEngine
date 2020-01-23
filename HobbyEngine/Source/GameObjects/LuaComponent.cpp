#include "LuaComponent.h"

#include <glm/glm.hpp>

#include "../Engine/GLApplication.h"
#include "../ResourceCache/ResourceCache.h"
#include "ParticleSystemComponent.h"
#include "TransformComponent.h"
#include "../Utils/DebugLogger.h"

bool LuaComponent::init(tinyxml2::XMLElement* data)
{
	auto script = data->FirstChildElement("Script");

	if (!script) {
		LOG_DEBUG("LuaComponent::init: could not init component - missing script tag");
		return false;
	}

	auto scriptFile = script->Attribute("file");

	if (!scriptFile) {
		LOG_DEBUG("LuaComponent::init: could not init component - missing script file attribute in script tag");
		return false;
	}

	Resource scriptResource(scriptFile);
	auto scriptHandle = Game::instance().resourceCache().getHandle(scriptResource);

	if (!scriptHandle) {
		LOG_DEBUG("LuaComponent::init: could not init component - could not get script resource handle");
		return false;
	}

	initLuaState();

	const char* scriptText = scriptHandle->buffer;
	auto res = m_state.script(scriptText);

	m_luaUpdate = m_state["update"];

	if (!m_luaUpdate) {
		LOG_DEBUG("LuaComponent::init: could not init component - no update function in script");
		return false;
	}

	return true;
}

void LuaComponent::update(int deltaTime)
{
	m_luaUpdate(deltaTime);
}

bool LuaComponent::initLuaState()
{
	m_state.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);

	m_state.set_function("move", &LuaComponent::move, this);
	m_state.set_function("rotate", &LuaComponent::rotate, this);
	m_state.set_function("debugLog", &LuaComponent::debugLog, this);
	m_state.set_function("activateParticles", &LuaComponent::activateParticles, this);
	m_state.set_function("stopParticles", &LuaComponent::stopParticles, this);
	m_state.set_function("getPosition", &LuaComponent::getPosition, this);

	return true;
}

bool LuaComponent::move(float x, float y, float z)
{
	auto weak_transformComponent = m_owner->findComponent<TransformComponent>("TransformComponent");

	if (weak_transformComponent.expired()) {
		return false;
	}

	auto transformComponent = weak_transformComponent.lock();

	auto position = transformComponent->position;
	transformComponent->position = glm::vec3(position.x + x, position.y + y, position.z + z);

	return true;
}

bool LuaComponent::rotate(float x, float y, float z)
{
	auto weak_transformComponent = m_owner->findComponent<TransformComponent>("TransformComponent");

	if (weak_transformComponent.expired()) {
		return false;
	}

	auto transformComponent = weak_transformComponent.lock();

	auto rotation = transformComponent->rotation;
	transformComponent->rotation = glm::vec3(glm::mod(rotation.x + x, 360.0f), glm::mod(rotation.y + y, 360.0f), glm::mod(rotation.z + z, 360.0f));

	return true;
}

void LuaComponent::debugLog(std::string msg)
{
	LOG_DEBUG(msg);
}

void LuaComponent::activateParticles()
{
	auto particleSystem = m_owner->findComponent<ParticleSystemComponent>("ParticleSystemComponent").lock();
	if (particleSystem) {
		particleSystem->activate();
	}
}

void LuaComponent::stopParticles()
{
	auto particleSystem = m_owner->findComponent<ParticleSystemComponent>("ParticleSystemComponent").lock();
	if (particleSystem) {
		particleSystem->stop();
	}
}

sol::variadic_results LuaComponent::getPosition()
{
	auto transformComponent = m_owner->findComponent<TransformComponent>("TransformComponent").lock();

	if (!transformComponent) {
		return sol::variadic_results();
	}

	sol::variadic_results res;
	res.push_back({m_state, sol::in_place_type<float>, transformComponent->position.x });
	res.push_back({m_state, sol::in_place_type<float>, transformComponent->position.y });
	res.push_back({m_state, sol::in_place_type<float>, transformComponent->position.z });

	return res;
}

IGOComponent* createLuaComponent()
{
	return new LuaComponent;
}
