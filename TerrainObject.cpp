#include "TerrainObject.h"

// FactoryManagerObject --------------------------
Recipe* FactoryManagerObject::CheckForMatchingIngredient(std::vector<ItemType>& ingredients) {
	static std::vector<Item2Probability*> emptyResult; // Static empty vector to return in case of no match

	for (int j = 0; j < craftingRecipes.size(); j++) {
		Recipe* currRecipe = craftingRecipes[j];
		for (int i = 0; i < ingredients.size(); i++) {
			if (currRecipe->inputPortIndex2RequiredItem[i] != ingredients[i]) {
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

void FactoryManagerObject::TriggerInteraction(const std::string& msg) {
	try {
		// Parse the message into key-value pairs
		auto keyValues = MessageParser::parse(msg);

		// Extract the target position and used item
		std::pair<int, int> targetPosition = MessageParser::parsePosition(keyValues["position"]);
		ItemType usedItem = MessageParser::parseItem(keyValues["item"]);

		// Check if the target position is valid
		if (Position2PortIndex.find(targetPosition) != Position2PortIndex.end()) {
			int targetPortIndex = Position2PortIndex[targetPosition];

			// Temporarily save the current item at the target port
			ItemType temp = portIndex2Item[targetPortIndex];

			// Add the used item to the target port
			portIndex2Item[targetPortIndex] = usedItem;

			// Reset crafting to evaluate the new configuration
			this->ResetCrafting();
		}
		else {
			LOG(LOG_WARNING, "Invalid position for interaction.");
		}
	}
	catch (const std::exception& e) {
		LOG(LOG_ERROR, e.what());
	}
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