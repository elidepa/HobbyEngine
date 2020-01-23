#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#include <string>

#ifdef LOG_LEVEL_DEBUG
#define LOG_DEBUG(str) DebugLogger::log(str);
#else
#define LOG_DEBUG(str)
#endif // LOG_LEVEL_DEBUG

class DebugLogger
{
public:
	DebugLogger();

	static void log(std::string msg);

private:

};

#endif // !DEBUG_LOGGER_H
