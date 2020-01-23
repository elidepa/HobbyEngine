#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <glm/glm.hpp>
#include <tinyxml2/tinyxml2.h>

#include "GameObject.h"

class TransformComponent : public IGOComponent
{
public:
	virtual bool init(tinyxml2::XMLElement* data);
	virtual ComponentId componentId() const { return COMPONENT_ID; }

	glm::mat4 getTransformMatrix();

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
private:
	const ComponentId COMPONENT_ID = "TransformComponent";
};

IGOComponent* createTransformComponent();

#endif // !TRANSFORM_COMPONENT_H