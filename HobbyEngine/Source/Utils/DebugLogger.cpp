#include "DebugLogger.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

#include <iostream>

DebugLogger::DebugLogger()
{
}

void DebugLogger::log(std::string msg)
{
#ifdef _WIN32
	std::string out(msg + "\n");
	OutputDebugString(out.c_str());
#endif // _WIN32
	std::cout << msg << std::endl;
}
