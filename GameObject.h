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
#include "Event.h"

//class Animation; 

class Texture; 

class Transform; 

class CameraObject;

class Item;

using Publisher = std::function<void(const std::string&)>; // using 'Alias' = std::function<'returnType'('argType')>


class GameObject {
public: 
	GeneralMesh* ptrModel;
	Texture* ptrTexture;  
	//Animation* ptrAnimation;  
	Transform* ptrTransform;  
    
	ItemListener itemListener;

	virtual ~GameObject();

	void SetMesh(GeneralMesh* ptrModel);
	void SetTexture(Texture* ptrTexture);
	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
	virtual void SetTransform(Transform* ptrTransform);

	virtual void Update();
	virtual glm::mat4 GetModelMatrixFromTransform();
	virtual void DrawGameObject(CameraObject& cameraObj); 
    virtual void onEvent(const std::string& message);
	virtual void onEvent(GameObject* who, Item* item, int y, int x);

	void PublishItem(GameObject* who, Item* item, int y, int x) {
		 EventDispatcher& dispatcher = EventDispatcher::GetInstance(); 
		 dispatcher.Publish(who, item, y, x); // item is published!
	}

	void SubscribeItemListener() {
		itemListener = [this](GameObject* who, Item* item, int y, int x) {
			LOG(LOG_INFO, "ItemListenerTriggered::Typeid of gameObj on which event is triggered: " + std::string(typeid(*this).name()));
			this->onEvent(who, item, y, x);
		};
		
		// Pass a pointer to the stored lambda
		EventDispatcher& dispatcher = EventDispatcher::GetInstance();
		dispatcher.Subscribe(&itemListener);
		LOG(LOG_INFO, "Subscribing game object as Listener");
	}
}; 



class PlayableObject : public GameObject {
public:
	int yCoord = 0; int xCoord = 0;
	Item* heldItem;

    void onEvent(const std::string& message) override;
	void DrawGameObject(CameraObject& cameraObj) override;
	void DropItem();
	void PickUpItem(Item* item); 

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
