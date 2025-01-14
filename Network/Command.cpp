#include "Command.h"
#include "GameState.h"
#include "NetworkMessage.h"
#include "../LOG.h" 
#include "../TerrainObject.h"
#include "../Item.h"
#include "../RidableObject.cpp"

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

std::string InteractionInfoCommand::GetName() const { return "InteractionInfoCommand"; }

InteractionInfoCommand::InteractionInfoCommand(uint32_t item, uint32_t who, int yCoord, int xCoord, Direction goingWhere)
    : heldItemID(item), whoID(who), yCoord(yCoord), xCoord(xCoord), goingWhere(goingWhere)  
{

}

void InteractionInfoCommand::Execute(GameState& gameState)
{
    uint32_t itemID = heldItemID;  
    uint32_t actorID = whoID;  
    int y = yCoord;  
    int x = xCoord;  
    Direction curDirection = goingWhere;

    Item* ptrItem = dynamic_cast<Item*>(gameState.GetGameObject(itemID));  

    GameObject* ptrActor = gameState.GetGameObject(whoID);  

    bool actorIsPlayer = false;
    int targetY = y;
    int targetX = x;

	// directionActOnCoord(curDirection, targetY, targetX, this->GRID_SIZE, this->GRID_SIZE);
	NavigationInfo curCoord2dandDir = gameState.movementManager->Move({ y, x }, curDirection, curDirection);
	targetY = curCoord2dandDir.pos.first;
	targetX = curCoord2dandDir.pos.second; 

	GroundType currTargetGroundType = gameState.GetGroundTypeAtCoord(targetY, targetX);  
	FactoryComponentObject* currTargetFactory = gameState.GetStructureAtCoord(targetY, targetX);
	DroppedItemObject* currTargetDroppedItem = gameState.GetDroppedItemAtCoord(targetY, targetX);


    if (ptrActor == nullptr) { 
        // no Actor?... What? 

        return;  
    }

	if (PlayableObject* ptrPlayerActor = dynamic_cast<PlayableObject*>(ptrActor)) {
		actorIsPlayer = true; 
		ptrPlayerActor->direction = curCoord2dandDir.direction;
		ptrPlayerActor->orientation += curCoord2dandDir.changeOfOrientation;
		ptrPlayerActor->orientation = PositiveModulo(ptrPlayerActor->orientation, 4);
	}

	if (curDirection != Direction::IDLE) {
		// Somebody Requested Walk 
		if (currTargetFactory != nullptr) {
			if (actorIsPlayer) {
				// throw current held Item to target

				// if factory exist in target
				log(LOG_INFO, "Attempt to interact with factory");
				if (currTargetFactory->componentType == INPUTPORT) {
					// if factory component is input port 
					log(LOG_INFO, "Interact with inputPort of factory"); 

					currTargetFactory->Interact(ptrItem);

					// drop players item
					PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(ptrActor); 
					ptrPlayer->heldItem = nullptr;

					return; // end interaction
				}
				else {
					// you can do nothing 
					return;
				}
			}
			else {
				// you can't walk on factories

				return;
			}

		}
		else if (currTargetGroundType == GroundType::WATER) {
			// throw in water? fishing?
			if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(ptrActor)) {
				// if ptrPlayer->heldItem->itemType == FISHING_ROD // todo
			}
			else {
				// you can't walk on water. Or can you?
			}

			return;
		}
		else {
			// No water no factory
			// you can walk now. 
			if (GameObjectOnGrid* gameobjectOnGrid = dynamic_cast<GameObjectOnGrid*>(ptrActor)) {
				gameobjectOnGrid->SetCoordinates(targetY, targetX);
			}
			return;
		}
		return;
	}
	else {
		// is IDLE 
		if (currTargetFactory != nullptr && currTargetFactory->componentType == INPUTPORT) {
			// the gameobject is facing the input port
			if (actorIsPlayer) {
				// interact with factory?
			}
			else {
				// if factory component is input port 
				log(LOG_INFO, "Non-Player GameObject Interacting with Factory InputPort");

				currTargetFactory->Interact(ptrItem);


				return; // end interaction
			}
		}
		else if (currTargetDroppedItem) {
			// Item Exist on Grid
			// 			
			// dynamic cast
			if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(ptrActor)) {
				log(LOG_INFO, "Player Picked up: " + itemType2ItemName[currTargetDroppedItem->item->itemType]);

				// if player 
				Item* pickUp = currTargetDroppedItem->item;
				ptrPlayer->PickUpItem(pickUp);

				currTargetDroppedItem = nullptr;

				return;
			}
		}
		else if (ptrItem != nullptr) {
			// item exists in hand 
			if (currTargetDroppedItem) {
				// if item already exists on Ground
				// 			
				// dynamic cast
				if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(ptrActor)) {
					log(LOG_INFO, "PlayerDropped x and Picked up y");

					// if player 
					Item* pickUp = currTargetDroppedItem->item;
					ptrPlayer->PickUpItem(pickUp);

					gameState.DropItemAt(y, x, ptrItem);

					return;
				}
				else {


					// if not a player, since another item is already occupying space, 
					// try somewhere else
					this->yCoord += 1;
					this->Execute(gameState); // todo, dynamic casting of 'who' is going to be retried isn't that a waste? 

					return;
				}
			}
			if (currTargetDroppedItem==nullptr && currTargetGroundType==GroundType::GRASS) { // todo: Grass -> walkable
				// if no item is on it and ground is walkable 
				log(LOG_INFO, "PlayerDropped x");
				gameState.DropItemAt(y, x, ptrItem);

				return;
			}
			else {
				// go interact somewhere else ;) 
				this->yCoord += 1;
				this->Execute(gameState); // might cause unintended behavior, let's leave it for now

				return;
			}
		}
		else {
			// item doesn't exist do nothing
			log(LOG_INFO, "Player go brrrrr");
		}
	}
}

std::string IGameCommand::GetName() const { return "IGameCommand"; }

void IGameCommand::log(LogLevel level, std::string text) {
	LOG(level, GetName() + "::" + text);
}

void WalkCommand::Execute(GameState& gameState)
{
	 

	RidableObject* walkerGameObject; // to do 
	Direction curDirection = this->direction; 

	uint32_t parentID = walkerGameObject->GetParentID();   

	if (parentID == 0) {
		log(LOG_WARNING, "The object which is trying to walk has no parent, therefore missing a grid. You don't have a space to walk on");  
	}
	else {
		RidableObject* parentObj; // to do 

		Coord2d curWalkerPos = parentObj->GetPosition(walkerID); 

		NavigationInfo curNavigationInfo = parentObj->GetMovementManager()->Move(curWalkerPos, curDirection, curDirection);  

		bool is_occupied = parentObj->CheckIfPositionIsOccupied(curNavigationInfo.pos);  

		if (is_occupied) {
			// well you can't walk there. 
			// to do: Create Interaction Message, and Publish it 
		}
		else {
			// is not occupied 
			
			// just walk. no problem
			parentObj->SwapObjOnGrid(curWalkerPos, curNavigationInfo.pos); 
		}
		
	}

}
