#pragma once
#pragma once
#include "../Core/GameMode.h"

class PlayingMode : public GameMode {
private:
    // Track whether we're in a networked game
    bool isNetworked = false;

public:
    PlayingMode(GameEngine* engine);

    ~PlayingMode();

    void Enter() override;

    void Update() override;

    void Draw() override;

    void Exit() override;
}; 

