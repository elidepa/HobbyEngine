#ifndef TEXT_ELEMENT_H
#define TEXT_ELEMENT_H

#include <memory>
#include <string>

#include <glm/glm.hpp>
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <tinyxml2/tinyxml2.h>

#include "Font.h"
#include "UIElement.h"

// TODO: update only a part of the text with the script

class TextElement : public UIElement
{
public:
	virtual bool init(tinyxml2::XMLElement* data);
	virtual UIElementType elementType() { return m_type; }

	virtual void update(int deltaTime) override;

	std::string text() { return m_text; }
	glm::vec4 color() { return m_color; }
	float textScale() { return m_textScale; }
	std::shared_ptr<Font> font() { return m_font; }
private:
	bool initLuaState();

	// Lua API
	void updateText(std::string str);
	void lollero(int i);

	UIElementType m_type = "TextElement";

	std::unique_ptr<sol::state> m_luaState;
	sol::function m_luaUpdate;

	std::string m_text;
	glm::vec4 m_color;
	float m_textScale;
	std::shared_ptr<Font> m_font;
};

#endif // !TEXT_ELEMENT_H
