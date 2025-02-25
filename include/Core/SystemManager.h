#pragma once
#include <SDL_gesture.h>
#include <SDL.h>
#include <iostream>
#include <glad/glad.h>
#include "ApplicationConfig.h"

#include "asio.hpp" 
#include <thread>

void GetOpenGLVersionInfo();

// SystemManager.h - New class to handle SDL/OpenGL initialization
class SystemManager {
private:
    SDL_Window* window;
    SDL_GLContext glContext; 



public:
    bool InitializeSDLAndOpenGL();

    SDL_Window* GetWindow();


    void Cleanup();
};