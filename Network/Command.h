#pragma once
#include <cstdint>
#include <memory>
#include "../PlayerDirection.h"

// Forward declarations 
class GameState;

// Messages <-> Commands 
// Messages correspond to commands 
// however Messages only act on our brain. 
// We make it possible for Messages to act upon States by 
// Processing Messages to Commands which can directly act on GameStates 

// Abstract base class for all game commands
// GameState -> GameState
class IGameCommand { 
public: 
    virtual ~IGameCommand() = default; 
    virtual void Execute(GameState& gameState) = 0; 
}; 

//// Example command for updating player position
//// GameState -> GameState
//class UpdatePlayerPositionCommand : public IGameCommand {
//    float x, y, z;
//    uint32_t player_id;
//
//public:
//    UpdatePlayerPositionCommand(float x, float y, float z, uint32_t id)
//        : x(x), y(y), z(z), player_id(id) {}
//
//    void Execute(GameState& gameState) override {
//        // Update player position in game state
//        // This is where the actual state modification happens
//        gameState.UpdatePlayerPosition(player_id, x, y, z);
//    }
//};

class PlayerInputCommand : public IGameCommand {
    Direction user_input; 
    uint32_t player_id; 

public: 
    PlayerInputCommand(Direction userInput, uint32_t playerID);

    void Execute(GameState& gameState) override;
}; 

class AddGameObjectCommand : public IGameCommand {
    uint8_t gameobject_type_id; 
    uint32_t gameobject_id; 

public: 
    AddGameObjectCommand(uint8_t gameObjectTypeID, uint32_t gameObjectID);

    void Execute(GameState& gameState) override;
};

class RemoveGameObjectCommand : public IGameCommand {
    uint32_t gameobject_id; 

public: 
    RemoveGameObjectCommand(uint32_t gameObjectID);

    void Execute(GameState& gameState) override;
};

class GameObjectPositionCommand : public IGameCommand {
    int y_coord; 
    int x_coord; 

    uint32_t gameobject_id;  

public: 
    GameObjectPositionCommand(int y, int x, uint32_t gameObjectID);

    void Execute(GameState& gameState) override;
};

class GameObjectParentCommand : public IGameCommand {
    uint32_t parent_object_id; 
    uint32_t gameobject_id; 

public: 
    GameObjectParentCommand(uint32_t parentObjectID, uint32_t gameObjectID);

    void Execute(GameState& gameState) override;
}; 


