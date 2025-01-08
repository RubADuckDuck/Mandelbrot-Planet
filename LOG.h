#pragma once

#include <iostream>
#include <string>

// Logging Levels
enum LogLevel { LOG_NONE, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG };

// Global log level to control verbosity
extern LogLevel CURRENT_LOG_LEVEL;  // Declaration (not definition)

// Macro for logging with levels
#define LOG(level, message) \
    if (level <= CURRENT_LOG_LEVEL) { \
        std::cout << "[" << GetLogLevelString(level) << "] " << message << std::endl; \
    }

// Function declaration to convert log level to string
std::string GetLogLevelString(LogLevel level);