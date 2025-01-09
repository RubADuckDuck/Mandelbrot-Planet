#pragma once

#include "GameMode.h"

class HostPlayingMode : public GameMode {
private:
    // Track whether we're in a networked game
    bool isNetworked = true; 

public:
    HostPlayingMode(GameEngine* engine);

    ~HostPlayingMode(); 

    void Enter() override;

    void Update() override;

    void Draw() override;

    void Exit() override;
};

