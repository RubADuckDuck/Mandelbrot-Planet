#pragma once
#include "GameObject.h"
#include "GlobalMappings.h"

template <typename T> class Type2MeshAndTexture {
public:
	std::string assetDirectory;

	std::map<T, std::string> type2ObjPath;
	std::map<T, std::unique_ptr<GeneralMesh>> type2Mesh;

	std::map<T, std::string> type2TexturePath;
	std::map<T, std::unique_ptr<Texture>> type2Texture;



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
    Type2MeshAndTexture<std::pair<FactoryType, FactoryComponentType>> factoryComponentLoader;

    TerrainManager();

    void Update() override;

    void DrawGameObject(CameraObject& cameraObj) override;

    void DrawBlockOfTerrainAt(int yIndex, int xIndex, CameraObject& cameraObj); 

	void DrawDroppedItemAt(int yIndex, int xIndex, CameraObject& cameraObj);

	void DrawFactoryComponentAt(int yIndex, int xIndex, CameraObject& cameraObj);

    // Function to randomly initialize the groundGrid
    void RandomizeGroundGrid();
};