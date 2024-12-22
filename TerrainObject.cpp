#include "TerrainObject.h"
#include "Item.h"
#include <queue>

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
    if (heldItem) {
        heldItem->SetTransform(transform);
    }
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

// We'll use BFS for the region growth process.
void FactoryManagerObject::RandomlyGenerateBuildingShape()
{
    // 1) Fill everything with EMPTY
    for (int r = 0; r < MAX_STRUCTURE_LENGTH; r++) {
        for (int c = 0; c < MAX_STRUCTURE_LENGTH; c++) {
            buildingShape[r][c] = FactoryComponentType::EMPTY;
        }
    }
    portIndex2PortPosition.clear();
    position2PortIndex.clear();

    // Seed random generator if not done elsewhere
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }

    // 2) Pick a random start cell
    int startR = std::rand() % MAX_STRUCTURE_LENGTH;
    int startC = std::rand() % MAX_STRUCTURE_LENGTH;

    // Mark this cell as part of the building (DEFAULT)
    buildingShape[startR][startC] = FactoryComponentType::DEFAULT;

    // 3) Use BFS (or DFS) to expand the shape randomly
    std::queue< std::pair<int, int> > frontier;
    frontier.push(std::make_pair(startR, startC));

    // Decide how large our random shape should be
    int minCells = 5;                 // min size (customize as needed)
    int maxCells = 20;                // max size (customize as needed)
    int targetSize = (std::rand() % (maxCells - minCells + 1)) + minCells;

    int currentSize = 1;

    // Directions for adjacency (up/down/left/right)
    std::vector< std::pair<int, int> > directions;
    directions.push_back(std::make_pair(1, 0));
    directions.push_back(std::make_pair(-1, 0));
    directions.push_back(std::make_pair(0, 1));
    directions.push_back(std::make_pair(0, -1));

    while (!frontier.empty() && currentSize < targetSize)
    {
        // Extract (r, c) from the queue
        std::pair<int, int> cell = frontier.front();
        frontier.pop();

        int r = cell.first;
        int c = cell.second;

        // Shuffle the directions for randomness
        std::random_shuffle(directions.begin(), directions.end());

        // Try random neighbors
        for (size_t i = 0; i < directions.size(); i++) {
            int dr = directions[i].first;
            int dc = directions[i].second;

            int nr = r + dr;
            int nc = c + dc;

            // Bounds check
            if (nr < 0 || nr >= MAX_STRUCTURE_LENGTH ||
                nc < 0 || nc >= MAX_STRUCTURE_LENGTH) {
                continue;
            }

            // If neighbor is EMPTY, we can convert it to DEFAULT sometimes
            if (buildingShape[nr][nc] == FactoryComponentType::EMPTY) {
                // 50% chance to expand into this neighbor (customize this probability)
                if (std::rand() % 100 < 50) {
                    buildingShape[nr][nc] = FactoryComponentType::DEFAULT;
                    frontier.push(std::make_pair(nr, nc));
                    currentSize++;

                    // Stop if we reached target size
                    if (currentSize >= targetSize) {
                        break;
                    }
                }
            }
        }

        // Double-check if we reached the target size inside the loop
        if (currentSize >= targetSize) {
            break;
        }
    }

    // 4) Now we have a random connected shape of DEFAULT cells.
    //    Next, we place input/output ports on boundary cells 
    //    (i.e., DEFAULT cells that have an adjacent EMPTY or out-of-bounds).

    std::vector< std::pair<int, int> > boundaryCells;

    // Directions array is the same as above
    for (int r = 0; r < MAX_STRUCTURE_LENGTH; r++) {
        for (int c = 0; c < MAX_STRUCTURE_LENGTH; c++) {
            if (buildingShape[r][c] == FactoryComponentType::DEFAULT) {
                bool isBoundary = false;

                // Check the four neighbors
                for (size_t i = 0; i < directions.size(); i++) {
                    int nr = r + directions[i].first;
                    int nc = c + directions[i].second;

                    if (nr < 0 || nr >= MAX_STRUCTURE_LENGTH ||
                        nc < 0 || nc >= MAX_STRUCTURE_LENGTH ||
                        buildingShape[nr][nc] == FactoryComponentType::EMPTY)
                    {
                        isBoundary = true;
                        break;
                    }
                }

                if (isBoundary) {
                    boundaryCells.push_back(std::make_pair(r, c));
                }
            }
        }
    }

    // 5) Place input ports
    int placedInput = 0;
    while (placedInput < nInputPort && !boundaryCells.empty()) {
        int index = std::rand() % boundaryCells.size();
        int br = boundaryCells[index].first;
        int bc = boundaryCells[index].second;

        buildingShape[br][bc] = FactoryComponentType::INPUTPORT;
        portIndex2PortPosition.push_back(std::make_pair(br, bc));
        position2PortIndex[std::make_pair(br, bc)] =
            static_cast<int>(portIndex2PortPosition.size()) - 1;

        placedInput++;

        // Remove it so we don't pick the same cell again
        boundaryCells.erase(boundaryCells.begin() + index);
    }

    // 6) Place output ports
    int placedOutput = 0;
    while (placedOutput < nOutputPort && !boundaryCells.empty()) {
        int index = std::rand() % boundaryCells.size();
        int br = boundaryCells[index].first;
        int bc = boundaryCells[index].second;

        buildingShape[br][bc] = FactoryComponentType::OUTPUTPORT;
        portIndex2PortPosition.push_back(std::make_pair(br, bc));
        position2PortIndex[std::make_pair(br, bc)] =
            static_cast<int>(portIndex2PortPosition.size()) - 1;

        placedOutput++;

        // Remove it so we don't pick the same cell again
        boundaryCells.erase(boundaryCells.begin() + index);
    }

    // All remaining DEFAULT cells form the interior of the shape
    // while outside is EMPTY.
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
		curItem = portIndex2FactoryComponent[i]->GetHeldItem();
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
		portIndex2FactoryComponent[i]->DiscardHeldItem();
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
		portIndex2FactoryComponent[i]->DropItem(currDrop); // I hope the object persists after we're out of scope.
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