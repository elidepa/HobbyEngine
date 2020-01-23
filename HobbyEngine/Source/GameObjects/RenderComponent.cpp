#include "RenderComponent.h"

#include <memory>

#include "../Engine/GLApplication.h"
#include "../ResourceCache/ImageLoader.h"
#include "../ResourceCache/ModelLoader.h"
#include "../Utils/DebugLogger.h"

bool RenderComponent::loadTexture(tinyxml2::XMLElement* elem, uint32_t& buffer)
{
	auto texturePath = elem->Attribute("file");

	if (!texturePath) {
		LOG_DEBUG("RenderComponent::loadTexture: could not load texture - missing texture file attribute");
		return false;
	}

	Resource textureResource(texturePath);
	auto textureHandle = Game::instance().resourceCache().getHandle(textureResource);

	if (!textureHandle) {
		LOG_DEBUG("RenderComponent::loadTexture: could not load texture - could not get texture file handle");
		return false;
	}

	glGenTextures(1, &buffer);
	glBindTexture(GL_TEXTURE_2D, buffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	auto textureProcessedData = std::dynamic_pointer_cast<ImageResProcessedData>(textureHandle->processedData);

	// TODO: Sometimes when the same texture is used this throws a read access violation from reading the textureHandle->buffer
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureProcessedData->width(), textureProcessedData->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, textureHandle->buffer);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

RenderComponent::~RenderComponent()
{
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_uvBuffer);
	glDeleteBuffers(1, &m_normalBuffer);
	glDeleteBuffers(1, &m_tangentBuffer);
	glDeleteBuffers(1, &m_bitangentBuffer);
	glDeleteBuffers(1, &m_EBO);
	glDeleteTextures(1, &m_normalMap);
	glDeleteVertexArrays(1, &m_VAO);
}

bool RenderComponent::init(tinyxml2::XMLElement* data)
{
	auto modelData = data->FirstChildElement("Model");

	if (!modelData) {
		LOG_DEBUG("RenderComponent::init: could not initialize component - missing Model tag");
		return false;
	}

	auto modelPath = modelData->Attribute("file");

	if (!modelPath) {
		LOG_DEBUG("RenderComponent::init: could not initialize component - missing model file attribute in model tag");
		return false;
	}

	Resource modelResource(modelPath);
	auto modelHandle = Game::instance().resourceCache().getHandle(modelResource);

	if (!modelHandle) {
		LOG_DEBUG("RenderComponent::init: could not initialize component - could not get model file handle");
		return false;
	}

	std::shared_ptr<ModelResProcessedData> processedModelData = std::dynamic_pointer_cast<ModelResProcessedData>(modelHandle->processedData);

	auto vertices = processedModelData->vertices();
	auto uvs = processedModelData->uvs();
	auto normals = processedModelData->normals();
	auto tangents = processedModelData->tangents();
	auto bitangents = processedModelData->bitangents();
	auto indices = processedModelData->indices();

	glGenVertexArrays(1, &m_VAO);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_tangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_tangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_bitangentBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_bitangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	m_nIndices = processedModelData->indices().size();

	auto normalMapData = data->FirstChildElement("NormalMap");

	if (!normalMapData) {
		LOG_DEBUG("RenderComponent::init: could not initialize component - could not find valid normal map element.");
		return false;
	}

	if (!loadTexture(normalMapData, m_normalMap)) {
		LOG_DEBUG("RenderComponent::init: could not initialize component - could not load normal map.");
		return false;
	}

	auto materialData = data->FirstChildElement("Material");
	if (materialData) {
		auto diffuseMap = materialData->FirstChildElement("DiffuseMap");
		auto specularMap = materialData->FirstChildElement("SpecularMap");
		auto shininess = materialData->FirstChildElement("Shininess");
		auto reflectionMap = materialData->FirstChildElement("ReflectionMap");

		if (!diffuseMap || !specularMap || !reflectionMap || !shininess || !shininess->Attribute("value")) {
			LOG_DEBUG("RenderComponent::init: could not initialize component - could not find valid diffuse, specular and reflection map or shininess elements in material data");
			return false;
		}

		if (!loadTexture(diffuseMap, m_material.diffuseMap) || !loadTexture(specularMap, m_material.specularMap) || !loadTexture(reflectionMap, m_material.reflectionMap)) {
			LOG_DEBUG("RenderComponent::init: could not initialize component - could not load diffuse, specular and reflection maps");
			return false;
		}

		try
		{
			m_material.shininess = std::stof(shininess->Attribute("value"));
		}
		catch (const std::exception&)
		{
			LOG_DEBUG("RenderComponent::init: could not initialize component - could not convert shininess value to float");
			return false;
		}

	}

	return true;
}

IGOComponent* createRenderComponent()
{
	return new RenderComponent;
}

Material::~Material()
{
	glDeleteTextures(1, &diffuseMap);
	glDeleteTextures(1, &specularMap);
}
