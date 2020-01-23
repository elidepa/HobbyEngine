#ifndef LUA_COMPONENT_H
#define LUA_COMPONENT_H

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <tinyxml2/tinyxml2.h>

#include "GameObject.h"

class LuaComponent : public IGOComponent
{
public:
	virtual bool init(tinyxml2::XMLElement* data);
	virtual ComponentId componentId() const { return COMPONENT_ID; }

	virtual void update(int deltaTime);

private:
	const ComponentId COMPONENT_ID = "LuaComponent";
	sol::state m_state;

	bool initLuaState();

	// Lua update function
	sol::function m_luaUpdate;

	// Lua API
	bool move(float x, float y, float z);
	bool rotate(float x, float y, float z);

	void debugLog(std::string msg);

	void activateParticles();
	void stopParticles();

	sol::variadic_results getPosition();
};

IGOComponent* createLuaComponent();

#endif // !LUA_COMPONENT_H
