#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"
#include "utils.h"
#include "LOG.h"

//class Animation; 

class Texture; 

class Transform; 

class CameraObject;


class GameObject {
public: 
	GeneralMesh* ptrModel;
	Texture* ptrTexture;  
	//Animation* ptrAnimation;  
	Transform* ptrTransform;  

	

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
    void onEvent(const std::string& message) override{
        glm::vec3 curTranslation = ptrTransform->GetTranslation(); 
        glm::vec3 temp = glm::vec3(0);
        

        if (message == "w_up") {
            temp = glm::vec3(0, 0, -1);
        } 
        else if (message == "s_up") {
            temp = glm::vec3(0, 0, 1);
        }
        else if (message == "d_up") {
            temp = glm::vec3(1, 0, 0);
        }
        else if (message == "a_up") {
            temp = glm::vec3(-1, 0, 0);
        }
        else {

        }
        curTranslation = curTranslation + temp;
        ptrTransform->SetTranslation(curTranslation);
    }
};

class TerrainObject : public GameObject {
public:
    static const int GRID_SIZE = 100;
    float BLOCK_SIZE = 0.5f;
    float BLOCK_OFFSET = 1.0f;

    enum MapLayerType {
        BIOME,        // Water, Grassland, volcano ...
        BUILDING,
        DROPPEDITEM,  // Resources
        PLAYER,
    };

    enum GroundType {
        WATER,
        GRASS,
        LAST // dummy that for iteration
    };

    GroundType groundGrid[GRID_SIZE][GRID_SIZE];
    Transform index2GroundTransform[GRID_SIZE][GRID_SIZE];

    std::map<GroundType, std::string> groundType2ObjPath; 
    std::map<GroundType, GeneralMesh*> groundType2Mesh;    // Use pointers to GeneralMesh 

    std::map<GroundType, std::string> groundType2TexturePath;
    std::map<GroundType, Texture*> groundType2Texture;     // Use pointers to Texture
    
    

    TerrainObject() {
        // Initialize paths for each GroundType
        groundType2ObjPath[WATER] = "E:/repos/[DuckFishing]/model/Water.obj";
        groundType2ObjPath[GRASS] = "E:/repos/[DuckFishing]/model/Grass.obj";

        groundType2TexturePath[WATER] = "E:/repos/[DuckFishing]/model/texture/Water.png";
        groundType2TexturePath[GRASS] = "E:/repos/[DuckFishing]/model/texture/Grass.png";

        // Load meshes and textures for each GroundType
        for (int groundTypeInt = WATER; groundTypeInt < LAST; groundTypeInt++) {
            GroundType groundType = static_cast<GroundType>(groundTypeInt); // 

            // Load Mesh
            groundType2Mesh[groundType] = new StaticMesh();
            groundType2Mesh[groundType]->LoadMesh(groundType2ObjPath[groundType]);

            // Load Texture
            groundType2Texture[groundType] = new Texture();
            groundType2Texture[groundType]->LoadandSetTextureIndexFromPath(groundType2TexturePath[groundType]);
        }

        // Randomly initialize the ground grid
        RandomizeGroundGrid();
    }

    void Update() override {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                // Set the transform positions for each block
                float xCoord = j * BLOCK_OFFSET;
                float yCoord = i * BLOCK_OFFSET;
                index2GroundTransform[i][j].SetScale(glm::vec3(BLOCK_SIZE));
                index2GroundTransform[i][j].SetTranslation(glm::vec3(xCoord, 0.0f, yCoord));
            }
        }
    }

    void DrawGameObject(CameraObject& cameraObj) override {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                DrawBlockOfTerrainAt(i, j, cameraObj);
            }
        }
    }

    void DrawBlockOfTerrainAt(int yIndex, int xIndex, CameraObject& cameraObj) {
        // Retrieve transform, ground type, mesh, and texture
        Transform& currTransform = index2GroundTransform[yIndex][xIndex];
        glm::mat4 transformMat = currTransform.GetTransformMatrix();

        GroundType currGroundType = groundGrid[yIndex][xIndex];
        GeneralMesh* currMesh = groundType2Mesh[currGroundType];
        Texture* currTexture = groundType2Texture[currGroundType];

        // Ensure mesh and texture are valid before rendering
        if (currMesh && currTexture) {
            currMesh->Render(cameraObj, transformMat, currTexture);
        }
    }

    // Function to randomly initialize the groundGrid
    void RandomizeGroundGrid() {
        // Seed the random number generator
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                // Assign a random GroundType
                groundGrid[i][j] = static_cast<GroundType>(std::rand() % LAST);
            }
        }
    }
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
