#include "TransformComponent.h"

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/transform.hpp>

#include "../Utils/DebugLogger.h"
#include "../Utils/XMLUtils.h"

bool TransformComponent::init(tinyxml2::XMLElement* data)
{
	auto positionElem = data->FirstChildElement("Position");
	auto rotationElem = data->FirstChildElement("Rotation");

	if (positionElem) {
		float x, y, z;
		if (!XMLUtils::xmlXYZAttribsToFloat(positionElem, x, y, z)) {
			return false;
		}

		position = glm::vec3(x, y, z);
	}

	if (rotationElem) {
		float x, y, z;
		if (!XMLUtils::xmlXYZAttribsToFloat(rotationElem, x, y, z)) {
			return false;
		}

		rotation = glm::vec3(x, y, z);
	}

	return true;
}

glm::mat4 TransformComponent::getTransformMatrix()
{
	glm::mat4 res = glm::mat4(1.0f);
	res = glm::translate(res, position);
	res = glm::rotate(res, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	res = glm::rotate(res, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	res = glm::rotate(res, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	return res;
}

IGOComponent* createTransformComponent()
{
	return new TransformComponent;
}
