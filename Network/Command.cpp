#include "Command.h"
#include "GameState.h"

inline PlayerInputCommand::PlayerInputCommand(Direction userInput, uint32_t playerID)
    : user_input(userInput), player_id(playerID) {}

inline void PlayerInputCommand::Execute(GameState& gameState) {
    gameState.PlayerTakeAction(player_id, user_input, true);
}

AddGameObjectCommand::AddGameObjectCommand(uint8_t gameObjectTypeID, uint32_t gameObjectID)
    : gameobject_type_id(gameObjectTypeID), gameobject_id(gameObjectID) {}

void AddGameObjectCommand::Execute(GameState& gameState) {
    gameState.CreateAndRegisterGameObjectWithID(gameobject_id, gameobject_type_id, true);
}

RemoveGameObjectCommand::RemoveGameObjectCommand(uint32_t gameObjectID)
    : gameobject_id(gameObjectID) {}

void RemoveGameObjectCommand::Execute(GameState& gameState) {
    gameState.RemoveGameObjectOfID(gameobject_id, true);
}

GameObjectPositionCommand::GameObjectPositionCommand(int y, int x, uint32_t gameObjectID)
    : y_coord(y), x_coord(x), gameobject_id(gameObjectID) {}

void GameObjectPositionCommand::Execute(GameState& gameState) {
    gameState.UpdateGameObjectPosition(gameobject_id, y_coord, x_coord, true);
}

GameObjectParentCommand::GameObjectParentCommand(uint32_t parentObjectID, uint32_t gameObjectID)
    : parent_object_id(parentObjectID), gameobject_id(gameObjectID) {}

void GameObjectParentCommand::Execute(GameState& gameState) {
    gameState.SetParent(gameobject_id, parent_object_id, true);
}
