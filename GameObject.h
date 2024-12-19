#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility> // Required for std::pair

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

enum MapLayerType {
    BIOME,        // Water, Grassland, volcano ...
    STRUCTURE, // buildings, Primary Resources (one time use blocks)
    DROPPEDITEM,  // Resources
    PLAYER,
    LAST
};

enum GroundType {
    WATER,
    GRASS,
    LAST // dummy that for iteration
};

enum StructureType {
    TREE, 
    ROCK, 
    WHEATFIELD,
    BAKERY, 
    TOOLFACTORY,
    WALL,
    LAST
};

enum ItemType {
    WOOD, 
    ROCK, 
    IRON, 
    RUBY,
    PYTHON,
    LAST
};

#define MAX_STRUCTURE_LENGTH 5; 

using Publisher = std::function<void(const std::string&)>; // using 'Alias' = std::function<'returnType'('argType')>

using Item2Probability = std::map<ItemType, float>; 


class StructureObject : public GameObject {

public: 
    Publisher publisher; 

    bool buildingShape[MAX_STRUCTURE_LENGTH][MAX_STRUCTURE_LENGTH]; 

    virtual void TriggerInteraction(const std::string& msg) = 0;
};

struct Recipe {
    std::vector<ItemType> inputPortIndex2RequiredItem;  
    std::vector<Item2Probability*> outputPortIndex2ToProductItem;
    float craftingDuration; 
};

class FactoryGameObjects : public StructureObject { 
    /*
        FactoryType Structures have input ports that receive certain kinds of resources and OutputPorts that throw the produced objects to a certain building.
        InputPorts and Output ports are designated from a block that is part of the buildings shape. 

    */

    int nInputPort; 
    int nOutputPort; // output port index is (nInputPort + index) 

    std::vector<std::pair<int, int>> portIndex2PortPosition; 
    std::vector<ItemType> portIndex2Item;

    std::map<std::pair<int, int>, int> Position2PortIndex; 

    std::vector<Recipe*> craftingRecipes; 
    bool isCrafting; 

    virtual void InitFactory() = 0; 

    Item2Probability* CheckForMatchingIngredient(std::vector<ItemType>& ingredients) {
        for (int j = 0; j < craftingRecipes.size(); j++) { Recipe* currRecipe = craftingRecipes[j];
            for (int i = 0; i < ingredients.size(); i++) {
                if (currRecipe->inputPortIndex2RequiredItem[i] != ingredients[i]) {
                    // not the right recipe 
                    break;
                }
                else {
                    if (i == ingredients.size() - 1) {
                        LOG(LOG_INFO, "Found recipe for current configuration");  

                        return currRecipe->outputPortIndex2ToProductItem[j];
                    }
                }
            }
        }
        return nullptr;
    }

    void TriggerInteraction(const std::string& msg) {
        // parse message to get which position the player is trying to interact 
        std::pair<int, int> targetPosition = this->parsePosition(msg);
        ItemType usedItem = this->parseItem(msg);

        if (targetPosition in Position2PortIndex) {
            int targetPortIndex = Position2PortIndex[targetPosition];
            
            // temporarily save what was in at targetport 
            ItemType temp = portIndex2Item[targetPortIndex]; 

            // add the used Item to the target port slot! / we don't have to take away Item from player.
            // they probably already did that by themselves.
            portIndex2Item[targetPortIndex] = usedItem; 

            // If
        }
        else { 
            return; 
        }
        

    }
};

class NaturalResourceStructureObject : public StructureObject {
    static std::map<ItemType, const std::string> itemType2ItemName; 
    static std::map<const std::string, ItemType> itemName2ItemType;

    std::map<ItemType, float> itemType2ProbabilityOfDroppingIt; 

    void TriggerInteraction(const std::string& msg) {
        // Decide Drop
        ItemType currDrop = this->RandomRollDrop();

        LOG(LOG_INFO, "DROP_" + itemType2ItemName[currDrop]);

        (*publisher)("DROP_" + itemType2ItemName[currDrop]); // publish message
    }

    ItemType RandomRollDrop() {
        // do a randomRoll
        ItemType result; 

        return result;
    }
};

class TerrainObject : public GameObject {
public:
    static const int GRID_SIZE = 100;
    float BLOCK_SIZE = 0.5f;
    float BLOCK_OFFSET = 1.0f;



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
