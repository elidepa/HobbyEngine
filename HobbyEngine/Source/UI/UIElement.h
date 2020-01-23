#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <tinyxml2/tinyxml2.h>

#include "Font.h"

// TODO: should this be just a std::string? Or maybe enum? What's the benefit of const char*?
typedef const char* UIElementType;

// TODO: elements should be able to have children, for example a button might have its text as a 
// child, this makes it easier to render everything in correct order as well as makes it possible
// to define element's position in relation to parent element
class UIElement
{
public:
	virtual bool init(tinyxml2::XMLElement* data) = 0;
	virtual UIElementType elementType() = 0;

	virtual void update(int deltaTime) {}

	uint32_t vao() { return m_VAO; }
	uint32_t vbo() { return m_VBO; }
	glm::vec2 position() { return m_position; }

protected:
	uint32_t m_VAO;
	uint32_t m_VBO;

	glm::vec2 m_position;
};

class UIElementFactory
{
public:
	std::shared_ptr<UIElement> createUIElement(tinyxml2::XMLElement* data);
};

#endif // !UI_ELEMENT_H
