#include "GameState.h" 
#include "NetworkMessage.h"

#include "../GameObject.h" 
#include "../RidableObject.h"

std::string GameState::GetName() const { return "GameState"; }

void GameState::log(LogLevel level, std::string text) {
    LOG(level, GetName() + "::" + text);
}

uint32_t GameState::GenerateNewGameObjectId() {
    return nextGameObjectId++;
}

void GameState::Draw() {
    // Debug 
    renderer_->DrawRespectTo(2, 1, 3);
}

// Grid Management 
void GameState::FoldGridIntoCubeAt(int startY, int startX, int size, bool fromNetwork) {

    if (gridTransformManager != nullptr) {
        gridTransformManager->InitTransforms(startY, startX, size);
    }
    if (movementManager != nullptr) {
        movementManager->InitPlanarFigure(startY, startX, size);
    }
}

void GameState::CreatePortalOnGridAt(int yCoord, int xCoord, bool fromNetwork) {
    // to do
}

void GameState::CreateAndRegisterPlayerObject(uint32_t player_id)
{
    

    GameObject* newPlayer = new PlayableObject();  

    uint32_t newID = GenerateNewGameObjectId();  

    newPlayer->SetID(newID); 

    log(LOG_INFO, "Creating Playerable Object for player id: " + std::to_string(player_id) + "  objID: " + std::to_string(newID));

    // we could think of Using CreateGameObject. However, if the call of this method encompases all the functions, we don't have to pass GameObject Creation message 
    gameObjects[newID] = std::unique_ptr<GameObject>(newPlayer);    
    players[player_id] = dynamic_cast<PlayableObject*>(newPlayer);   

    return; 
}

void GameState::CreateAndRegisterGameObject(uint8_t typeId, bool fromNetwork)
{
    // Create GameObject with factory 
    std::unique_ptr<GameObject> newObject = factoryRegistry[typeId]->Create();  

    uint32_t newID = GenerateNewGameObjectId();  

    newObject->SetID(newID);

    log(LOG_INFO, "Generated GameObject id of typeId: " + std::to_string(typeId) + "  ObjID: " + std::to_string(newID));  

    gameObjects[newID] = std::move(newObject);  
    gameObjects[newID] = std::move(newObject);  
    objectsByType.insert({typeId, newID});  

    return;  
}

void GameState::CreateAndRegisterGameObjectWithID(uint32_t id, uint8_t typeId, bool fromNetwork) {
    if (!fromNetwork) {
        // propagate update to server
    }

    std::unique_ptr<GameObject> newGameObject;
    newGameObject = factoryRegistry[typeId]->Create();

    newGameObject->SetID(id);

    gameObjects[id] = std::move(newGameObject);

    // Add the object to the objectsByType map
    objectsByType.insert({ typeId, id });

    return;
}

void GameState::AddRidableObject(uint32_t objID, uint32_t meshID, uint32_t textureID, uint8_t gridHeight, uint8_t gridWidth)
{
    if (objID == 0) {
        // when 0, allocate a new ID
        objID = GenerateNewGameObjectId();
    }

    std::unique_ptr<RidableObject> newRidableObject; 

    if (gridWidth != 0) {
        newRidableObject = std::make_unique<RidableObject>(objID, meshID, textureID, gridHeight, gridWidth);
    }
    else {
        newRidableObject = std::make_unique<RidableObject>(objID, meshID, textureID, gridHeight);
    }

    gameObjects[objID] = std::move(newRidableObject);

    log(LOG_INFO, "Generated Ridable of ID: " + std::to_string(objID));
}

void GameState::RemoveGameObjectOfID(uint32_t id, bool fromNetwork) {
    if (!fromNetwork) {
        // propagate update to server
    }

    // Find and erase the GameObject from the gameObjects map
    auto it = gameObjects.find(id);
    if (it != gameObjects.end()) {
        gameObjects.erase(it);

        // Remove the object from the objectsByType map
        auto range = objectsByType.equal_range(it->second->GetTypeID());

        for (auto it2 = range.first; it2 != range.second; ++it2) {
            if (it2->second == id) {
                objectsByType.erase(it2);
                break;
            }
        }
    }
}

GameObject* GameState::GetGameObject(uint32_t id) {

    auto it = gameObjects.find(id);
    if (it != gameObjects.end()) {
        return it->second.get();  // Returns raw pointer without transferring ownership
    }
    // Log an error 
    return nullptr;
}

// Hierarchical Operations
void GameState::SetParent(uint32_t childId, uint32_t parentId, bool fromNetwork) {
    // to do 
    
    //if (!fromNetwork) {
    //    // propagate update to server
    //}

    //GameObject* pChild = gameObjects[childId].get();
    //GameObject* pParent = gameObjects[parentId].get();

    //pChild->SetParent(pParent);
    //pParent->AddChild(pChild);
}

// Version 1: Safer map access and cleaner dynamic_cast
void GameState::UpdateGameObjectPosition(uint32_t id, int y, int x, bool fromNetwork) { 
    // to do 
    //if (!fromNetwork) {
    //    // propagate update to server
    //}

    //auto it = gameObjects.find(id);
    //if (it != gameObjects.end()) {
    //    if (auto* gridObject = dynamic_cast<GameObjectOnGrid*>(it->second.get())) {
    //        gridObject->SetCoordinates(y, x);
    //    }
    //}
}

