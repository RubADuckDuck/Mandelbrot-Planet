#pragma once
#include <vector> 
#include <map>
#include "ItemType.h"
#include "RidableObject.h"

enum ItemState {
	ON_GROUND, 
	HELD_BY_PLAYER, 
	IN_FACTORY, 
	INVALID_STATE
};

class Item : public RidableObject {
public:
    ItemType itemType = ItemType::INVALID;
	ItemState itemState; 

	uint8_t GetTypeID() {
		return static_cast<uint8_t>(ObjectType::ITEM_OBJECT);
	}

	Item(ItemType itemType) : itemType(itemType), itemState(INVALID_STATE){}

	// void DrawGameObject(CameraObject& cameraObj) override; // differentiate draw method based on ItemState
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