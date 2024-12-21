#include "TerrainManager.h"
#include "TerrainObject.h"
#include "Item.h"


// TerrainManager----------------------------------------------------------------------
TerrainManager::TerrainManager() 
	: groundLoader("E:/repos/[DuckFishing]/model", groundType2GroundName),
	itemLoader("E:/repos/[DuckFishing]/model", itemType2ItemName),
	factoryComponentLoader("E:/repos/[DuckFishing]/model", factoryType2FactoryName) { 

	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) { 
			coord2Transform[i][j] = new Transform();
		}
	}

	// Randomly initialize the ground grid
	RandomizeGroundGrid();
}

void TerrainManager::DropItemAt(int yIndex, int xIndex, Item* item) {
	DroppedItemObject* newDroppedItem = new DroppedItemObject();

	newDroppedItem->SetCoordinates(yIndex, xIndex);
	newDroppedItem->SetItem(item);
}

Item* TerrainManager::GetNewItemGameObject(ItemType itemType) {
	Item* newItem = new Item(itemType);

	// get mesh and texture
	GeneralMesh* ptrMesh = itemLoader.type2Mesh[itemType];
	Texture* ptrTexture = itemLoader.type2Texture[itemType];

	// item itself has default transform 
	Transform* defaultTransform = new Transform();

	newItem->SetMesh(ptrMesh);
	newItem->SetTexture(ptrTexture);
	newItem->SetTransform(defaultTransform);

	return newItem;
}

void TerrainManager::BuildFactoryAt(FactoryType factoryType) {
	FactoryManagerObject* factoryManager = new FactoryManagerObject(factoryType);

	// example 
	FactoryComponentType factoryComponentType = DEFAULT;
	GetNewFactoryComponentObject(factoryType, factoryComponentType, factoryManager);
}

FactoryComponentObject* TerrainManager::GetNewFactoryComponentObject(
	FactoryType factoryType,
	FactoryComponentType factoryComponentType,
	FactoryManagerObject* factoryManager)
{
	FactoryComponentObject* newFactoryComponent = new FactoryComponentObject(factoryComponentType, factoryManager);

	// get mesh and texture 
	std::pair<FactoryType, FactoryComponentType> componentType = { factoryType, factoryComponentType };
	GeneralMesh* ptrMesh = factoryComponentLoader.type2Mesh[componentType];
	Texture* ptrTexture = factoryComponentLoader.type2Texture[componentType];

	Transform* defaultTransfrom = new Transform();

	newFactoryComponent->SetMesh(ptrMesh);
	newFactoryComponent->SetTexture(ptrTexture);
	newFactoryComponent->SetTransform(defaultTransfrom); 

	return newFactoryComponent;
}

void TerrainManager::CreateAndAddDroppedItemAt(int yIndex, int xIndex, Item* item) {
	DroppedItemObject* newDroppedItem = new DroppedItemObject();

	newDroppedItem->SetCoordinates(yIndex, xIndex);
	newDroppedItem->SetItem(item);

	itemGrid[yIndex][xIndex] = newDroppedItem;
}

void TerrainManager::Update() {
	Transform* curTransform;
	
	// ground is manually drawn
	DroppedItemObject* curDroppedItem; 
	FactoryComponentObject* curFactoryComponent;

	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			// Set the transform positions for each block
			float yCoord = i * BLOCK_OFFSET;
			float xCoord = j * BLOCK_OFFSET;
			
			coord2Transform[i][j]->SetScale(glm::vec3(BLOCK_SIZE));
			coord2Transform[i][j]->SetTranslation(glm::vec3(xCoord, 0.0f, yCoord));

			curTransform = coord2Transform[i][j]; 
			curDroppedItem = itemGrid[i][j]; 
			curFactoryComponent = factoryGrid[i][j];

			if (curDroppedItem) {
				curDroppedItem->SetTransform(curTransform);
			}
			if (curFactoryComponent) {
				curFactoryComponent->SetTransform(curTransform);
			}
		}
	}
}

void TerrainManager::DrawGameObject(CameraObject& cameraObj) {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			DrawBlockOfTerrainAt(i, j, cameraObj);
			DrawDroppedItemAt(i, j, cameraObj);
			DrawFactoryComponentAt(i, j, cameraObj);
		}
	}
}

void TerrainManager::DrawBlockOfTerrainAt(int yIndex, int xIndex, CameraObject& cameraObj) {
	// Retrieve transform, ground type, mesh, and texture
	Transform* currTransform = coord2Transform[yIndex][xIndex];
	glm::mat4 transformMat = currTransform->GetTransformMatrix();

	GroundType currGroundType = groundGrid[yIndex][xIndex];
	GeneralMesh* currMesh = groundLoader.type2Mesh[currGroundType];
	Texture* currTexture = groundLoader.type2Texture[currGroundType];

	// Ensure mesh and texture are valid before rendering
	if (currMesh && currTexture) { // TerrainManager directly renders
		currMesh->Render(cameraObj, transformMat, currTexture);
	}
}

void TerrainManager::DrawDroppedItemAt(int yIndex, int xIndex, CameraObject& cameraObj) {
	DroppedItemObject* currItem = itemGrid[yIndex][xIndex];
	if (currItem) {
		Transform* currTransform = coord2Transform[yIndex][xIndex];
		glm::mat4 transformMat = currTransform->GetTransformMatrix();
		currItem->DrawGameObject(cameraObj); 
	}
}

void TerrainManager::DrawFactoryComponentAt(int yIndex, int xIndex, CameraObject& cameraObj) {
	FactoryComponentObject* currFactory = factoryGrid[yIndex][xIndex];
	if (currFactory) {
		Transform* currTransform = coord2Transform[yIndex][xIndex];
		glm::mat4 transformMat = currTransform->GetTransformMatrix();
		currFactory->DrawGameObject(cameraObj);
	}
}

// Function to randomly initialize the groundGrid
void TerrainManager::RandomizeGroundGrid() {
	// Seed the random number generator
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			// Generate a random integer in the range of GroundType
			int randomValue = std::rand() % static_cast<int>(GroundType::LAST);

			// Assign a random GroundType
			groundGrid[i][j] = static_cast<GroundType>(randomValue);
		}
	}
}
