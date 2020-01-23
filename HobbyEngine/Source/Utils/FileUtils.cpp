#include "FileUtils.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "DebugLogger.h"

int FileUtils::readFileStr(const char* filename, std::string& dest)
{
	std::stringstream ss;
	std::ifstream ifs(filename);

	if (!ifs.is_open()) {
		LOG_DEBUG(std::string("FileUtils::readFileStr: Could not open file: ") + filename);
		return -1;
	}

	ss << ifs.rdbuf();
	dest = ss.str();

	ifs.close();

	return 0;
}

int FileUtils::fileSize(const char* filename)
{
	std::ifstream ifs(filename, std::ios::binary);

	if (!ifs.is_open()) {
		LOG_DEBUG(std::string("FileUtils::fileSize: Could not open file: ") + filename);
		return -1;
	}

	const auto begin = ifs.tellg();
	ifs.seekg(0, std::ios::end);
	const auto end = ifs.tellg();
	ifs.close();
	return end - begin;
}

// TODO: Return read bytes, if(!ifs) seems odd, make sure it's correct
bool FileUtils::readRawFile(const char* filename, char* buffer, size_t length)
{
	std::ifstream ifs(filename, std::ios::binary);

	if (!ifs.is_open()) {
		LOG_DEBUG(std::string("FileUtils::readRawFile: Could not open file:") + filename);
		return false;
	}

	ifs.read(buffer, length);
	ifs.close();
	return true;
}
