#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "ResourceCache.h"

class ModelResProcessedData : public IResProcessedData
{
public:
	ModelResProcessedData() {};
	ModelResProcessedData(std::vector<glm::vec3> vertices, std::vector<glm::vec2> uvs, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents, std::vector<glm::vec3> bitangents, std::vector<unsigned short> indices) :
		m_vertices(vertices), m_uvs(uvs), m_normals(normals), m_tangents(tangents), m_bitangents(bitangents), m_indices(indices) {}

	virtual std::string toString() { return std::string("ModelResProcessedData"); }

	std::vector<glm::vec3>& vertices() { return m_vertices; }
	std::vector<glm::vec2>& uvs() { return m_uvs; }
	std::vector<glm::vec3>& normals() { return m_normals; }
	std::vector<glm::vec3>& tangents() { return m_tangents; }
	std::vector<glm::vec3>& bitangents() { return m_bitangents; }
	std::vector<unsigned short>& indices() { return m_indices; }

private:
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec2> m_uvs;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_tangents;
	std::vector<glm::vec3> m_bitangents;
	std::vector<unsigned short> m_indices;
};

class ObjLoader : public IResLoader
{
public:
	virtual std::string getWildcard();
	virtual bool useRawFile();
	virtual size_t getLoadedResourceSize(char* rawBuffer, size_t rawSize);
	virtual bool isText();
	virtual bool loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle);
};

#endif // !MODEL_LOADER_H
