#pragma once
#include <vector> 
#include <map>
#include "ItemType.h"
#include "GameObject.h"

class GameObject;

enum ItemState {
	ON_GROUND, 
	HELD_BY_PLAYER, 
	IN_FACTORY
};

class Item : public GameObject {
public:
    ItemType itemType;
	ItemState itemState; 

	void DrawGameObject(CameraObject& cameraObj) override; // differentiate draw method based on ItemState
};

struct Item2Probability {
    std::map<Item*, float> item2ProbMap;

	Item* RandomRollDrop();
};

struct Recipe {
	std::vector<ItemType> inputPortIndex2RequiredItem;
	std::vector<Item2Probability*> outputPortIndex2ToProductItem;
	float craftingDuration;
};