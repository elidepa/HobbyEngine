#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <memory>
#include <string>

#include "ResourceCache.h"

class ImageResProcessedData : public IResProcessedData
{
	friend class ImageLoader;
public:
	ImageResProcessedData();

	virtual std::string toString() { return "ImageResProcessedData"; }

	int width() { return m_width; }
	int height() { return m_height; }
	int nChannels() { return m_nChannels; }

private:
	int m_width;
	int m_height;
	int m_nChannels;
};

class ImageLoader : public IResLoader
{
public:
	ImageLoader();

	virtual std::string getWildcard() { return ".*\\.(jpeg|jpg|png|bmp)"; }
	virtual bool useRawFile() { return false; }
	virtual size_t getLoadedResourceSize(char* rawBuffer, size_t rawSize);
	virtual bool isText() { return false; }
	virtual bool loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle);

private:
	unsigned char* m_preloadCache;
	std::shared_ptr<ImageResProcessedData> m_preloadData;
};

#endif // !IMAGE_LOADER_H
