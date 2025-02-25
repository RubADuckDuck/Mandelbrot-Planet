#include "Network/Command.h"
#include "Network/GameState.h"
#include "Network/NetworkMessage.h"
#include "Utils/LOG.h" 
#include "Core/Item.h"
#include "Core/RidableObject.h"
#include "Core/GameObject.h"

PlayerInputCommand::PlayerInputCommand(Direction userInput, uint32_t playerID)
    : user_input(userInput), player_id(playerID) {}

void PlayerInputCommand::Execute(GameState& gameState) {
    LOG(LOG_INFO, "PlayerInput \n  Player id: " + std::to_string(player_id) + "\n  User Input: " + direction2String[user_input]);
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

UdpVerificationCommand::UdpVerificationCommand(uint32_t session_id, uint64_t verification_code, uint64_t timestamp) : session_id(session_id), verification_code(verification_code), timestamp(timestamp) {}

void UdpVerificationCommand::Execute(GameState& gameState) {
    if (gameState.isServerSide) {
        // verify code 
        gameState.server->verify_pending_udp_connection(verification_code);
    }
    else {
        // send back verification message through udp 
        INetworkMessage* newMessage = new UdpVerificationMessage(session_id, verification_code);

        gameState.client->send_message(newMessage, true);

        delete newMessage;
    }
}


std::string IGameCommand::GetName() const { return "IGameCommand"; }

void IGameCommand::log(LogLevel level, std::string text) {
	LOG(level, GetName() + "::" + text);
}

void WalkOnRidableObjectCommand::Execute(GameState& gameState)
{
	tempGameState = &gameState; 

	RidableObject* walkerGameObject = dynamic_cast<RidableObject*>(gameState.GetGameObject(this->walkerID)); 
	if (walkerGameObject != nullptr) {
		log(LOG_ERROR, "This GameObject is not a RidableObject, therefore cannot walk"); 
		return; 
	}

	Direction curDirection = this->direction; 

	uint32_t parentID = walkerGameObject->GetParentID();   

	if (parentID == 0) {
		log(LOG_WARNING, "The object which is trying to walk has no parent, therefore missing a grid. You don't have a space to walk on");  
		return; 
	}
	else {
		RidableObject* parentObj = dynamic_cast<RidableObject*>(gameState.GetGameObject(parentID));

		if (parentObj == nullptr) {
			log(LOG_ERROR, "Can't walk on this");
			return;
		}

		// get 'from where'
		Coord2d curWalkerPos = parentObj->GetPosition(walkerID);
		
		this->Walk(walkerGameObject, curDirection, curWalkerPos, parentObj); 
	}

}

void WalkOnRidableObjectCommand::Interact(RidableObject* who, Direction did_what, GameObject* to_whom)
{
	uint32_t previousParentID = who->GetParentID(); 
	RidableObject* prevParentObj = dynamic_cast<RidableObject*>(tempGameState->GetGameObject(previousParentID));

	if (prevParentObj == nullptr) {
		log(LOG_ERROR, "Can't walk on this");
		return;
	}

	if (RidableObject* ptrRidable = dynamic_cast<RidableObject*>(to_whom)) {
		// if target object is ridable, ride on it.  
		// {A(B, P), B(A')} -> {A(B), B(A', P)}
		// When, P enter B at A...
		// 1. P's parent is now B. 
		// 2. P's position at B is move(B::A', direction, direction) / how can we trigger this walk starting from B::A'? 

		// A(B, P) -> A(B)
		prevParentObj->RemoveChildAtGrid(this->walkerID);
		
		// A::P -> B::P 
		who->SetParentObjectAndExit(ptrRidable->GetID()); 

		// Player moves from A's exit at B, which we notate as B::A' 
		// get position of B::A'  
		Coord2d walk_from = ptrRidable->GetPosition(previousParentID);
		
		// B(A') -> B(A', P)
		this->Walk(who, did_what, walk_from, ptrRidable); 
	}


}
void WalkOnRidableObjectCommand::Walk(RidableObject* who, Direction to_where, Coord2d from_where, RidableObject* walking_on)
{
	NavigationInfo curNavigationInfo = walking_on->GetMovementManager()->Move(from_where, to_where, to_where);

	uint32_t objIDAtPos = walking_on->GetObjectIDAt(curNavigationInfo.pos);
	GameObject* objAtPos = tempGameState->GetGameObject(objIDAtPos);

	if (objIDAtPos != 0) {
		// well something exists.  
		// Information needed for describing interaction. 
		// 1. who 2. did what 3. to whom?
		this->Interact(who, direction, objAtPos);
	}
	else {
		// is not occupied 

		// just walk. no problem
		walking_on->SetObjIdAtPos(curNavigationInfo.pos, who->GetID());
	}
}



