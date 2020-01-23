#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

class FileUtils
{
public:
	static int readFileStr(const char* filename, std::string& dest);
	static int fileSize(const char* filename);
	static bool readRawFile(const char* filename, char* buffer, size_t length);
};

#endif // !FILE_UTILS_H
