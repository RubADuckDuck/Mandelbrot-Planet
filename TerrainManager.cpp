#include "TerrainManager.h"





// TerrainManager----------------------------------------------------------------------
TerrainManager::TerrainManager() 
	: groundLoader("E:/repos/[DuckFishing]/model/", groundType2GroundName),
	itemLoader("E:/repos/[DuckFishing]/model/", itemType2ItemName),
	factoryComponentLoader("E:/repos/[DuckFishing]/model/", factoryType2FactoryName) { 

	// Randomly initialize the ground grid
	RandomizeGroundGrid();
}


void TerrainManager::Update() {
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			// Set the transform positions for each block
			float xCoord = j * BLOCK_OFFSET;
			float yCoord = i * BLOCK_OFFSET;
			coord2Transform[i][j].SetScale(glm::vec3(BLOCK_SIZE));
			coord2Transform[i][j].SetTranslation(glm::vec3(xCoord, 0.0f, yCoord));
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
	Transform& currTransform = coord2Transform[yIndex][xIndex];
	glm::mat4 transformMat = currTransform.GetTransformMatrix();

	GroundType currGroundType = groundGrid[yIndex][xIndex];
	GeneralMesh* currMesh = groundLoader.type2Mesh[currGroundType].get();
	Texture* currTexture = groundLoader.type2Texture[currGroundType].get();

	// Ensure mesh and texture are valid before rendering
	if (currMesh && currTexture) {
		currMesh->Render(cameraObj, transformMat, currTexture);
	}
}

void TerrainManager::DrawDroppedItemAt(int yIndex, int xIndex, CameraObject& cameraObj) {
	DroppedItemObject* currItem = itemGrid[yIndex][xIndex];
	if (currItem) {
		Transform& currTransform = coord2Transform[yIndex][xIndex];
		glm::mat4 transformMat = currTransform.GetTransformMatrix();
		currItem->DrawGameObject(cameraObj, transformMat); 
	}
}

void TerrainManager::DrawFactoryComponentAt(int yIndex, int xIndex, CameraObject& cameraObj) {
	FactoryComponentObject* currFactory = factoryGrid[yIndex][xIndex];
	if (currFactory) {
		Transform& currTransform = coord2Transform[yIndex][xIndex];
		glm::mat4 transformMat = currTransform.GetTransformMatrix();
		currFactory->DrawGameObject(cameraObj, transformMat);
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
