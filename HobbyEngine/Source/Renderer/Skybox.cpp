#include "Skybox.h"

#include <memory>
#include <string>

#include "../Engine/GLApplication.h"
#include "../ResourceCache/ImageLoader.h"
#include "../ResourceCache/ResourceCache.h"
#include "../Utils/DebugLogger.h"

Skybox::~Skybox()
{
	glDeleteTextures(1, &m_texture);
	glDeleteBuffers(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}

std::shared_ptr<ResHandle> loadFace(tinyxml2::XMLElement* elem) 
{
	auto filename = elem->Attribute("file");
	if (!filename) {
		LOG_DEBUG("Skybox::init: could not find file attributes for all faces in xml.");
		return std::shared_ptr<ResHandle>();
	}
	Resource resource(filename);
	return Game::instance().resourceCache().getHandle(resource);
}

bool Skybox::init(tinyxml2::XMLElement* root)
{
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	auto right = root->FirstChildElement("Right");
	auto left = root->FirstChildElement("Left");
	auto top = root->FirstChildElement("Top");
	auto bottom = root->FirstChildElement("Bottom");
	auto back = root->FirstChildElement("Back");
	auto front = root->FirstChildElement("Front");
	
	if (!right || !left || !top || !bottom || !back || !front) {
		LOG_DEBUG("Skybox::init: could not find elements for all faces in xml.");
		return false;
	}

	auto rightHandle = loadFace(right);
	auto leftHandle = loadFace(left);
	auto topHandle = loadFace(top);
	auto bottomHandle = loadFace(bottom);
	auto backHandle = loadFace(back);
	auto frontHandle = loadFace(front);

	if (!rightHandle || !leftHandle || !topHandle || !bottomHandle || !backHandle || !frontHandle) {
		LOG_DEBUG("Skybox::init: could not load resources for all faces.");
		return false;
	}

	auto rightData = std::static_pointer_cast<ImageResProcessedData>(rightHandle->processedData);
	auto leftData = std::static_pointer_cast<ImageResProcessedData>(leftHandle->processedData);
	auto topData = std::static_pointer_cast<ImageResProcessedData>(topHandle->processedData);
	auto bottomData = std::static_pointer_cast<ImageResProcessedData>(bottomHandle->processedData);
	auto backData = std::static_pointer_cast<ImageResProcessedData>(backHandle->processedData);
	auto frontData = std::static_pointer_cast<ImageResProcessedData>(frontHandle->processedData);

	if (!rightData || !leftData || !topData || !bottomData || !backData || !frontData) {
		LOG_DEBUG("Skybox::init: could not get handles for all face textures.");
		return false;
	}

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, rightData->width(), rightData->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, rightHandle->buffer);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, leftData->width(), leftData->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, leftHandle->buffer);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, topData->width(), topData->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, topHandle->buffer);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, bottomData->width(), bottomData->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, bottomHandle->buffer);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, backData->width(), backData->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, backHandle->buffer);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, frontData->width(), frontData->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, frontHandle->buffer);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return true;
}
