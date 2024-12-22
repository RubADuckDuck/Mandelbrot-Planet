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
		for (const auto& pair : typeToNameMap) {
			const T& type = pair.first;
			const std::string& name = pair.second;

			type2ObjPath[type] = assetDirectory + "/" + name + ".obj";
		}
	}


	void InitTexturePaths(const std::map<T, const std::string>& typeToNameMap) {
		for (const auto& pair : typeToNameMap) {
			const T& type = pair.first;
			const std::string& name = pair.second;

			type2TexturePath[type] = assetDirectory + "/texture/" + name + ".png";
		}
	}


	void SetMeshes() {
		GeneralMesh* currMesh;
		for (const auto& pair : type2ObjPath) {
			const auto& type = pair.first;
			const auto& path = pair.second;

			currMesh = new StaticMesh();
			currMesh->LoadMesh(path);
			type2Mesh[type] = currMesh;
		}
	}

	void SetTextures() { 
		Texture* currTexture;
		for (const auto& pair : type2TexturePath) {
			const auto& type = pair.first;
			const auto& path = pair.second;

			currTexture = new Texture;
			currTexture->LoadandSetTextureIndexFromPath(path);
			type2Texture[type] = currTexture;
		}
	}
};

class DroppedItemObject; 
class FactoryComponentObject;
class Item; 
class FactoryManagerObject;
class GameEngine;

class TerrainManager : public GameObject { // Manages Interaction between Player2Terrain Terrain2Terrain Interaction
public:
    static const int GRID_SIZE = 100;
    float BLOCK_SIZE = 0.5f;
    float BLOCK_OFFSET = 1.0f;

	GameEngine* gameEngine; 

    Transform* coord2Transform[GRID_SIZE][GRID_SIZE]; // 2d coordinate to transfrom
	// every gm on (y,x) is transformed by coord2Transform[y, x]

    GroundType groundGrid[GRID_SIZE][GRID_SIZE];
    DroppedItemObject* itemGrid[GRID_SIZE][GRID_SIZE];
    FactoryComponentObject* factoryGrid[GRID_SIZE][GRID_SIZE];

    Type2MeshAndTexture<GroundType> groundLoader;
    Type2MeshAndTexture<ItemType> itemLoader;
    Type2MeshAndTexture<std::pair<FactoryType, FactoryComponentType>> factoryComponentLoader; // this could be a mistake. I could have been more general. 

	std::vector<PlayableObject*> players;

    TerrainManager();

	void SetGameEngine(GameEngine* ge) {
		gameEngine = ge;
	}

	void onEvent(GameObject* who, Item* item, int y, int x) override;

	void HandleInteraction(GameObject* who, Item* item, int y, int x);

	void CreateAndAddPlayer(
		const std::string& meshPath,
		std::string& texturePath
	);

	void AddPlayer(PlayableObject* newPlayer);

	void DropItemAt(int yIndex, int xIndex, Item* item);

	Item* GetNewItemGameObject(ItemType itemType);

	void BuildFactoryAt(FactoryType factoryType);

	FactoryComponentObject* GetNewFactoryComponentObject(
		FactoryType factoryType,
		FactoryComponentType factoryComponentType,
		FactoryManagerObject* factoryManager);

	void CreateAndAddDroppedItemAt(int yIndex, int xIndex, Item* item);

	void Update() override;

    void DrawGameObject(CameraObject& cameraObj) override; 

	void DrawPlayers(CameraObject& cameraObj);

    void DrawBlockOfTerrainAt(int yIndex, int xIndex, CameraObject& cameraObj); 

	void DrawDroppedItemAt(int yIndex, int xIndex, CameraObject& cameraObj);

	void DrawFactoryComponentAt(int yIndex, int xIndex, CameraObject& cameraObj);

    // Function to randomly initialize the groundGrid
    void RandomizeGroundGrid();
};