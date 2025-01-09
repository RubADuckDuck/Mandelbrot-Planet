#pragma once
#pragma once
#include "GameMode.h"

class TerrainManager;

class PlayingMode : public GameMode {
private:
    TerrainManager* terrainManager;
    // Track whether we're in a networked game
    bool isNetworked;

public:
    PlayingMode(GameEngine* engine);

    ~PlayingMode();

    void Enter() override;

    void Update() override;

    void Draw() override;

    void Exit() override;
};