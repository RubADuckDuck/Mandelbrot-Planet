#include "HostLobbyMode.h"

HostLobbyMode::HostLobbyMode(GameEngine* engine) : GameMode(engine) {}

void HostLobbyMode::Enter() {
    // Initialize the server when entering host mode
    // server = std::make_unique<GameServer>();
    // server->Start();  // Your server initialization
}

void HostLobbyMode::Update() {
    // Check for new connections
    // server->Update();

    // If player clicks "Start Game" and we have players connected
    if (/* start game button clicked */ false && !connectedPlayers.empty()) {
        // Switch to PlayingMode
        gameEngine->GetModeController()->SwitchMode(GameModeType::PLAYING);
    }
}

void HostLobbyMode::Draw() {
    // Draw the lobby UI
    // You'll need to implement actual rendering
    // Show connected players
    // Show "Start Game" button when ready
}

void HostLobbyMode::Exit() {
    // Clean up server when leaving
    server.reset();
    connectedPlayers.clear();
}
