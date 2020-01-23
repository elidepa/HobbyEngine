#ifndef TEXT_LOADER_H
#define TEXT_LOADER_H

#include "ResourceCache.h"

class TextLoader : public IResLoader
{
public:
	virtual std::string getWildcard() { return std::string(".*(\\.txt|\\.xml|\\.glsl)"); }
	virtual bool useRawFile() { return true; }
	virtual size_t getLoadedResourceSize(char* rawBuffer, size_t rawSize) { return rawSize; }
	virtual bool isText() { return true; }
	virtual bool loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle) { return true; }
};

#endif // !TEXT_LOADER_H
