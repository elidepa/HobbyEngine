#include "FontLoader.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../Utils/DebugLogger.h"

#ifdef LOG_LEVEL_DEBUG
#define FT_DEBUG_LEVEL_TRACE
#endif // LOG_LEVEL_DEBUG

size_t FontLoader::getLoadedResourceSize(char* rawBuffer, size_t rawSize)
{
	return rawSize;
}

bool FontLoader::loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle)
{
	FT_Library ft;

	if (FT_Init_FreeType(&ft)) {
		LOG_DEBUG("FontLoader::loadResource: could not init Freetype.");
		return false;
	}

	FT_Face face;
	// TODO: Fix this OR add an option for the loader to excplicitly read the resource from disk by themselves
	// so that the ResourceCache doesn't load it too. Currently FT_New_Memory_Face returns error 6, meaning
	// invalid argument. ResHandle::getName should be replaced with ResHandle::getPath that return the full
	// path including the resource directory if the loader loads the resource
	//if (FT_Error err = FT_New_Memory_Face(ft, (const FT_Byte*)handle->getBuffer(), rawSize, 0, &face)) {
	if (FT_Error err = FT_New_Face(ft, (std::string("Resources/") + handle->name()).c_str(), 0, &face)) {
		LOG_DEBUG("FontLoader::loadResource: could not load font face, error: " + std::to_string(err));
		return false;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);

	std::shared_ptr<Font> font = std::shared_ptr<Font>(new Font);
	if (!font->init(face, 256)) {
		LOG_DEBUG("FontLoader::loadResource: could not init font.");
		return false;
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	FontResProcessedData* data = new FontResProcessedData;
	data->m_font = font;

	handle->processedData = std::shared_ptr<IResProcessedData>(data);

	return true;
}
