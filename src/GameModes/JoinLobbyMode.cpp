#include "GameModes/JoinLobbyMode.h"
#include "Core/GameEngine.h"

JoinLobbyMode::JoinLobbyMode(GameEngine* engine) : GameMode(engine), isConnected(false) {}

void JoinLobbyMode::Enter() {

    // You might want to show a UI for entering server address
    serverAddress = "127.0.0.1";  // Default to localhost for testing 

    unsigned short tcp_port = 10429;
    unsigned short udp_port = 20429; 
    
    // Initialize client when entering join mode
    client = std::make_unique<GameClient>(gameEngine->GetIOContext(), tcp_port, udp_port); 

    client->connect(gameEngine->GetIOContext(), serverAddress);

    gameEngine->RunIOContextOnIOThread(); 
}

void JoinLobbyMode::Update() {

}

void JoinLobbyMode::Draw() {

}

void JoinLobbyMode::Exit() {
    client.reset();
    isConnected = false;
}
