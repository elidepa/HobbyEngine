#ifndef FONT_H
#define FONT_H

#include <memory>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>

struct Character
{
	Character() = default;
	~Character();

	uint32_t textureID;
	glm::vec2 size;
	glm::vec2 bearing;
	uint32_t advance;
	char c;
};

class Font
{
public:
	bool init(FT_Face& face, uint16_t nglyphs);

	uint16_t nGlyphs() { return m_nglyphs; }
	std::vector<std::shared_ptr<Character>>& glyphs() { return m_glyphs; }

private:
	uint16_t m_nglyphs;
	std::vector<std::shared_ptr<Character>> m_glyphs;
};

#endif // !FONT_H
