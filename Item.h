#pragma once
#include <vector> 
#include <map>
#include "ItemType.h"
#include "GameObject.h"

class GameObject;

class Item : public GameObject {
public:
    ItemType itemType;
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