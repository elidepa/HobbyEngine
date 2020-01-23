#include "UIElement.h"

#include "../Engine/GLApplication.h"
#include "../ResourceCache/FontLoader.h"
#include "../ResourceCache/ResourceCache.h"
#include "TextElement.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/XMLUtils.h"

std::shared_ptr<UIElement> UIElementFactory::createUIElement(tinyxml2::XMLElement* data)
{
	auto elementType = data->Value();

	std::shared_ptr<UIElement> pElement;
	if (std::string("TextElement") == std::string(elementType)) {
		TextElement* element = new TextElement;
		pElement = std::shared_ptr<TextElement>(element);
		pElement->init(data);
	}
	else {
		LOG_DEBUG("UIElementFactory::createUIElement: couldn't recognize element type " + std::string(elementType));
		return std::shared_ptr<UIElement>();
	}

	return pElement;
}
