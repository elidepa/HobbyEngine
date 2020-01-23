#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

ImageLoader::ImageLoader()
{
	m_preloadCache = nullptr;
	m_preloadData = std::shared_ptr<ImageResProcessedData>(new ImageResProcessedData());
}

size_t ImageLoader::getLoadedResourceSize(char* rawBuffer, size_t rawSize)
{
	m_preloadCache = stbi_load_from_memory((const unsigned char*)rawBuffer, rawSize, &m_preloadData->m_width, &m_preloadData->m_height, &m_preloadData->m_nChannels, 0);
	return m_preloadData->width() * m_preloadData->height() * m_preloadData->nChannels();
}

bool ImageLoader::loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle)
{
	ImageResProcessedData processedData;

	if (!m_preloadCache) {
		auto loadRes = stbi_load_from_memory((const unsigned char*)rawBuffer, rawSize, &processedData.m_width, &processedData.m_height, &processedData.m_nChannels, 0);
		std::copy(loadRes, loadRes + handle->size, handle->buffer);
		handle->processedData = std::make_shared<ImageResProcessedData>(processedData);

		delete[] loadRes;
	} else {
		std::copy(m_preloadCache, m_preloadCache + handle->size, handle->buffer);
		handle->processedData = m_preloadData;

		delete[] m_preloadCache;
	}

	delete[] rawBuffer;

	return true;
}

ImageResProcessedData::ImageResProcessedData()
{
	m_width = 0;
	m_height = 0;
	m_nChannels = 0;
}
