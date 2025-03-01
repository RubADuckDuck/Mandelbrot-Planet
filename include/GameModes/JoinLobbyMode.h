#pragma once
#include "../Core/GameMode.h"
#include "../Network/UDPClient.h"  // Your networking code

class JoinLobbyMode : public GameMode {
private:
    std::unique_ptr<GameClient> client;
    bool isConnected;
    std::string serverAddress;

public:
    JoinLobbyMode(GameEngine* engine);

    void Enter() override;
    void Update() override;
    void Update(float delta_time) override;
    void Draw() override;
    void Exit() override;
};

