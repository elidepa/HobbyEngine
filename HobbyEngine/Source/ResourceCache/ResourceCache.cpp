#include "ResourceCache.h"

#include <regex>

#include "../Utils/DebugLogger.h"
#include "../Utils/FileUtils.h"

bool ResCache::init()
{
	return true;
}

void ResCache::flush()
{
	m_handles.clear();
}

void ResCache::registerLoader(std::shared_ptr<IResLoader> loader)
{
	m_loaders.push_front(loader);
}

std::shared_ptr<ResHandle> ResCache::getHandle(Resource& resource)
{
	auto handle = find(resource);

	if (!handle) {
		handle = load(resource);
	}

	return handle;
}

std::shared_ptr<ResHandle> ResCache::load(Resource& resource)
{
	std::shared_ptr<IResLoader> loader;
	std::shared_ptr<ResHandle> handle;

	for (auto it = m_loaders.begin(); it != m_loaders.end(); ++it) {
		auto it_loader = *it;
		std::regex rgx(it_loader->getWildcard());

		if (std::regex_match(resource.name(), rgx)) {
			loader = it_loader;
			break;
		}
	}

	if (!loader) {
		LOG_DEBUG("Could not find loader for resource: " + resource.name());
		return std::shared_ptr<ResHandle>();
	}

	std::string resPath = "Resources/" + resource.name();

	int rawSize = FileUtils::fileSize(resPath.c_str());

	if (rawSize < 0) {
		LOG_DEBUG("Could not find resource: " + resource.name());
		return std::shared_ptr<ResHandle>();
	}

	rawSize += ((loader->isText()) ? 1 : 0);
	char* rawBuffer = new char[rawSize];

	if (!rawBuffer) {
		LOG_DEBUG("Could not allocate " + std::to_string(rawSize) + " bytes for raw resource: " + resource.name());
		return std::shared_ptr<ResHandle>();
	}

	memset(rawBuffer, 0, rawSize);

	if (!FileUtils::readRawFile(resPath.c_str(), rawBuffer, rawSize)) {
		LOG_DEBUG("Could not read resource: " + resource.name());
		return std::shared_ptr<ResHandle>();
	}

	if (loader->isText()) {
		rawBuffer[rawSize - 1] = '\0';
	}

	if (loader->useRawFile()) {
		handle = std::shared_ptr<ResHandle>(new ResHandle(resource, rawBuffer, rawSize));
	} else {
		size_t loadedSize = loader->getLoadedResourceSize(rawBuffer, rawSize);
		char* buffer = new char[loadedSize];

		if (!buffer) {
			LOG_DEBUG("Could not allocate " + std::to_string(rawSize) + " bytes for loading resource: " + resource.name());
			return std::shared_ptr<ResHandle>();
		}

		handle = std::shared_ptr<ResHandle>(new ResHandle(resource, buffer, loadedSize));
		bool success = loader->loadResource(rawBuffer, rawSize, handle);

		if (!success) {
			LOG_DEBUG("Could not allocate " + std::to_string(rawSize) + " bytes for loading resource: " + resource.name());
			return std::shared_ptr<ResHandle>();
			}
		}

	if (handle) {
		m_handles[resource.name()] = handle;
	}

	return handle;

	return std::shared_ptr<ResHandle>();
}

std::shared_ptr<ResHandle> ResCache::find(Resource& resource)
{
	auto it_handle = m_handles.find(resource.name());

	if (it_handle == m_handles.end()) {
		return std::shared_ptr<ResHandle>();
	}

	return it_handle->second;
}

ResHandle::~ResHandle()
{
	if (buffer != nullptr) {
		delete[] buffer;
		buffer = nullptr;
	}
}
