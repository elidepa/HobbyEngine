#ifndef RESOURCE_CACHE_H
#define RESOURCE_CACHE_H

#include <list>
#include <map>
#include <memory>
#include <string>

class IResLoader;

class Resource
{
public:
	Resource(const std::string& name) : m_name(name) {}

	std::string name() { return m_name; }

private:
	std::string m_name;
};

class IResProcessedData
{
public:
	virtual std::string toString() = 0;
};

class ResHandle
{
	friend class IResLoader;
public:
	ResHandle(Resource& resource, char* buffer, size_t size) :
		m_resource(resource), buffer(buffer), size(size) {};
	~ResHandle();

	const std::string name() { return m_resource.name(); }

	size_t size;
	char* buffer;
	std::shared_ptr<IResProcessedData> processedData;
private:
	Resource m_resource;
};

class IResLoader
{
public:
	virtual std::string getWildcard() = 0;
	virtual bool useRawFile() = 0;
	virtual size_t getLoadedResourceSize(char* rawBuffer, size_t rawSize) = 0;
	virtual bool isText() = 0;
	virtual bool loadResource(char* rawBuffer, size_t rawSize, std::shared_ptr<ResHandle> handle) = 0;
};

class ResCache
{
public:
	bool init();
	void flush();

	void registerLoader(std::shared_ptr<IResLoader> loader);

	std::shared_ptr<ResHandle> getHandle(Resource& resource);

private:
	std::shared_ptr<ResHandle> load(Resource& resource);
	std::shared_ptr<ResHandle> find(Resource& resource);

	std::map<std::string, std::shared_ptr<ResHandle>> m_handles;
	std::list<std::shared_ptr<IResLoader>> m_loaders;

};

#endif // !RESOURCE_CACHE_H
