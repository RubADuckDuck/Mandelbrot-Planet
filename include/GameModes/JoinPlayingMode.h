#pragma once

#include "../Core/GameMode.h"

class TerrainManager;

class JoinPlayingMode : public GameMode {
private:
    // Track whether we're in a networked game
    bool isNetworked = true;

public:

    JoinPlayingMode(GameEngine* engine);

    ~JoinPlayingMode();

    void Enter() override;

    void Update() override;

    void Draw() override;

    void Exit() override;
}; 

