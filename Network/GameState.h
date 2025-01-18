#pragma once
#include "../PlayerDirection.h"
#include "../GridManager.h"
#include "../GameObject.h"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

#include "UDPServer.h"
#include "UDPClient.h"

#include "../TerrainObject.h"
#include "../RidableObject.h"
#include "../Renderer.h"

class PlayableObject;
class GameObject;

class MovementManager;
class GridTransformManager;

class GameObjectFactory { 
public:  
    virtual ~GameObjectFactory() = default;  
    virtual std::unique_ptr<GameObject> Create() = 0;  
}; 

class PlayerFactory : public GameObjectFactory {
public: 
    std::unique_ptr<GameObject> Create() override;  
};

class ItemFactory : public GameObjectFactory {
public: 
    std::unique_ptr<GameObject> Create() override; 
}; 

class StructureComponentFactory : public GameObjectFactory {
public:
    std::unique_ptr<GameObject> Create() override;
};


// This GameState class is all that the client receives.
// All components of the game which must me synced between players 
// are all stored in this game object. 

// For the sake of Easy network processing. 
// these goals shall be met 
// 1. All information can be serialized and deserialized. 
// 2. New Sort of items should be serialized easily. 

class GameState {
public:
    std::string GetName() const;

private:
    void log(LogLevel level, std::string text);

private:
    // Current highest ID + 1, always increases
    uint32_t nextGameObjectId = 1;

    // gameobject_type_id -> gameobject_factory
    std::unordered_map<uint8_t, std::unique_ptr<GameObjectFactory>> factoryRegistry;
    // gameobject_id -> gameobject 
    std::unordered_map<uint32_t, std::unique_ptr<GameObject>> gameObjects;

    // Optional: Keep a separate map for quick type-based lookups
    std::unordered_multimap<uint8_t, uint32_t> objectsByType;

    std::unordered_map<uint8_t, PlayableObject*> playableObjects; 

    // f: CLientID -> PlayerObject
    std::unordered_map<uint32_t, PlayableObject*> players; 

    int gridHeight = 32; 
    int gridWidth = 32; 

    std::vector<uint32_t> droppedItemGrid; 
    std::vector<uint32_t> structureGrid;  
    std::vector<GroundType> groundTypeGrid;

    // Render 
    std::unique_ptr<Renderer> renderer_; 

public: 
    uint32_t GenerateNewGameObjectId() {
        return nextGameObjectId++;
    }

    void Draw() {
        // Debug 
        renderer_->DrawRespectTo(2, 1, 2);
    }

public:
    // If from Network. The resulting updates don't have to be passed on to the network. 
    // Because they are the one who informed us. 
    // If the calls are directly made, it is a desicion made locally and has to be propagated. 

    // If you told me A, there is no reason for me to repeat it for you.

    // Grid Management 
    void FoldGridIntoCubeAt(int startY, int startX, int size, bool fromNetwork);
    void CreatePortalOnGridAt(int yCoord, int xCoord, bool fromNetwork);

    // Object Management
    void CreateAndRegisterPlayerObject(uint32_t player_id);  

    void CreateAndRegisterGameObject(uint8_t typeId, bool fromNetwork);  
    // How do you know the ID of Object in Prior? When The Object's ID is given by the server :) 
    void CreateAndRegisterGameObjectWithID(uint32_t id, uint8_t typeId, bool fromNetwork); 

    void AddRidableObject(
        uint32_t objID,
        uint32_t meshID,
        uint32_t textureID,
        uint8_t gridHeight,
        uint8_t gridWidth
        )
    {
        if (objID == 0) {
            // when 0, allocate a new ID
            objID = GenerateNewGameObjectId(); 
        }

        std::unique_ptr<RidableObject> newRidableObject = std::make_unique<RidableObject>(objID, meshID, textureID, gridHeight, gridWidth);
        gameObjects[objID] = std::move(newRidableObject); 

        log(LOG_INFO, "Generated Ridable of ID: " + std::to_string(objID));
    }

    
    void RemoveGameObjectOfID(uint32_t id, bool fromNetwork);
    
    GameObject* GetGameObject(uint32_t id);

    // Hierarchical Operations
    void SetParent(uint32_t childId, uint32_t parentId, bool fromNetwork);

    // State Updates
    void UpdateGameObjectPosition(uint32_t id, int y, int x, bool fromNetwork);
    void PlayerTakeAction(uint32_t playerId, Direction input, bool fromNetwork);

    // Queries
    std::vector<GameObject*> GetObjectsByType(uint8_t typeId);
    bool IsValidGameObject(uint32_t id) const; 

public: 
    // BroadCast from Server  
    void RegisterAndBroadcastNewGameObject(GameObject* newGameObject);

    void BroadcastGameObjectRemoval(uint32_t id);

    void BroadcastGameObjectPosition(GameObject* gameObject);

    void BroadcastGameObjectParenting(uint32_t parentID, uint32_t objID); 

    std::unique_ptr<INetworkMessage> CaptureGameState();

public: 
    // send from client 
    void SendPlayerInput(Direction direction);

public: 
    GroundType GetGroundTypeAtCoord(int y, int x);  

    void DropItemAt(int y, int x, Item* item); 

public: 
    // abstract interface 
    // client::update
    void DrawGameState(); 

    // server / host 
    void UpdateGameState(float deltaTime);

    GameState() {
        isWorking = false; 


        client = nullptr; 
        server = nullptr; 

        Initialize();
    }

    GameState(GameClient* client) 
        : client(client), server(nullptr)
    {
        isWorking = true;
        isServerSide = false;  

        Initialize();
    }

    GameState(GameServer* server)
        : client(nullptr), server(server)
    {
        isWorking = true; 
        isServerSide = true;

        Initialize();
    }

    void Initialize() {
        log(LOG_INFO, "Initialize GameState"); 
        this->InitializeGrid(gridHeight, gridWidth);  
        this->InitializeRenderer(); 
    }

    void InitializeGrid(int height, int width) {
        int total_size = height * width; 

        droppedItemGrid.resize(total_size, 0);  
        structureGrid.resize(total_size, 0); 
        groundTypeGrid.resize(total_size, GroundType::GRASS); 

        movementManager = std::make_unique<MovementManager>();
        gridTransformManager = std::make_unique<GridTransformManager>();
    } 

    void InitializeRenderer() {
        renderer_ = std::make_unique<Renderer>(this);
    }

public: 
    // server (& client potentially)
    std::unique_ptr<MovementManager> movementManager; 

    // Client 
    std::unique_ptr<GridTransformManager> gridTransformManager;


public:  
    // A bit messy but who cares 
    bool isWorking = false;
    bool isServerSide = false;  
    GameClient* client; 
    GameServer* server;  
};



