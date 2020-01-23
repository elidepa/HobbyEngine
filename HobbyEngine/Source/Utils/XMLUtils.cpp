#include "XMLUtils.h"

#include <string>

#include "DebugLogger.h"
#include "../Engine/GLApplication.h"

// TODO: For some reason VS throws an identifier undefined error for LOG_DEBUG even thoufh DebugLogger.h is clearly
// included above.
#ifdef LOG_LEVEL_DEBUG
#define LOG_DEBUG(str) DebugLogger::log(str);
#else
#define LOG_DEBUG(str)
#endif // LOG_LEVEL_DEBUG

bool XMLUtils::loadXMLFile(std::string filename, tinyxml2::XMLDocument& doc)
{
	auto resourceCache = Game::instance().resourceCache();

	Resource resource(filename);
	auto xmlhandle = resourceCache.getHandle(resource);

	tinyxml2::XMLError err;
	if ((err = doc.Parse(xmlhandle->buffer)) != tinyxml2::XML_SUCCESS) {
		LOG_DEBUG("XMLUtils::loadXMLFile: Could not parse XML, error: " + err + std::string("\n") + xmlhandle->buffer);
		return false;
	}

	return true;
}

bool XMLUtils::xmlXYZAttribsToFloat(tinyxml2::XMLElement* elem, float& x, float& y, float& z)
{
	auto s_x = elem->Attribute("x");
	auto s_y = elem->Attribute("y");
	auto s_z = elem->Attribute("z");

	if (!s_x | !s_y | !s_z) {
		LOG_DEBUG("XMLUtils::xmlXYZAttribsToFloat: could not find x, y, or z attributes");
		return false;
	}

	try
	{
		x = std::stof(s_x);
		y = std::stof(s_y);
		z = std::stof(s_z);
	}
	catch (const std::exception&)
	{
		LOG_DEBUG("XMLUtils::xmlXYZAttribsToFloat: could not convert element values to float");
		return false;
	}
	return true;
}

bool XMLUtils::xmlRGBAAttribsToFloat(tinyxml2::XMLElement* elem, float& r, float& g, float& b, float& a)
{
	auto s_r = elem->Attribute("r");
	auto s_g = elem->Attribute("g");
	auto s_b = elem->Attribute("b");
	auto s_a = elem->Attribute("a");

	if (!s_r | !s_g | !s_b) {
		LOG_DEBUG("XMLUtils::xmlXYZAttribsToFloat: could not find x, y, or z attributes");
		return false;
	}

	try
	{
		r = std::stof(s_r);
		g = std::stof(s_g);
		b = std::stof(s_b);
		a = std::stof(s_a);
	}
	catch (const std::exception&)
	{
		LOG_DEBUG("XMLUtils::xmlXYZAttribsToFloat: could not convert element values to float");
		return false;
	}
	return true;
}

bool XMLUtils::xmlAttribToFloat(tinyxml2::XMLElement* elem, std::string attrib, float& res)
{
	auto s_attrib = elem->Attribute(attrib.c_str());

	if (!s_attrib) {
		LOG_DEBUG("XMLUtils::xmlAttribToFloat: could not find attribute " + attrib);
		return false;
	}

	try
	{
		res = std::stof(s_attrib);
	}
	catch (const std::exception&)
	{
		LOG_DEBUG("XMLUtils::xmlAttribToFloat: could not find convert value of attribute \"" + attrib + "\" to float");
		return false;
	}

	return true;
}

bool XMLUtils::xmlAttribToInt(tinyxml2::XMLElement* elem, std::string attrib, int& res)
{
	auto s_attrib = elem->Attribute(attrib.c_str());

	if (!s_attrib) {
		LOG_DEBUG("XMLUtils::xmlAttribToFloat: could not find attribute " + attrib);
		return false;
	}

	try
	{
		res = std::stoi(s_attrib);
	}
	catch (const std::exception&)
	{
		LOG_DEBUG("XMLUtils::xmlAttribToFloat: could not find convert value of attribute \"" + attrib + "\" to int");
		return false;
	}

	return true;
}
