#include "Utils/LOG.h"

// Definition of the global log level
LogLevel CURRENT_LOG_LEVEL = LOG_DEBUG;

// Function definition to convert log level to string
std::string GetLogLevelString(LogLevel level) {
    switch (level) {
    case LOG_ERROR:   return "ERROR";
    case LOG_WARNING: return "WARNING";
    case LOG_INFO:    return "INFO";
    case LOG_DEBUG:   return "DEBUG";
    default:          return "NONE";
    }
}
