#include "HostLobbyMode.h"

HostLobbyMode::HostLobbyMode(GameEngine* engine) : GameMode(engine) {}

void HostLobbyMode::Enter() {
    unsigned short tcp_port = 10429; 
    unsigned short udp_port = 20429;


    LOG(LOG_INFO, "HostLobbyMode::Initializing Gameserver");
    // Initialize the server when entering host mode
    server = std::make_unique<GameServer>(*(gameEngine->GetIOContext()), tcp_port, udp_port);

    LOG(LOG_INFO, "HostLobbyMode::Run IO Context on IO Thread"); 

    gameEngine->RunIOContextOnIOThread(); 
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
