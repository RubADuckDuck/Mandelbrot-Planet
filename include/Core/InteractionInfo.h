#pragma once
#include "PlayerDirection.h"

class GameObject;
class Item; 

struct InteractionInfo {
	GameObject* who;
	Item* item; 
	int yCoord; 
	int xCoord;

	Direction goingWhere = Direction::IDLE;
};