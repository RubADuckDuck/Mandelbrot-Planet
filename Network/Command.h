#pragma once
#include <cstdint>
#include <memory>
#include "../PlayerDirection.h"
#include "../LOG.h"
#include "../GameObject.h"
#include "../RidableObject.h"
#include "GameState.h"

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
    std::string GetName() const;

protected:
    virtual void log(LogLevel level, std::string text); 

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
class UdpVerificationCommand : public IGameCommand {
    uint32_t session_id;

    // Keeping the 64-bit verification code
    uint64_t verification_code;

    // Keeping the 64-bit timestamp
    uint64_t timestamp; 

public: 
    UdpVerificationCommand(
        uint32_t session_id, 
        uint64_t verification_code, 
        uint64_t timestamp 
    );

    void Execute(GameState& gameState) override;
};

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

// v1.0 ----------------------------------------------------
class AddRidableObjectCommand : public IGameCommand {
    uint32_t objID_;
    uint32_t meshID_;
    uint32_t textureID_;
    uint8_t gridHeight_;
    uint8_t gridWidth_;
public:
    std::string GetName() const {
        return "AddRidableObjectCommand";
    }
public:
    AddRidableObjectCommand(
        uint32_t objID,
        uint32_t meshID,
        uint32_t textureID,
        uint8_t gridHeight,
        uint8_t gridWidth
    ) :
        objID_(objID),
        meshID_(meshID),
        textureID_(textureID),
        gridHeight_(gridHeight),
        gridWidth_(gridWidth)
    {}

    void Execute(GameState& gameState) override {
        gameState.AddRidableObject(
            objID_,
            meshID_,
            textureID_,
            gridHeight_,
            gridWidth_
        );
    }
};

class WalkOnRidableObjectCommand : public IGameCommand {
    uint32_t walkerID;

    Direction direction;


    // -----------------------------
    GameState* tempGameState;
public:
    std::string GetName() const {
        return "WalkOnRidableObjectCommand";
    }

public:
    WalkOnRidableObjectCommand(uint32_t walkerID, Direction direction)
        : walkerID(walkerID), direction(direction), tempGameState(nullptr) {}

    void Execute(GameState& gameState) override; 
    void Interact(RidableObject* who, Direction did_what, GameObject* to_whom);
    void Walk(RidableObject* who, Direction to_where, Coord2d from_where, RidableObject* walking_on); 
};

class RideOnRidableObjectCommand : public IGameCommand {
    uint32_t vehicleID;
    uint32_t riderID;

    uint8_t rideAt;

public:
    std::string GetName() const {
        return "RideOnRidableObjectCommand";
    }
public:
    RideOnRidableObjectCommand(uint32_t vehicleID, uint32_t riderID, uint8_t rideAt)
        : vehicleID(vehicleID), riderID(riderID), rideAt(rideAt) {}

    void Execute(GameState& gameState) override {
        RidableObject* vehicleObj = dynamic_cast<RidableObject*>(gameState.GetGameObject(vehicleID));  
        
        // I am torn between whether I should let Non-ridable objects Ride.
        // I made up this rule to settle my mind. 
        // You can only ride, if you let others ride yourself. 
        RidableObject* riderObj = dynamic_cast<RidableObject*>(gameState.GetGameObject(vehicleID)); 

        riderObj->SetParentObjectAndExit(vehicleID); 

        vehicleObj->SetObjIdAtPos(rideAt, riderID);

        return; 
    }
    
};

