#pragma once

#include "../Core/GameMode.h"
#include "../Network/UDPServer.h"  // Your networking code
#include "../Core/GameEngine.h"
#include <asio.hpp>

class HostLobbyMode : public GameMode {
private:
    std::unique_ptr<GameServer> server;
    std::vector<std::string> connectedPlayers;  // List of connected players


public:
    HostLobbyMode(GameEngine* engine);

    void Enter() override;

    void Update() override; 

    void Update(float delta_time) override; 

    void Draw() override;

    void Exit() override;

private: 
    void TestRendering(); 
}; 

