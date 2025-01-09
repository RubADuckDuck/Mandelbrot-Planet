#pragma once
#include "GameMode.h"
#include <vector>
#include <functional>
#include <SDL_video.h>
#include <string>

class MainMenuMode : public GameMode {
private:
    struct MenuButton {
        SDL_Rect rect;
        std::string text;
        std::function<void()> onClick;
    };

    std::vector<MenuButton> buttons;
    SDL_Color buttonColor = { 100, 100, 100, 255 };
    SDL_Color hoverColor = { 150, 150, 150, 255 };

public:
    MainMenuMode(GameEngine* engine);
    void Enter() override;
    void Update() override;
    void Draw() override;
    void Exit() override;
};

