#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility> // Required for std::pair

#include "Mesh.h"
#include "utils.h"
#include "LOG.h"
#include "MessageParser.h"
#include "ItemType.h"
#include "GlobalMappings.h"
#include "FactoryType.h"
#include "GroundType.h"

//class Animation; 

class Texture; 

class Transform; 

class CameraObject;


using Publisher = std::function<void(const std::string&)>; // using 'Alias' = std::function<'returnType'('argType')>



class GameObject {
public: 
	GeneralMesh* ptrModel;
	Texture* ptrTexture;  
	//Animation* ptrAnimation;  
	Transform* ptrTransform;  
    
    Publisher* singletonPublisher;

	virtual ~GameObject();

	void SetMesh(GeneralMesh* ptrModel);
	void SetTexture(Texture* ptrTexture);
	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
	void SetTransform(Transform* ptrTransform);

	virtual void Update();
	virtual glm::mat4 GetModelMatrixFromTransform();
	virtual void DrawGameObject(CameraObject& cameraObj); 
    virtual void onEvent(const std::string& message);
}; 

class PlayableObject : public GameObject {
    void onEvent(const std::string& message) override;
}; 

enum class MapLayerType {
    BIOME,        // Water, Grassland, volcano ...
    STRUCTURE, // buildings, Primary Resources (one time use blocks)
    DROPPEDITEM,  // Resources
    PLAYER,
    LAST
};



//struct ReusableAsset {
//    int id;
//    std::string name;
//    std::string texturePath;
//    std::string objPath;
//
//    Texture texture;
//    StaticMesh mesh;
//
//    bool walkable;
//};
//
//
//struct TerrainType : ReusableAsset {
//    
//};
//
//struct StructureType {
//    
//};

//enum class StructureType {
//    TREE, 
//    ROCK, 
//    WHEATFIELD,
//    BAKERY, 
//    TOOLFACTORY,
//    WALL,
//    LAST
//};


#define MAX_STRUCTURE_LENGTH 5


// todo: move this somewhere else
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
        const std::map<T, std::string>& typeToNameMap
    ) : assetDirectory(directory) {
        InitObjPaths(typeToNameMap);
        InitTexturePaths(typeToNameMap);

        SetMeshes();
        SetTextures();
    }

    void InitObjPaths(const std::map<T, std::string>& typeToNameMap) {
        for (const auto& [type, name] : typeToNameMap) {
            type2ObjPath[type] = assetDirectory + "/" + name + ".obj";
        }
    }

    void InitTexturePaths(const std::map<T, std::string>& typeToNameMap) {
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

    Transform index2GroundTransform[GRID_SIZE][GRID_SIZE];

    GroundType groundGrid[GRID_SIZE][GRID_SIZE];
    DroppedItemObject* itemGrid[GRID_SIZE][GRID_SIZE]; 
    FactoryComponentObject* factoryGrid[GRID_SIZE][GRID_SIZE];

    std::map<GroundType, std::string> groundType2ObjPath; 
    std::map<GroundType, GeneralMesh*> groundType2Mesh;    // Use pointers to GeneralMesh 

    std::map<GroundType, std::string> groundType2TexturePath;
    std::map<GroundType, Texture*> groundType2Texture;     // Use pointers to Texture
    
    

    TerrainManager();

    void Update() override;

    void DrawGameObject(CameraObject& cameraObj) override;

    void DrawBlockOfTerrainAt(int yIndex, int xIndex, CameraObject& cameraObj);

    // Function to randomly initialize the groundGrid
    void RandomizeGroundGrid();
};

class RotatingGameObject : public GameObject {
public: 
	float radian = 0; 
	glm::vec3 axis = glm::vec3(0, 1, 0); 

	~RotatingGameObject();

	void Update() override;
};


class CameraObject : GameObject{
public: 
	bool showCamera; 

    glm::vec3 position; // High angle position
    glm::vec3 target;      // Looking at origin
    glm::vec3 up;          // Up vector 

    float angle = 0;

    // Define projection parameters
    float fov;                       // Field of view in degrees
    float aspectRatio;     // Aspect ratio (adjust as needed)
    float nearPlane;                  // Near clipping plane
    float farPlane;                  // Far clipping plane 

    std::vector<GameObject*> targetGameObjects; 

    void AddTarget(GameObject* targetGameObj);

	glm::mat4 viewProjectionMatrix; 

	// Default Constructor
	CameraObject();

	// Function to set the viewProjectionMatrix
    void SetViewProjMat();

	// Getter for viewProjectionMatrix
	const glm::mat4& GetViewProjMat() const;

	glm::mat4& GetviewProjMat();
	glm::vec3& GetGlobalCameraPosition();

    void Update() override;

	void DrawGameObject(CameraObject& cameraObj) override;
private:
	// Method to initialize viewProjectionMatrix
	void InitializeCamera();
};
