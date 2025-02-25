#include "GameModes/PlayingMode.h"
#include "Core/GameEngine.h"

PlayingMode::PlayingMode(GameEngine* engine) : GameMode(engine), isNetworked(false) {}

PlayingMode::~PlayingMode() {
    
}

void PlayingMode::Enter() {
    
}

void PlayingMode::Update() {
    // Your existing game update logic from MainLoop

}

void PlayingMode::Update(float delta_time)
{
}

void PlayingMode::Draw() {
    // Your existing game rendering logic

}

void PlayingMode::Exit() {
    // Clean up when leaving playing mode

}
