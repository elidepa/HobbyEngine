#include "TextElement.h"

#include "../Engine/GLApplication.h"
#include "../ResourceCache/FontLoader.h"
#include "../ResourceCache/ResourceCache.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/XMLUtils.h"

bool TextElement::init(tinyxml2::XMLElement* data)
{
	float x, y;

	if (!data->Attribute("x") && !data->Attribute("y")) {
		LOG_DEBUG("TextElement::init: could not init text element - missing x or y attributes.");
		return false;
	}

	if (!XMLUtils::xmlAttribToFloat(data, "x", x) || !XMLUtils::xmlAttribToFloat(data, "y", y)) {
		return false;
	}

	m_position = glm::vec2(x, y);

	if (!XMLUtils::xmlAttribToFloat(data, "scale", m_textScale)) {
		m_textScale = 1.0f;
	}

	auto fontFile = data->Attribute("font");
	if (!fontFile) {
		LOG_DEBUG("TextElement::init: could not init text element - missing font attribute.");
		return false;
	}

	Resource fontResource = Resource(fontFile);
	auto fontHandle = Game::instance().resourceCache().getHandle(fontResource);

	if (!fontHandle) {
		LOG_DEBUG("TextElement::init: could not init text element - could not get font handle.");
		return false;
	}

	auto fontData = std::static_pointer_cast<FontResProcessedData>(fontHandle->processedData);
	m_font = fontData->font();

	auto colorElement = data->FirstChildElement("Color");
	float r, g, b, a;
	if (!colorElement || !XMLUtils::xmlRGBAAttribsToFloat(colorElement, r, g, b, a)) {
		m_color = glm::vec4(1.0, 0.0, 0.0, 1.0);
	}
	else 
	{
		m_color = glm::vec4(r, g, b, a);
	}

	auto scriptElement = data->FirstChildElement("Script");
	if (scriptElement) {
		auto scriptFile = scriptElement->Attribute("file");

		if (!scriptFile) {
			LOG_DEBUG("TextElement::init: found script element but no file attribute.");
			return false;
		}

		Resource scriptResource(scriptFile);
		auto scriptHandle = Game::instance().resourceCache().getHandle(scriptResource);

		if (!scriptHandle) {
			LOG_DEBUG("TextElement::init: found script element and file attribute, but could not load the script.");
			return false;
		}

		initLuaState();

		const char* scriptText = scriptHandle->buffer;
		auto res = m_luaState->script(scriptText);

		m_luaUpdate = (*m_luaState)["update"];

		if (!m_luaUpdate) {
			LOG_DEBUG("TextElement::init: could not init lua script - no update function in script");
			return false;
		}
	}

	auto textElem = data->FirstChildElement("Text");

	if (!textElem) {
		LOG_DEBUG("TextElement::init: could not init text element - could not get XML text element.");
		return false;
	}

	if (data->Attribute("text")) {
		LOG_DEBUG("TextElement::init: could not init text element - could not get text attribute.");
		return false;
	}

	m_text = std::string(textElem->Attribute("text"));

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return true;
}

void TextElement::update(int deltaTime)
{
	if (m_luaState.get()) {
		m_luaUpdate(deltaTime);
	}
}

bool TextElement::initLuaState()
{
	m_luaState = std::unique_ptr<sol::state>(new sol::state);
	m_luaState->open_libraries(sol::lib::base, sol::lib::math, sol::lib::string);

	m_luaState->set_function("updateText", &TextElement::updateText, this);
	m_luaState->set_function("lollero", &TextElement::lollero, this);

	return true;
}

void TextElement::updateText(std::string str)
{
	m_text = str;
}

void TextElement::lollero(int i)
{
	LOG_DEBUG(std::to_string(i));
}
