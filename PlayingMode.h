#pragma once
#pragma once
#include "GameMode.h"

class PlayingMode : public GameMode {
private:
    // Track whether we're in a networked game
    bool isNetworked = false;

public:
    PlayingMode(GameEngine* engine);

    ~PlayingMode();

    void Enter() override;

    void Update() override;
    void Update(float delta_time) override;

    void Draw() override;

    void Exit() override;
}; 

