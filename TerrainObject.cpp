#include "TerrainObject.h"
#include "Item.h"

// TerrainObject----------------------------------
bool& TerrainObject::CheckCondition(Condition condition) {
	return terrainCondition[condition]; 
}

void TerrainObject::DropItem(Item* item) {
	PublishItemDrop(item);
}

void TerrainObject::PublishItemDrop(Item* item) {
	// to do
}

// FactoryComponentObject-------------------------
FactoryComponentObject::FactoryComponentObject(FactoryComponentType componentType) {
	myType = componentType;
}

FactoryComponentObject::~FactoryComponentObject() {}

void FactoryComponentObject::Interact(Item* item) {
	if (myType == INPUTPORT) {
		// shout that Item is dropped
		PublishItemDrop(heldItem);

		// pick up item
		this->heldItem = item;
	}
	else {
		LOG(LOG_ERROR, "It was this moment, you realize you fu*k up.")
	}
}

Item* FactoryComponentObject::GetHeldItem() {
	return this->heldItem;
}

void FactoryComponentObject::DiscardHeldItem() {
	delete heldItem; 
	heldItem = nullptr;
	return;
}

void FactoryComponentObject::ResetFactoryFromCrafting() {
	ptrParentStructure->ResetCrafting();
}

// FactoryManagerObject --------------------------
Recipe* FactoryManagerObject::CheckForMatchingIngredient(std::vector<Item*>& ingredients) {
	static std::vector<Item2Probability*> emptyResult; // Static empty vector to return in case of no match

	for (int j = 0; j < craftingRecipes.size(); j++) {
		Recipe* currRecipe = craftingRecipes[j];
		ItemType curItemType;

		for (int i = 0; i < ingredients.size(); i++) {
			curItemType = ingredients[i]->itemType;

			if (currRecipe->inputPortIndex2RequiredItem[i] != curItemType) {
				// not the right recipe 
				break;
			}
			else {
				if (i == ingredients.size() - 1) {
					LOG(LOG_INFO, "Found recipe for current configuration");

					return currRecipe;
				}
			}
		}
	}
	LOG(LOG_INFO, "No matching recipe found");
	return nullptr;
}

std::vector<Item*>& FactoryManagerObject::GetCurrentIngredients() {
	std::vector<Item*> currIngredients; 
	Item* curItem;

	for (int i = 0; i < nInputPort; i++) {
		curItem = portIndex2FactoryComponent[i].GetHeldItem();
		currIngredients.push_back(curItem);
	}
	return currIngredients;
}

void FactoryManagerObject::ResetCrafting() {
	// reset duration
	this->isCrafting = nullptr; 
	duration = 0; 
	targetDuration = -1; 

	// get current ingredients 
	std::vector<Item*> currIngredients = this->GetCurrentIngredients(); 

	// check if it is a existing recipe 
	Recipe* targetRecipe = this->CheckForMatchingIngredient(currIngredients);

	if (targetRecipe != nullptr) {
		SetCrafting(targetRecipe);
	}
}

void FactoryManagerObject::SetCrafting(Recipe* currRecipe) {
	this->isCrafting = currRecipe;
	targetDuration = currRecipe->craftingDuration;
}

void FactoryManagerObject::GenerateItemAndReset() {
	// Reset Input Port 
	for (int i = 0; i < this->nInputPort; i++) {
		portIndex2FactoryComponent[i].DiscardHeldItem();
	}

	GenerateItem();

	ResetCrafting();
}

void FactoryManagerObject::GenerateItem() {
	Item2Probability* randomDropper;
	Item* currDrop;

	for (int i = 0 + nInputPort; i < nInputPort + nOutputPort; i++) {
		randomDropper = this->isCrafting->outputPortIndex2ToProductItem[i];
		currDrop = randomDropper->RandomRollDrop();
		portIndex2FactoryComponent[i].DropItem(currDrop); // I hope the object persists after we're out of scope.
	}
	return;
}

void FactoryManagerObject::Update(float deltaTime) {
	if (targetDuration > 0) {
		duration = duration + deltaTime;

		if (duration >= targetDuration) {
			GenerateItemAndReset();
		}
	}
}