#include "Core/GameEngine.h"


int main(int argc, char* argv[]) {
    GameEngine engine = GameEngine();
    if (!engine.Initialize()) {
        return 1;
    }

    const int targetFPS = 60;
    const float targetFrameTime = 1.0f / targetFPS;

    while (!ApplicationConfig::ShouldQuit()) {  // You might want to change this to check engine state
        Uint64 startTicks = SDL_GetPerformanceCounter();

        engine.Update();
        engine.Draw();

        // Frame timing logic
        Uint64 endTicks = SDL_GetPerformanceCounter();
        float frameTime = (float)(endTicks - startTicks) / SDL_GetPerformanceFrequency();
        if (frameTime < targetFrameTime) {
            SDL_Delay((Uint32)((targetFrameTime - frameTime) * 1000.0f));
        }
    }

    return 0;
}