#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <string>

#include <tinyxml2/tinyxml2.h>

class XMLUtils
{
public:
	static bool loadXMLFile(std::string filename, tinyxml2::XMLDocument& doc);
	static bool xmlXYZAttribsToFloat(tinyxml2::XMLElement* elem, float& x, float& y, float& z);
	static bool xmlRGBAAttribsToFloat(tinyxml2::XMLElement* elem, float& r, float& g, float& b, float& a);
	static bool xmlAttribToFloat(tinyxml2::XMLElement* elem, std::string attrib, float& res);
	static bool xmlAttribToInt(tinyxml2::XMLElement* elem, std::string attrib, int& res);
};

#endif // !XML_UTILS_H
