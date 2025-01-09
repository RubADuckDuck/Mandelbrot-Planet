// MainMenuMode.cpp
#include "MainMenuMode.h"
#include "ApplicationConfig.h"
#include "GameEngine.h"

MainMenuMode::MainMenuMode(GameEngine* engine) : GameMode(engine) {
    // Create menu buttons
    int buttonWidth = 200;
    int buttonHeight = 50;
    int startY = 300;
    int spacing = 60;

    buttons = {
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY, buttonWidth, buttonHeight},
            "Host Game",
            [this]() { gameEngine->GetModeController()->SwitchMode(GameModeType::HOST_LOBBY); }
        },
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY + spacing, buttonWidth, buttonHeight},
            "Join Game",
            [this]() { gameEngine->GetModeController()->SwitchMode(GameModeType::JOIN_LOBBY); }
        },
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY + spacing * 2, buttonWidth, buttonHeight},
            "Settings",
            [this]() { gameEngine->GetModeController()->SwitchMode(GameModeType::SETTINGS); }
        },
        {
            {ApplicationConfig::GetScreenWidth() / 2 - buttonWidth / 2, startY + spacing * 3, buttonWidth, buttonHeight},
            "Exit",
            [this]() { /* Implement exit logic */ }
        }
    };
}

void MainMenuMode::Enter() {}

void MainMenuMode::Update() {
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    for (const auto& button : buttons) {
        SDL_Point mousePoint = { mouseX, mouseY };
        if (SDL_PointInRect(&mousePoint, &button.rect)) {
            if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                button.onClick();
            }
        }
    }
}

void MainMenuMode::Draw() {
    // Clear screen with background color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //// Draw buttons (you'll need to implement actual rendering)
    //for (const auto& button : buttons) {
    //    // Draw button and text here using your rendering system
    //    // This is pseudo-code - implement according to your rendering setup
    //    DrawRect(button.rect, buttonColor);
    //    DrawText(button.text, button.rect);
    //}
}

void MainMenuMode::Exit() {}
