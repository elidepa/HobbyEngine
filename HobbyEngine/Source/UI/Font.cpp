#include "Font.h"

#include <GLFW/glfw3.h>

#include "../Utils/DebugLogger.h"

bool Font::init(FT_Face& face, uint16_t nglyphs)
{
	for (uint32_t c = 0; c < nglyphs; ++c) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			LOG_DEBUG("Font::init: could not load character " + c);
			return false;
		}

		Character* ch = new Character;
		std::shared_ptr<Character> pCh = std::shared_ptr<Character>(ch);

		glGenTextures(1, &ch->textureID);
		glBindTexture(GL_TEXTURE_2D, ch->textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		ch->c = c;
		ch->size = glm::vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
		ch->bearing = glm::vec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
		ch->advance = face->glyph->advance.x;

		m_glyphs.push_back(pCh);
	}

	return true;
}

Character::~Character()
{
	glDeleteTextures(1, &textureID);
}
