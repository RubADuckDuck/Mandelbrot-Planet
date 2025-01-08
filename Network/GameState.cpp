#include "GameState.h" 
#include "../GameObject.h" 
#include "NetworkMessage.h"

void GameState::CreateAndRegisterGameObjectWithID(uint32_t id, uint8_t typeId, bool fromNetwork) {
    if (!fromNetwork) {
        // propagate update to server
    }

    std::unique_ptr<GameObject> newGameObject;
    newGameObject = factoryRegistry[typeId]->Create();

    gameObjects[id] = std::move(newGameObject);

    // Add the object to the objectsByType map
    objectsByType.insert({ typeId, id });

    return;
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

    // turn raw pointer unique  
    std::unique_ptr<GameObject> pGameObject(newGameObject);
    // set ID of gameObject 
    newGameObject->SetID(newID);
    gameObjects[newID] = std::move(pGameObject);
    // to do: add to objects by type. This is not currently possible. 

    // Create Message 
    INetworkMessage* curMessage = new AddGameObjectMessage(0, newID);

    // Broadcast message through server  
    server->broadcast_message(curMessage);

    // Message is turned into data and sent through the server it is safe to delete 
    delete curMessage;
}

inline void GameState::BroadcastGameObjectRemoval(uint32_t id) {
    INetworkMessage* curMessage = new RemoveGameObjectMessage(id);

    server->broadcast_message(curMessage);

    delete curMessage;
}

inline void GameState::BroadcastGameObjectPosition(GameObject* gameObject) {
    if (auto gridObject = dynamic_cast<GameObjectOnGrid*>(gameObject)) {
        INetworkMessage* curMessage = new GameObjectPositionMessage(
            gridObject->yCoord,
            gridObject->xCoord,
            gridObject->GetID()
        );

        server->broadcast_message(curMessage);

        delete curMessage;
    }
    else {
        // this gameObject has no position
    }
}

inline void GameState::BroadcastGameObjectParenting(uint32_t parentID, uint32_t objID) {
    INetworkMessage* curMessage = new GameObjectParentObjectMessage(parentID, objID);

    server->broadcast_message(curMessage);

    delete curMessage;
}

// send from client 

inline void GameState::SendPlayerInput(Direction direction) {
    uint32_t playerID = 0;

    INetworkMessage* curMessage = new PlayerInputMessage(
        direction, playerID
    );

    client->send_message(curMessage);

    delete curMessage;
}

// Hierarchical Operations
void GameState::SetParent(uint32_t childId, uint32_t parentId, bool fromNetwork) {
    if (!fromNetwork) {
        // propagate update to server
    }

    GameObject* pChild = gameObjects[childId].get();
    GameObject* pParent = gameObjects[parentId].get();

    pChild->SetParent(pParent);
    pParent->AddChild(pChild);
}

// Version 1: Safer map access and cleaner dynamic_cast
void GameState::UpdateGameObjectPosition(uint32_t id, int y, int x, bool fromNetwork) {
    if (!fromNetwork) {
        // propagate update to server
    }

    auto it = gameObjects.find(id);
    if (it != gameObjects.end()) {
        if (auto* gridObject = dynamic_cast<GameObjectOnGrid*>(it->second.get())) {
            gridObject->SetCoordinates(y, x);
        }
    }
}

void GameState::PlayerTakeAction(uint32_t playerId, Direction input, bool fromNetwork) {
    if (!fromNetwork) {
        // propagate update to server
    }

    // take action
    playableObjects[playerId]->TakeAction(input);

    return;
}

void GameState::UpdateGameState(float deltaTime) {
    GameObject* curGameObject;
    for (auto& pObject : gameObjects) {
        curGameObject = pObject.second.get();

        curGameObject->Update(deltaTime);
    }
}