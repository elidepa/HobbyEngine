#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <map>
#include <memory>
#include <string>

#include <tinyxml2/tinyxml2.h>

class GameObject;
class GameObjectFactory;
class IGOComponent;
class Scene;

typedef IGOComponent* (*GOComponentCreator)(void);
typedef std::map<std::string, GOComponentCreator> GOComponentCreatorMap;

typedef const char* ComponentId;

class GameObject
{
	friend class GOFactory;
	typedef std::map<const ComponentId, std::shared_ptr<IGOComponent>> GOComponentsMap;

public:
	explicit GameObject(uint64_t id) : m_id(id) {};
	~GameObject();
	
	int init(tinyxml2::XMLElement* data);
	void postInit();
	void destroy();

	void update(int deltaTime);

	uint64_t getId() const { return m_id; }

	template <class T>
	std::weak_ptr<T> findComponent(ComponentId id)
	{
		auto componentIt = m_components.find(id);
		if (componentIt != m_components.end()) {
			std::shared_ptr<IGOComponent> found = componentIt->second;
			std::weak_ptr<T> component(std::static_pointer_cast<T>(found));
			return component;
		} else {
			return std::weak_ptr<T>();
		}
	}

	std::shared_ptr<Scene> scene;

private:	
	void addComponent(std::shared_ptr<IGOComponent> component);

	uint64_t m_id;
	GOComponentsMap m_components;
};

class GOFactory
{
public:
	GOFactory();
	~GOFactory();

	std::shared_ptr<GameObject> createGameObject(const char* xml);

protected:
	virtual std::shared_ptr<IGOComponent> createComponent(tinyxml2::XMLElement* data);

	GOComponentCreatorMap m_GOComponentCreators;

private:
	uint64_t getNextGOId() { ++m_lastId; return m_lastId; }

	uint64_t m_lastId = 0;
};

class IGOComponent
{
	friend class GOFactory;

public:
	virtual ~IGOComponent() { m_owner.reset(); }

	virtual bool init(tinyxml2::XMLElement* data) = 0;
	virtual void postInit() {}
	virtual void update(int deltaTime) {}

	virtual ComponentId componentId() const = 0;

protected:
	std::shared_ptr<GameObject> m_owner;

private:
	void setOwner(std::shared_ptr<GameObject> owner) { m_owner = owner; }
};

#endif // !GAME_OBJECT_H
