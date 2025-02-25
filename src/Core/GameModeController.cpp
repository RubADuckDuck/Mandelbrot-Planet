// GameModeController.cpp
#include "Core/GameModeController.h"
#include "GameModes/MainMenuMode.h"
#include "GameModes/PlayingMode.h"
#include "GameModes/HostLobbyMode.h"
#include "GameModes/JoinLobbyMode.h"


GameModeController::GameModeController(GameEngine* engine) : gameEngine(engine) {}

void GameModeController::Initialize() {
    // Create all possible game modes
    modes[GameModeType::MAIN_MENU] = std::make_unique<MainMenuMode>(gameEngine);
    modes[GameModeType::PLAYING] = std::make_unique<PlayingMode>(gameEngine);
    modes[GameModeType::HOST_LOBBY] = std::make_unique<HostLobbyMode>(gameEngine);
    modes[GameModeType::JOIN_LOBBY] = std::make_unique<JoinLobbyMode>(gameEngine); 

    // Start with main menu
    SwitchMode(GameModeType::MAIN_MENU);
}

void GameModeController::SwitchMode(GameModeType newMode) {
    LOG(LOG_INFO, "Attempting Switching GameMode to: " + mode2string[newMode]);
    if (currentMode) {
        
        currentMode->Exit();
    }

    auto it = modes.find(newMode);


    if (it != modes.end()) {
        LOG(LOG_INFO, "Switching GameMode to: " + mode2string[newMode]);

        currentMode = it->second.get();
        currentMode->Enter();
    }
    else {
        LOG(LOG_INFO, "We can't find that Mode");
    }
}

void GameModeController::Update() {
    if (currentMode) {
        currentMode->Update();
    }
}

void GameModeController::Draw() {
    if (currentMode) {
        currentMode->Draw();
    }
}