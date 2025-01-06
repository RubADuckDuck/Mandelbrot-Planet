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
#include "InteractionInfo.h"
#include "PlayerDirection.h"

#include "ObjectType.h"

#include "../NetCustomCommon/NetworkMessage.h"

//class Animation; 

class Texture; 

class Transform; 

class CameraObject;

class Item;

using Publisher = std::function<void(const std::string&)>; // using 'Alias' = std::function<'returnType'('argType')>


class GameObject {
public: 
	GameObject* parentGameObject; 
	std::vector<GameObject*> childrenGameObjects;

	Transform* ptrNodeTransform;
	glm::mat4 modelTransformMat;
	  
	GeneralMesh* ptrModel; 
	Texture* ptrTexture;   
	// Animation* ptrAnimation;  

    
	ItemListener itemListener;

	virtual ~GameObject();

	// client & server
	void RemoveChild(GameObject* child); 
	void AddChild(GameObject* child); 
	void SetParent(GameObject* parent); 

	// client & server 
	virtual uint8_t GetTypeID() { return 0; };

	// client::init
	void SetMesh(GeneralMesh* ptrModel);
	void SetTexture(Texture* ptrTexture);
	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
	virtual void SetTransform(Transform* ptrTransform);

	// client::routine 
	void SetTransformMatrixBeforeDraw();
	virtual void DrawGameObject(CameraObject& cameraObj);

	// server 
	virtual void Update();
	virtual void Update(float deltaTime); 
	virtual void onEvent(const std::string& message);
	virtual void onEvent(InteractionInfo* interactionInfo);

	// server 
	void PublishItem(InteractionInfo* interactionInfo) {
		 EventDispatcher& dispatcher = EventDispatcher::GetInstance(); 
		 dispatcher.Publish(interactionInfo); // item is published!
	}

	// server 
	void SubscribeItemListener() {
		itemListener = [this](InteractionInfo* interactionInfo) {
			LOG(LOG_INFO, "ItemListenerTriggered::Typeid of gameObj on which event is triggered: " + std::string(typeid(*this).name()));
			this->onEvent(interactionInfo);
		};
		
		// Pass a pointer to the stored lambda
		EventDispatcher& dispatcher = EventDispatcher::GetInstance();
		dispatcher.Subscribe(&itemListener);
		LOG(LOG_INFO, "Subscribing game object as Listener");
	}

	void SetID(uint32_t newID) {
		if (hasID) {
			LOG(LOG_WARNING, "This game object already has an ID. ID will not be altered");
		}
		else {
			hasID = true;
			this->objectID = newID; 
		}
		return;
	} 

	uint32_t GetID() {
		if (!hasID) {
			LOG(LOG_ERROR, "This object has never been given an ID"); 
			return 0; 
		}
		else {
			return this->objectID; 
		}
	}

private: 
	bool hasID = false;
	uint32_t objectID = 0;  
}; 

class GameObjectOnGrid : public GameObject {
public: 
	int yCoord = 0; int xCoord = 0;
	Direction direction = Direction::UP;
	int orientation = 0; 

	

	void SetCoordinates(int y, int x) { 
		// publish change to server 
		// however to generate message, 
		// I should know what the id is of this particular instance. 

		// there are two choices 
		// 1. Add ID variable to objects. We will have to make the implementations of sending messages explicit within each instance of gameobject class. 
		// 2. 
		

		yCoord = y;
		xCoord = x;
	}
};

class PlayableObject : public GameObjectOnGrid {
public:
	Item* heldItem; 
	
	// server & client
	uint8_t GetTypeID() override;

	// client
	void DrawGameObject(CameraObject& cameraObj) override;

	// server 
	void onEvent(const std::string& message) override;
	void TakeAction(Direction direction);
	void DropItem(); 
	void RequestWalk(); 
	void Walk(); 
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
