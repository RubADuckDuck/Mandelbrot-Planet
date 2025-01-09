// GameModeController.cpp
#include "GameModeController.h"
#include "MainMenuMode.h"
#include "PlayingMode.h"
#include "HostLobbyMode.h"
#include "JoinLobbyMode.h"

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
    if (currentMode) {
        currentMode->Exit();
    }

    auto it = modes.find(newMode);
    if (it != modes.end()) {
        currentMode = it->second.get();
        currentMode->Enter();
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