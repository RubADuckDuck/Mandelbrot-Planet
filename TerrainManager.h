#pragma once
#include "GameObject.h"
#include "GlobalMappings.h"

template <typename T> class Type2MeshAndTexture {
public:
	std::string assetDirectory;

	std::map<T, std::string> type2ObjPath;
	std::map<T, GeneralMesh*> type2Mesh;

	std::map<T, std::string> type2TexturePath;
	std::map<T, Texture*> type2Texture;



	// Constructor takes the asset directory and mapping
	Type2MeshAndTexture(
		const std::string& directory,
		const std::map<T, const std::string>& typeToNameMap
	) : assetDirectory(directory) {
		InitObjPaths(typeToNameMap);
		InitTexturePaths(typeToNameMap);

		SetMeshes();
		SetTextures();
	}

	void InitObjPaths(const std::map<T, const std::string>& typeToNameMap) {
		for (const auto& [type, name] : typeToNameMap) {
			type2ObjPath[type] = assetDirectory + "/" + name + ".obj";
		}
	}

	void InitTexturePaths(const std::map<T,const std::string>& typeToNameMap) {
		for (const auto& [type, name] : typeToNameMap) {
			type2TexturePath[type] = assetDirectory + "/" + name + ".png";
		}
	}

	void SetMeshes() {
		for (const auto& [type, path] : type2ObjPath) {
			type2Mesh[type] = std::make_unique<GeneralMesh>(path);
		}
	}

	void SetTextures() {
		for (const auto& [type, path] : type2TexturePath) {
			type2Texture[type] = std::make_unique<Texture>(path);
		}
	}
};

class TerrainManager : public GameObject { // Manages Interaction between Player2Terrain Terrain2Terrain Interaction
public:
    static const int GRID_SIZE = 100;
    float BLOCK_SIZE = 0.5f;
    float BLOCK_OFFSET = 1.0f;

    Transform coord2Transform[GRID_SIZE][GRID_SIZE]; // 2d coordinate to transfrom
	// every gm on (y,x) is transformed by coord2Transform[y, x]

    GroundType groundGrid[GRID_SIZE][GRID_SIZE];
    DroppedItemObject* itemGrid[GRID_SIZE][GRID_SIZE];
    FactoryComponentObject* factoryGrid[GRID_SIZE][GRID_SIZE];

    Type2MeshAndTexture<GroundType> groundLoader;
    Type2MeshAndTexture<ItemType> itemLoader;
    Type2MeshAndTexture<std::pair<FactoryType, FactoryComponentType>> factoryComponentLoader; // this could be a mistake. I could have been more general.

    TerrainManager();

	void DropItemAt(int yIndex, int xIndex, Item* item) {
		DroppedItemObject* newDroppedItem = new DroppedItemObject(); 

		newDroppedItem->SetCoordinates(yIndex, xIndex); 
		newDroppedItem->SetItem(item); 
	}

	Item* GetNewItemGameObject(ItemType itemType) {
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

	void BuildFactoryAt(FactoryType factoryType) {
		FactoryManagerObject* factoryManager = new FactoryManagerObject(factoryType); 

		// example 
		FactoryComponentType factoryComponentType; 
		GetNewFactoryComponentObject(factoryType, factoryComponentType, factoryManager);
	}

	FactoryComponentObject* GetNewFactoryComponentObject(
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
	}

	void CreateAndAddDroppedItemAt(int yIndex, int xIndex, Item* item) {
		DroppedItemObject* newDroppedItem = new DroppedItemObject(); 

		newDroppedItem->SetCoordinates(yIndex, xIndex);
		newDroppedItem->SetItem(item); 

		itemGrid[yIndex][xIndex] = newDroppedItem;
	} 



    void DrawGameObject(CameraObject& cameraObj) override;

    void DrawBlockOfTerrainAt(int yIndex, int xIndex, CameraObject& cameraObj); 

	void DrawDroppedItemAt(int yIndex, int xIndex, CameraObject& cameraObj);

	void DrawFactoryComponentAt(int yIndex, int xIndex, CameraObject& cameraObj);

    // Function to randomly initialize the groundGrid
    void RandomizeGroundGrid();
};