#include "TerrainManager.h"
#include "TerrainObject.h"
#include "Item.h"
#include "GameEngine.h"
#include "GameObject.h"
#include <cmath>

// TerrainManager::----------------------------------------------------------------------
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
	movementManager = MovementManager();
}

void TerrainManager::onEvent(InteractionInfo* interactionInfo) {
	GameObject::onEvent(interactionInfo); 
	this->HandleInteraction(interactionInfo);
}

void directionActOnCoord(Direction direction, int& yCoord, int& xCoord, int gridSizeY, int gridSizeX) {
	// Assume the object has xCoord and yCoord representing its position
	switch (direction) {
	case Direction::UP:
		yCoord--; // Move up (decrease y)
		if (yCoord < 0) {
			yCoord = gridSizeY - 1;
		}
		break;
	case Direction::DOWN:
		yCoord++; // Move down (increase y)
		if (yCoord >= gridSizeY) {
			yCoord = 0; 
		}
		break;
	case Direction::LEFT:
		xCoord--; // Move left (decrease x)
		if (xCoord < 0) {
			xCoord = gridSizeX - 1;
		}
		break;
	case Direction::RIGHT:
		xCoord++; // Move right (increase x)
		if (xCoord >= gridSizeX) {
			xCoord = 0;
		}
		break;
	case Direction::IDLE:
		// Do nothing, the object remains in place
		break;
	default:
		// Handle unexpected direction (optional)
		break;
	}
}

void TerrainManager::HandleInteraction(InteractionInfo* interactionInfo) {
	GameObject* who = interactionInfo->who;
	Item* item = interactionInfo->item;
	int y = interactionInfo->yCoord;
	int x = interactionInfo->xCoord;
	Direction curDirection = interactionInfo->goingWhere;

	bool isPlayer = false;
	int targetY = y; 
	int targetX = x; 

	// directionActOnCoord(curDirection, targetY, targetX, this->GRID_SIZE, this->GRID_SIZE);
	NavigationInfo curCoord2dandDir = movementManager.Move({ y, x }, curDirection, curDirection);
	targetY = curCoord2dandDir.pos.first; 
	targetX = curCoord2dandDir.pos.second; 
	

	if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(who)) {
		isPlayer = true; 
		ptrPlayer->direction = curCoord2dandDir.direction; 
		ptrPlayer->orientation -= curCoord2dandDir.changeOfOrientation;
		ptrPlayer->orientation = PositiveModulo(ptrPlayer->orientation, 4);
	}

	if (curDirection != Direction::IDLE) {
		// Somebody Requested Walk 
		if (factoryGrid[targetY][targetX] != nullptr) {
			if (isPlayer) {
				// throw current held Item to target

				// if factory exist in target
				LOG(LOG_INFO, "TerrainManager::Attempt to interact with factory");
				if (factoryGrid[targetY][targetX]->componentType == INPUTPORT) {
					// if factory component is input port 
					LOG(LOG_INFO, "TerrainManager::Interact with inputPort of factory");
					factoryGrid[targetY][targetX]->Interact(item);
					
					// drop players item
					PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(who); 
					ptrPlayer->heldItem = nullptr;

					return; // end interaction
				}
				else {
					// you can do nothing 
					return;
				}
			}
			else {
				// you can't walk on factories

				return;
			}

		}
		else if (groundGrid[targetY][targetX] == GroundType::WATER) {
			// throw in water? fishing?
			if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(who)) {
				// if ptrPlayer->heldItem->itemType == FISHING_ROD // todo
			}
			else {
				// you can't walk on water. Or can you?
			} 

			return;
		}
		else {
			// No water no factory
			// you can walk now. 
			if (GameObjectOnGrid* gameobjectOnGrid= dynamic_cast<GameObjectOnGrid*>(who)) {
				gameobjectOnGrid->SetCoordinates(targetY, targetX);
			}
			return;
		}
		return;
	}
	else { 
		// is IDLE 
		if (factoryGrid[targetY][targetX] != nullptr && factoryGrid[targetY][targetX]->componentType == INPUTPORT) {
			// the gameobject is facing the input port
			if (isPlayer) {
				// players can interact with factory by attempting to walk on it 
				// don't end interaction player might one to grab something infront of the InputPort
			}
			else {
				// if factory component is input port 
				LOG(LOG_INFO, "TerrainManager::Non-Player GameObject Interacting with Factory InputPort");
				factoryGrid[targetY][targetX]->Interact(item);

				return; // end interaction
			}
		}
		else if (itemGrid[y][x]) {
			// Item Exist on Grid
			// 			
			// dynamic cast
			if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(who)) {
				LOG(LOG_INFO, "TerrainManager::Player Picked up: " + itemType2ItemName[itemGrid[y][x]->item->itemType]);

				// if player 
				Item* pickUp = itemGrid[y][x]->item;
				ptrPlayer->PickUpItem(pickUp);

				itemGrid[y][x] = nullptr;

				return;
			}
		}
		else if (item) {
			// item exists in hand
			if (itemGrid[y][x]) {
				// if item already exists on Ground
				// 			
				// dynamic cast
				if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(who)) {
					LOG(LOG_INFO, "TerrainManager::PlayerDropped x and Picked up y");

					// if player 
					Item* pickUp = itemGrid[y][x]->item;
					ptrPlayer->PickUpItem(pickUp);

					this->DropItemAt(y, x, item);

					return;
				}
				else {
					// if not a player, since another item is already occupying space, 
					// try somewhere else
					interactionInfo->yCoord += 1;
					this->HandleInteraction(interactionInfo); // todo, dynamic casting of 'who' is going to be retried isn't that a waste? 

					return;
				}
			}
			if (!itemGrid[y][x] && groundGrid[y][x] == GroundType::GRASS) { // todo: Grass -> walkable
				// if no item is on it and ground is walkable 
				LOG(LOG_INFO, "TerrainManager::PlayerDropped x");
				this->DropItemAt(y, x, item);

				return;
			}
			else {
				// go interact somewhere else ;) 
				interactionInfo->yCoord += 1;
				this->HandleInteraction(interactionInfo); // might cause unintended behavior, let's leave it for now

				return;
			}
		}
		else {
			// item doesn't exist do nothing
			LOG(LOG_INFO, "TerrainManager::Player go brrrrr");
		}
	}
}

