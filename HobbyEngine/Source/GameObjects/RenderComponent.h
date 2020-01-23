#ifndef RENDER_COMPONENT_H
#define RENDER_COMPONENT_H

#include <glm/glm.hpp>
#include <tinyxml2/tinyxml2.h>

#include "GameObject.h"

struct Material {
	Material() = default;
	~Material();

	uint32_t diffuseMap;
	uint32_t specularMap;
	uint32_t reflectionMap;
	float shininess;
};

class RenderComponent : public IGOComponent
{
public:
	RenderComponent() = default;
	~RenderComponent();

	virtual bool init(tinyxml2::XMLElement* data);

	virtual ComponentId componentId() const { return COMPONENT_ID; }

	uint32_t vao() { return m_VAO; }
	uint32_t vbo() { return m_VBO; }
	uint32_t uvs() { return m_uvBuffer; }
	uint32_t normals() { return m_normalBuffer; }
	uint32_t tangents() { return m_tangentBuffer; }
	uint32_t bitangents() { return m_bitangentBuffer; }
	uint32_t ebo() { return m_EBO; }

	uint32_t normalMap() { return m_normalMap; }

	Material& material() { return m_material; }

	int nIndices() { return m_nIndices; }

private:
	bool loadTexture(tinyxml2::XMLElement* elem, uint32_t& buffer);

	const ComponentId COMPONENT_ID = "RenderComponent";

	uint32_t m_VAO;
	uint32_t m_VBO;
	uint32_t m_uvBuffer;
	uint32_t m_normalBuffer;
	uint32_t m_tangentBuffer;
	uint32_t m_bitangentBuffer;
	uint32_t m_EBO;

	uint32_t m_normalMap;

	Material m_material;

	int m_nIndices;

};

IGOComponent* createRenderComponent();

#endif // !RENDER_COMPONENT_H
