#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"
#include "utils.h"

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

class TerrainObject : public GameObject {
public:
    static const int GRID_SIZE = 100;
    float BLOCK_SIZE = 1.0f;
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
    };

    GroundType groundGrid[GRID_SIZE][GRID_SIZE];
    Transform index2GroundTransform[GRID_SIZE][GRID_SIZE];

    std::map<GroundType, GeneralMesh*> groundType2Mesh;    // Use pointers to GeneralMesh
    std::map<GroundType, Texture*> groundType2Texture;     // Use pointers to Texture

    void Update() override {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                // Set the transform positions for each block
                float xCoord = j * BLOCK_OFFSET;
                float yCoord = i * BLOCK_OFFSET;
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
	glm::mat4 viewProjectionMatrix; 

	// Default Constructor
	CameraObject();

	// Function to set the viewProjectionMatrix
	void SetViewProjMat(float fov, float aspectRatio, float nearPlane, float farPlane);

	// Getter for viewProjectionMatrix
	const glm::mat4& GetViewProjMat() const;

	void SetViewProjMat();
	glm::mat4& GetviewProjMat();
	glm::vec3& GetGlobalCameraPosition();

	void DrawGameObject(CameraObject& cameraObj) override;
private:
	// Method to initialize viewProjectionMatrix
	void InitializeCamera();
};
