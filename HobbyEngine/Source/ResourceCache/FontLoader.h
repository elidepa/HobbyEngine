#ifndef FONT_LOADER_H
#define FONT_LOADER_H

#include <memory>

#include "ResourceCache.h"
#include "../UI/Font.h"

class FontResProcessedData : public IResProcessedData
{
	friend class FontLoader;
public:
	virtual std::string toString() { return "FontResProcessedData"; }

	std::shared_ptr<Font> font() { return m_font; }

private:
	std::shared_ptr<Font> m_font;
};

class FontLoader : public IResLoader
{
public:
	virtual std::string getWildcard() { return ".*\\.(ttf)"; }
	virtual bool useRawFile() { return false; }
	virtual size_t getLoadedResourceSize(char* rawBuffer, size_t rawSize);
	virtual bool isText() { return false; }
	virtual bool loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle);
private:

};

#endif // !FONT_LOADER_H
