#include "TerrainManager.h"
#include "TerrainObject.h"
#include "Item.h"
#include "GameEngine.h"
#include "GameObject.h"

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
}

void TerrainManager::onEvent(InteractionInfo* interactionInfo) {
	GameObject::onEvent(interactionInfo); 
	this->HandleInteraction(interactionInfo);
}

void TerrainManager::HandleInteraction(InteractionInfo* interactionInfo) {
	GameObject* who = interactionInfo->who;
	Item* item = interactionInfo->item;
	int y = interactionInfo->yCoord;
	int x = interactionInfo->xCoord;

	bool isPlayer = false;
	int targetY = 0; 
	int targetX = 0;

	if (PlayableObject* ptrPlayer = dynamic_cast<PlayableObject*>(who)) {
		isPlayer = true; 

		// if player 
		targetY = ptrPlayer->targetY;
		targetX = ptrPlayer->targetX; 
	}

	if (isPlayer && factoryGrid[targetY][targetX]!=nullptr) {
		// if factory exist in target
		LOG(LOG_INFO, "TerrainManager::Attempt to interact with factory"); 
		if (factoryGrid[targetY][targetX]->componentType == INPUTPORT) {
			// if factory component is input port 
			LOG(LOG_INFO, "TerrainManager::Interact with inputPort of factory");
			factoryGrid[targetY][targetX]->Interact(item);

			return; // end interaction
		}
		else {
			// interaction not satisfied
		}
	}
	if (itemGrid[y][x]) {
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
		// item exists
		if (itemGrid[y][x]) {
			// if item already exists 
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
