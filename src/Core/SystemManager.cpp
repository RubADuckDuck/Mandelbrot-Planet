#include "Core/SystemManager.h"

void GetOpenGLVersionInfo() {
    glGetString(GL_RENDERER);
    std::cout << "Vendor" << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer" << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version" << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

bool SystemManager::InitializeSDLAndOpenGL() {
    // Move SDL and OpenGL initialization here
    LOG(LOG_INFO, "  Current system: SDL + OPENGL");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not init" << std::endl;
        return false;
    }

    // Your existing SDL/OpenGL setup code...
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
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

    glViewport(0, 0, ApplicationConfig::GetScreenWidth(), ApplicationConfig::GetScreenHeight());

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    
    glDisable(GL_CULL_FACE);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // debugging 
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam) {
            std::cerr << "GL DEBUG: " << message << std::endl;
        }, nullptr);

    GetOpenGLVersionInfo();

    return true;
}

SDL_Window* SystemManager::GetWindow() { return window; }

inline void SystemManager::Cleanup() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}
