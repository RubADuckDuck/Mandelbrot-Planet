#pragma once
#include <iostream>
#include <string>

// Logging Levels
enum LogLevel { LOG_NONE, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };

// Global log level to control verbosity
LogLevel CURRENT_LOG_LEVEL = LOG_DEBUG;  // Set to desired level

// Macro for logging with levels
#define LOG(level, message) \
    if (level <= CURRENT_LOG_LEVEL) { \
        std::cout << "[" << GetLogLevelString(level) << "] " << message << std::endl; \
    }

// Function to convert log level to string
inline std::string GetLogLevelString(LogLevel level) {
    switch (level) {
    case LOG_ERROR:   return "ERROR";
    case LOG_WARNING: return "WARNING";
    case LOG_INFO:    return "INFO";
    case LOG_DEBUG:   return "DEBUG";
    default:          return "NONE";
    }
}