void TerrainManager::CreateAndAddPlayer(
	const std::string& meshPath,
	std::string& texturePath
)
{
	GeneralMesh* curMesh = new StaticMesh();
	curMesh->LoadMesh(meshPath);

	Texture* curTexture = new Texture();
	curTexture->LoadandSetTextureIndexFromPath(texturePath);

	Transform* defaultTransform = new Transform();
	defaultTransform->SetScale(glm::vec3(0.2f));
	defaultTransform->SetTranslation(glm::vec3(0, 1.0f, 0));

	PlayableObject* gameObject = new PlayableObject();
	gameObject->SetMesh(curMesh);
	gameObject->SetTexture(curTexture);
	gameObject->SetTransform(defaultTransform);

	this->AddPlayer(gameObject);
}

void TerrainManager::AddPlayer(PlayableObject* newPlayer) {
	this->gameEngine->ManuallySubscribe(newPlayer);

	players.push_back(newPlayer);
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

void TerrainManager::BuildFactoryAt(FactoryType factoryType, int yCoord, int xCoord, int nInput, int nOutput) {
	FactoryManagerObject* factoryManager = new FactoryManagerObject(factoryType, nInput, nOutput);

	FactoryComponentType curComponentType;
	FactoryComponentObject* curFactoryComponent; 

	int curY; int curX;

	// iterate through building 
	for (int i = 0; i < MAX_STRUCTURE_LENGTH; i++) {
		for (int j = 0; j < MAX_STRUCTURE_LENGTH; j++) {
			curComponentType = factoryManager->buildingShape[i][j];  

			if (curComponentType != EMPTY) { 
				curY = yCoord + i; curX = xCoord + j; 

				// make factory component
				curFactoryComponent = GetNewFactoryComponentObject(factoryType, curComponentType, factoryManager);

				// set coord 
				curFactoryComponent->SetCoordinates(curY, curX); 
				curFactoryComponent->SetLocalCoord(i, j); 
				curFactoryComponent->SetOffset(yCoord, xCoord);

				this->factoryGrid[curY][curX] = curFactoryComponent;
			}
		}
	}
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

void TerrainManager::DropItemAt(int yIndex, int xIndex, Item* item) {
	DroppedItemObject* newDroppedItem = new DroppedItemObject();

	newDroppedItem->SetCoordinates(yIndex, xIndex);
	newDroppedItem->SetItem(item);

	itemGrid[yIndex][xIndex] = newDroppedItem;
}

void TerrainManager::CreateAndAddDroppedItemAt(int yIndex, int xIndex, ItemType itemType) {
	Item* newItem = new Item(itemType);  

	// load mesh and texture
	GeneralMesh* ptrMesh = itemLoader.type2Mesh[itemType];
	Texture* ptrTexture = itemLoader.type2Texture[itemType]; 
	Transform* defaultTransform = new Transform();

	newItem->SetMesh(ptrMesh); 
	newItem->SetTexture(ptrTexture); 
	newItem->SetTransform(defaultTransform);

	DroppedItemObject* newDroppedItem = new DroppedItemObject();

	newDroppedItem->SetCoordinates(yIndex, xIndex);
	newDroppedItem->SetItem(newItem);

	itemGrid[yIndex][xIndex] = newDroppedItem;
}

void TerrainManager::Update() {
	Transform* curTransform;
	
	// ground is manually drawn
	DroppedItemObject* curDroppedItem; 
	FactoryComponentObject* curFactoryComponent;

	float radius1 = 1; 
	float degreePerStep1 = 360 / GRID_SIZE; 

	float curDegree1 = 0; 
	float curRadian1 = 0;

	float radius2 = 4; 
	float degreePerStep2 = 360 / GRID_SIZE;

	float curDegree2 = 0; 
	float curRadian2 = 0; 

	bool normalMode = true;
	bool torus = false;

	for (int i = 0; i < GRID_SIZE; i++) {
		curDegree1 += degreePerStep1; 
		curRadian1 = glm::radians(curDegree1);
		for (int j = 0; j < GRID_SIZE; j++) {
			curDegree2 -= degreePerStep2; 
			curRadian2 = glm::radians(curDegree2);
			if (normalMode) {
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
			else if(!torus) {
				// Set the transform positions for each block
				
				float zCoord = radius1 * cos(curRadian1);
				float yCoord = radius1 * sin(curRadian1);
				float xCoord = j * BLOCK_OFFSET;


				coord2Transform[i][j]->SetScale(glm::vec3(BLOCK_SIZE));
				coord2Transform[i][j]->SetTranslation(glm::vec3(xCoord, zCoord, yCoord));
				coord2Transform[i][j]->SetRotation(glm::radians(curDegree1), glm::vec3(1, 0, 0));

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
			else {
				// Set the transform positions for each block

				float zCoord = radius1 * cos(curRadian1); // leave the z 
				float yCoord = sin(curRadian2) * (radius2 + radius1 * sin(curRadian1));
				float xCoord = cos(curRadian2) * (radius2 + radius1 * sin(curRadian1));


				coord2Transform[i][j]->SetScale(glm::vec3(BLOCK_SIZE));
				coord2Transform[i][j]->SetTranslation(glm::vec3(xCoord, zCoord, yCoord));
				coord2Transform[i][j]->SetRotation(glm::radians( - curDegree1), glm::vec3(0, 0, 1));
				coord2Transform[i][j]->AddRotation(glm::radians( - curDegree2), glm::vec3(0, 1, 0));

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

	PlayableObject* curPlayer;

	// Update player position 
	for (int i = 0; i < players.size(); i++) {
		curPlayer = players[i]; 
		int yIndex = curPlayer->yCoord; 
		int xIndex = curPlayer->xCoord; 

		curTransform = coord2Transform[yIndex][xIndex]; 

		// Rotate the transform based on the player's facing direction
		glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f); // Assuming Y-axis for rotation
		float rotationAngle = 0.0f;

		switch (curPlayer->direction) {
		case Direction::UP:
			rotationAngle = glm::radians(180.0f); // No rotation needed
			break;
		case Direction::DOWN:
			rotationAngle = glm::radians(0.0f); // Rotate 180 degrees
			break;
		case Direction::LEFT:
			rotationAngle = glm::radians(-90.0f); // Rotate 90 degrees counterclockwise
			break;
		case Direction::RIGHT:
			rotationAngle = glm::radians(90.0f); // Rotate 90 degrees clockwise
			break;
		}
		curTransform->SetRotation(rotationAngle, rotationAxis);

		curPlayer->SetTransform(curTransform);
		
		if (curPlayer->heldItem) {
			curPlayer->heldItem->SetTransform(curTransform);
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

	DrawPlayers(cameraObj);
}

void TerrainManager::DrawPlayers(CameraObject& cameraObj) {
	for (int i = 0; i < players.size(); i++) {
		players[i]->DrawGameObject(cameraObj);
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

void TerrainManager::RandomizeGroundGrid() {
	// Seed the random number generator
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	// Step 1: Initialize the grid with weighted probabilities
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			// Weighted randomization: 70% GRASS, 30% WATER
			int randomValue = std::rand() % 100; // Random value between 0-99
			if (randomValue < 70) {
				groundGrid[i][j] = GroundType::GRASS;
			}
			else {
				groundGrid[i][j] = GroundType::WATER;
			}
		}
	}

	// Step 2: Smooth the grid to create clusters
	for (int iteration = 0; iteration < 3; iteration++) { // Repeat smoothing for better results
		SmoothGroundGrid();
	}
}

void TerrainManager::SmoothGroundGrid() {
	GroundType tempGrid[GRID_SIZE][GRID_SIZE];

	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			int grassCount = 0;
			int waterCount = 0;

			// Count neighbors
			for (int di = -1; di <= 1; di++) {
				for (int dj = -1; dj <= 1; dj++) {
					if (di == 0 && dj == 0) continue; // Skip the current cell

					int ni = i + di;
					int nj = j + dj;

					// Check bounds
					if (ni >= 0 && ni < GRID_SIZE && nj >= 0 && nj < GRID_SIZE) {
						if (groundGrid[ni][nj] == GroundType::GRASS) {
							grassCount++;
						}
						else if (groundGrid[ni][nj] == GroundType::WATER) {
							waterCount++;
						}
					}
				}
			}

			// Smooth based on neighbor counts
			if (grassCount > waterCount) {
				tempGrid[i][j] = GroundType::GRASS;
			}
			else {
				tempGrid[i][j] = GroundType::WATER;
			}
		}
	}

	// Copy tempGrid back to groundGrid
	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE; j++) {
			groundGrid[i][j] = tempGrid[i][j];
		}
	}
}