std::vector<GameObject*> GameState::GetObjectsByType(uint8_t typeId) {
    std::vector<GameObject*> objects;

    // Find all objects with the given typeId in the objectsByType map
    auto range = objectsByType.equal_range(typeId);
    for (auto it = range.first; it != range.second; ++it) {
        // Get the GameObject pointer from the gameObjects map
        auto gameObjectIt = gameObjects.find(it->second);
        if (gameObjectIt != gameObjects.end()) {
            objects.push_back(gameObjectIt->second.get());
        }
    }

    return objects;
} 

bool GameState::IsValidGameObject(uint32_t id) const {
    return true; 
}

// BroadCast from Server  
void GameState::RegisterAndBroadcastNewGameObject(GameObject* newGameObject) {
    uint32_t newID = GenerateNewGameObjectId(); 
    uint8_t objTypeID = 0; 

    // turn raw pointer unique  
    std::unique_ptr<GameObject> pGameObject(newGameObject);
    // set ID of gameObject 
    newGameObject->SetID(newID);
    gameObjects[newID] = std::move(pGameObject);
    // to do: add to objects by type. This is not currently possible. 

    // Create Message 
    INetworkMessage* curMessage = new AddGameObjectMessage(objTypeID, newID);

    // Broadcast message through server  
    server->broadcast_message(curMessage);

    // Message is turned into data and sent through the server it is safe to delete 
    delete curMessage;
}

void GameState::BroadcastGameObjectRemoval(uint32_t id) {
    INetworkMessage* curMessage = new RemoveGameObjectMessage(id);

    server->broadcast_message(curMessage);

    delete curMessage;
}

// GameObjects don't set 
//void GameState::BroadcastGameObjectPosition(GameObject* gameObject) {
//    if (auto gridObject = dynamic_cast<GameObjectOnGrid*>(gameObject)) {
//        INetworkMessage* curMessage = new GameObjectPositionMessage(
//            gridObject->yCoord,
//            gridObject->xCoord,
//            gridObject->GetID()
//        );
//
//        server->broadcast_message(curMessage);
//
//        delete curMessage;
//    }
//    else {
//        // this gameObject has no position
//    }
//}

void GameState::BroadcastGameObjectParenting(uint32_t parentID, uint32_t objID) {
    INetworkMessage* curMessage = new GameObjectParentObjectMessage(parentID, objID);

    server->broadcast_message(curMessage);

    delete curMessage;
}

std::unique_ptr<INetworkMessage> GameState::CaptureGameState() {

    // to do : create a child class of INetworkMessages that has all the infromation of current state of the game. 
    return std::move(std::unique_ptr<INetworkMessage>(new PlayerInputMessage()));
}

// send from client 

void GameState::SendPlayerInput(Direction direction) {
    uint32_t playerID = 0;

    INetworkMessage* curMessage = new PlayerInputMessage(
        direction, playerID
    );

    client->send_message(curMessage, true);

    delete curMessage;
}

// We don't set parents this way
//// Hierarchical Operations
//void GameState::SetParent(uint32_t childId, uint32_t parentId, bool fromNetwork) {
//    if (!fromNetwork) {
//        // propagate update to server
//    }
//
//    GameObject* pChild = gameObjects[childId].get();
//    GameObject* pParent = gameObjects[parentId].get();
//
//    pChild->SetParent(pParent);
//    pParent->AddChild(pChild);
//}

// No more Object On Grid
//// Version 1: Safer map access and cleaner dynamic_cast
//void GameState::UpdateGameObjectPosition(uint32_t id, int y, int x, bool fromNetwork) {
//    if (!fromNetwork) {
//        // propagate update to server
//    }
//
//    auto it = gameObjects.find(id);
//    if (it != gameObjects.end()) {
//        if (auto* gridObject = dynamic_cast<GameObjectOnGrid*>(it->second.get())) {
//            gridObject->SetCoordinates(y, x);
//        }
//    }
//}

void GameState::PlayerTakeAction(uint32_t playerId, Direction input, bool fromNetwork) {
    if (!fromNetwork) {
        // propagate update to server
    }
    
    auto it = players.find(playerId);  

    if (it == players.end()) {
        log(LOG_INFO, "Player id: " + std::to_string(playerId) + " is invalid");  
    }
    else {
        // take action
        log(LOG_INFO, "Player id: " + std::to_string(playerId) + " taking action : " + direction2String[input]);
        players[playerId]->TakeAction(input);
        return;
    }

}

// No more Factory Component objects 
//FactoryComponentObject* GameState::GetStructureAtCoord(int y, int x)
//{
//    uint32_t id = structureGrid[y * gridWidth + x];
//    
//    return dynamic_cast<FactoryComponentObject*>(gameObjects[id].get());
//}

GroundType GameState::GetGroundTypeAtCoord(int y, int x)
{
    return groundTypeGrid[y * gridWidth + x];
}

// No More Dropped Item
//DroppedItemObject* GameState::GetDroppedItemAtCoord(int y, int x)
//{
//    uint32_t id = structureGrid[y * gridWidth + x];
//
//    return dynamic_cast<DroppedItemObject*>(gameObjects[id].get());
//}

void GameState::DropItemAt(int y, int x, Item* item)
{
    // Publish Creation of new dropped Item.  

    // Publish position change of (y, x) 

    // Pushlish parent-child relation of droppedItem-item
}

void GameState::DrawGameState()
{
}

void GameState::UpdateGameState(float deltaTime) {
    GameObject* curGameObject;
    for (auto& pObject : gameObjects) {
        curGameObject = pObject.second.get();

        curGameObject->Update(deltaTime);
    }
}