#pragma once
#include <unordered_map> 
#include <string> 

// Different modes our game can be in
enum class GameModeType {
    MAIN_MENU,
    PLAYING,
    SETTINGS,
    HOST_LOBBY,
    JOIN_LOBBY, 
    HOST_PLAYING, 
    CLIENT_PLAYING
};

extern std::unordered_map<GameModeType, std::string> mode2string;

class GameEngine;

class GameMode {
protected:
    GameEngine* gameEngine;  // Reference to the main game engine

public:
    GameMode(GameEngine* engine) : gameEngine(engine) {}
    virtual ~GameMode() = default;

    // Core functions that every game mode must implement
    virtual void Enter() = 0;   // Called when entering this mode
    virtual void Update() = 0;  // Called each frame for logic updates
    virtual void Draw() = 0;    // Called each frame for rendering
    virtual void Exit() = 0;    // Called when leaving this mode
}; 

