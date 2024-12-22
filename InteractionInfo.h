#pragma once

class GameObject;
class Item; 

struct InteractionInfo {
	GameObject* who;
	Item* item; 
	int yCoord; 
	int xCoord;
};