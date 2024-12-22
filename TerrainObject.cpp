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

// DroppedItemObject:: --------------------------
void DroppedItemObject::SetItem(Item* ptrItem) {
	this->item = ptrItem;
}

void DroppedItemObject::Interact(Item* item) {
	item->itemState = ON_GROUND;
	DropItem(item); // drop players Item right here
}

void DroppedItemObject::SetTransform(Transform* transform) {
	ptrTransform = transform;
	item->SetTransform(transform);
}

void DroppedItemObject::DrawGameObject(CameraObject& cameraObj) {
	// droppedItem iteself don't have nothing to draw.
	item->DrawGameObject(cameraObj);
}

// FactoryComponentObject-------------------------
FactoryComponentObject::FactoryComponentObject(FactoryComponentType componentType, FactoryManagerObject* factoryManager) 
	: componentType(componentType), heldItem(nullptr), ptrParentStructure(factoryManager) { }

FactoryComponentObject::~FactoryComponentObject() {}

void FactoryComponentObject::Interact(Item* item) {
	if (componentType == INPUTPORT) {
		// shout that Item is dropped 
		heldItem->itemState = IN_FACTORY;
		this->DropItem(heldItem);

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

void FactoryComponentObject::SetTransform(Transform* transform) {
	ptrTransform = transform;
	heldItem->SetTransform(transform);
}

void FactoryComponentObject::DrawGameObject(CameraObject& cameraObj) {
	GameObject::DrawGameObject(cameraObj); // draw this object
	if (heldItem) {
		heldItem->DrawGameObject(cameraObj); // draw held object
	}
}


void FactoryComponentObject::ResetFactoryFromCrafting() {
	ptrParentStructure->ResetCrafting();
}

// FactoryManagerObject --------------------------
void FactoryManagerObject::InitFactory() {

}

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