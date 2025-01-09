#include "JoinLobbyMode.h"

JoinLobbyMode::JoinLobbyMode(GameEngine* engine) : GameMode(engine), isConnected(false) {}

void JoinLobbyMode::Enter() {
    // Initialize client when entering join mode
    client = std::make_unique<GameClient>();
    // You might want to show a UI for entering server address
    serverAddress = "127.0.0.1";  // Default to localhost for testing
}

void JoinLobbyMode::Update() {
    //if (!isConnected) {
    //    // Try to connect if we haven't yet
    //    if (client->Connect(serverAddress, 12345)) {  // Your connection port
    //        isConnected = true;
    //    }
    //}

    //// Once connected, wait for host to start game
    //if (isConnected && client->HasGameStarted()) {
    //    gameEngine->GetModeController()->SwitchMode(GameModeType::PLAYING);
    //}
}

void JoinLobbyMode::Draw() {
    // Draw the join lobby UI
    // Show connection status
    // Show "Waiting for host" message when connected
}

void JoinLobbyMode::Exit() {
    client.reset();
    isConnected = false;
}
