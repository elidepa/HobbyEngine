#include "ModelLoader.h"

#include <map>
#include <regex>
#include <sstream>
#include <vector>

#include "../Utils/DebugLogger.h"

std::string ObjLoader::getWildcard()
{
	return std::string(".*(\\.obj)");
}

bool ObjLoader::useRawFile()
{
	return false;
}

size_t ObjLoader::getLoadedResourceSize(char* rawBuffer, size_t rawSize)
{
	return rawSize;
}

bool ObjLoader::isText()
{
	return true;
}

struct PackedVertex {
	glm::vec3 vertex;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

static const std::regex vRgx("v (-?\\d+\.?\\d*) (-?\\d+\.?\\d*) (-?\\d+\.?\\d*)");
static const std::regex vtRgx("vt (-?\\d+\.?\\d*) (-?\\d+\.?\\d*)");
static const std::regex vnRgx("vn (-?\\d+\.?\\d*) (-?\\d+\.?\\d*) (-?\\d+\.?\\d*)");
static const std::regex fRgx("f (\\d+)\\/(\\d+)\\/(\\d+) (\\d+)\\/(\\d+)\\/(\\d+) (\\d+)\\/(\\d+)\\/(\\d+)");

bool ObjLoader::loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle)
{
	std::istringstream iss(rawBuffer);
	std::string line;

	// These match one-to-one the data in the .obj file
	std::vector<glm::vec3> rawVertices;
	std::vector<glm::vec2> rawUvs;
	std::vector<glm::vec3> rawNormals;

	// These are ordered based on the face elements of the .obj file
	std::vector<glm::vec3> orderedVertices;
	std::vector<glm::vec2> orderedUvs;
	std::vector<glm::vec3> orderedNormals;
	std::vector<glm::vec3> orderedTangents;
	std::vector<glm::vec3> orderedBitangents;

	// Final, indexed vectors
	std::vector<glm::vec3> outVertices;
	std::vector<glm::vec2> outUvs;
	std::vector<glm::vec3> outNormals;
	std::vector<glm::vec3> outTangents;
	std::vector<glm::vec3> outBitangents;
	std::vector<unsigned short> outIndices;

	while (std::getline(iss, line)) {
		std::smatch m;

		if (std::regex_match(line, m, vRgx)) {

			float x, y, z;
			try {
				x = std::stof(m[1]);
				y = std::stof(m[2]);
				z = std::stof(m[3]);
			}
			catch (const std::invalid_argument&) {
				LOG_DEBUG("ObjLoader::loadResource: Could not convert string to float on line: " + line);
				return false;
			}
			rawVertices.push_back(glm::vec3(x, y, z));

		} else if (std::regex_match(line, m, vtRgx)) {

			float x, y;
			try {
				x = std::stof(m[1]);
				y = std::stof(m[2]);
			}
			catch (const std::invalid_argument&) {
				LOG_DEBUG("ObjLoader::loadResource: Could not convert string to float on line: " + line);
				return false;
			}
			rawUvs.push_back(glm::vec2(x, y));

		} else if (std::regex_match(line, m, vnRgx)) {

			float x, y, z;
			try {
				x = std::stof(m[1]);
				y = std::stof(m[2]);
				z = std::stof(m[3]);
			}
			catch (const std::invalid_argument&) {
				LOG_DEBUG("ObjLoader::loadResource: Could not convert string to float on line: " + line);
				return false;
			}
			rawNormals.push_back(glm::vec3(x, y, z));

		} else if (std::regex_match(line, m, fRgx)) {

			unsigned short va, vb, vc, vta, vtb, vtc, vna, vnb, vnc;
			try {
				va = std::stoi(m[1]);
				vb = std::stoi(m[4]);
				vc = std::stoi(m[7]);
				vta = std::stoi(m[2]);
				vtb = std::stoi(m[5]);
				vtc = std::stoi(m[8]);
				vna = std::stoi(m[3]);
				vnb = std::stoi(m[6]);
				vnc = std::stoi(m[9]);
			}
			catch (const std::invalid_argument&) {
				LOG_DEBUG("ObjLoader::loadResource: Could not convert string to float on line: " + line);
				return false;
			}

			// Fill ordered vectors based on face element indices
			try {
				orderedVertices.push_back(rawVertices.at(va - 1));
				orderedVertices.push_back(rawVertices.at(vb - 1));
				orderedVertices.push_back(rawVertices.at(vc - 1));
				orderedUvs.push_back(rawUvs.at(vta - 1));
				orderedUvs.push_back(rawUvs.at(vtb - 1));
				orderedUvs.push_back(rawUvs.at(vtc - 1));
				orderedNormals.push_back(rawNormals.at(vna - 1));
				orderedNormals.push_back(rawNormals.at(vnb - 1));
				orderedNormals.push_back(rawNormals.at(vnc - 1));
			}
			catch (const std::out_of_range&) {
				LOG_DEBUG("ObjLoader::loadResource: Face element indexes out of range: " + line);
				return false;
			}
		}
	}

	for (int i = 0; i < orderedVertices.size(); i += 3) {
		glm::vec3 tangent, bitangent;
		glm::vec3 edge1 = orderedVertices[i + 1] - orderedVertices[i];
		glm::vec3 edge2 = orderedVertices[i + 2] - orderedVertices[i];
		glm::vec2 deltaUV1 = orderedUvs[i + 1] - orderedUvs[i];
		glm::vec2 deltaUV2 = orderedUvs[i + 2] - orderedUvs[i];
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		orderedTangents.push_back(tangent);
		orderedTangents.push_back(tangent);
		orderedTangents.push_back(tangent);

		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		orderedBitangents.push_back(bitangent);
		orderedBitangents.push_back(bitangent);
		orderedBitangents.push_back(bitangent);
	}

	// Map for storing already indexed vertices
	auto comp_lt = [](const PackedVertex& v1, const PackedVertex& v2) {
		auto tuple1 = std::tie(v1.vertex.x, v1.vertex.y, v1.vertex.z, v1.uv.x, v1.uv.y, v1.normal.x, v1.normal.y, v1.normal.z, 
								v1.tangent.x, v1.tangent.y, v1.tangent.z, v1.bitangent.x, v1.bitangent.y, v1.bitangent.z);
		auto tuple2 = std::tie(v2.vertex.x, v2.vertex.y, v2.vertex.z, v2.uv.x, v2.uv.y, v2.normal.x, v2.normal.y, v2.normal.z,
								v2.tangent.x, v2.tangent.y, v2.tangent.z, v2.bitangent.x, v2.bitangent.y, v2.bitangent.z);
		return tuple1 < tuple2;
	};
	std::map<PackedVertex, unsigned short, decltype(comp_lt)> foundVertices(comp_lt);

	// Index vertices, go through all loaded vertices and save only unique combinations of v, vt and vn
	unsigned short nextOutIdx = 0, idx = 0;
	for (auto it = orderedVertices.begin(); it != orderedVertices.end(); ++it, ++idx) {
		PackedVertex packed = { orderedVertices[idx], orderedUvs[idx], orderedNormals[idx], orderedTangents[idx], orderedBitangents[idx] };

		auto it_found = foundVertices.find(packed);
		if (it_found == foundVertices.end()) {
			outVertices.push_back(packed.vertex);
			outUvs.push_back(packed.uv);
			outNormals.push_back(packed.normal);
			outTangents.push_back(packed.tangent);
			outBitangents.push_back(packed.bitangent);
			outIndices.push_back(nextOutIdx);
			foundVertices.insert(std::make_pair(packed, nextOutIdx));
			nextOutIdx++;
		} else {
			outIndices.push_back(it_found->second);
		}
	}

	std::shared_ptr<ModelResProcessedData> data(new ModelResProcessedData(outVertices, outUvs, outNormals, outTangents, outBitangents, outIndices));

	handle->size = rawSize;
	handle->buffer = rawBuffer;
	handle->processedData = data;

	return true;
}
