#ifndef SKYBOX_H
#define SKYBOX_H

#include <tinyxml2/tinyxml2.h>

class Skybox
{
public:
	Skybox() = default;
	~Skybox();

	bool init(tinyxml2::XMLElement* root);

	uint32_t texture() { return m_texture; }
	uint32_t vao() { return m_VAO; }
	uint32_t vbo() { return m_VBO; }

private:
	uint32_t m_texture;
	uint32_t m_VAO;
	uint32_t m_VBO;
};

#endif // !SKYBOX_H
