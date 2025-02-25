#pragma once
#include <string> 
#include "../Utils/LOG.h"

class ApplicationConfig {
private:
    // Make constructor private to prevent direct instantiation
    ApplicationConfig() = default;

    // Application state
    static bool quit;

    // Window configuration
    static int screenWidth;
    static int screenHeight;

public:
    // Delete copy constructor and assignment to ensure singleton
    ApplicationConfig(const ApplicationConfig&) = delete;
    ApplicationConfig& operator=(const ApplicationConfig&) = delete;

    // Accessor methods
    static bool ShouldQuit() { return quit; }
    static void SetQuit(bool shouldQuit) { quit = shouldQuit; }

    static int GetScreenWidth() { return screenWidth; }
    static int GetScreenHeight() { return screenHeight; }

    // Initialize with default values
    static void Initialize(int width = 1280, int height = 720) {
        screenWidth = width;
        screenHeight = height;

        std::string swsh = "  ScreenWidth: " + std::to_string(width) + "  ScreenHeight: " + std::to_string(height);

        LOG(LOG_INFO, swsh); 
        quit = false;
    }
};


