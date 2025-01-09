#pragma once
#include <unordered_map>
#include <memory>
#include "GameMode.h"

class GameEngine;


class GameModeController {
private:
    GameEngine* gameEngine;
    std::unordered_map<GameModeType, std::unique_ptr<GameMode>> modes;
    GameMode* currentMode = nullptr;

public:
    GameModeController(GameEngine* engine);

    void Initialize();
    void SwitchMode(GameModeType newMode);
    void Update();
    void Draw();
};

