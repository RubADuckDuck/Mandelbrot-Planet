#pragma once
#include <SDL_gesture.h>
#include <SDL.h>
#include <iostream>
#include <glad/glad.h>
#include "ApplicationConfig.h"

#include "asio.hpp" 
#include <thread>

// SystemManager.h - New class to handle SDL/OpenGL initialization
class SystemManager {
private:
    SDL_Window* window;
    SDL_GLContext glContext; 



public:
    bool InitializeSDLAndOpenGL() {
        // Move SDL and OpenGL initialization here
        LOG(LOG_INFO, "  Current system: SDL + OPENGL");  
         
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cout << "SDL could not init" << std::endl;
            return false;
        }

        // Your existing SDL/OpenGL setup code...
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        // ... rest of the attributes

        window = SDL_CreateWindow("OpenGL", 
            100, 100,
            ApplicationConfig::GetScreenWidth(), 
            ApplicationConfig::GetScreenHeight(), 
            SDL_WINDOW_OPENGL
        ); 


        if (!window) return false;

        glContext = SDL_GL_CreateContext(window);
        if (!glContext) return false;

        if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) return false;

        // OpenGL settings
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_DEBUG_OUTPUT);
        // ... rest of OpenGL setup

        return true;
    }

    SDL_Window* GetWindow() { return window; } 


    void Cleanup() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};