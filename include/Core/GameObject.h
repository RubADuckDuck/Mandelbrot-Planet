#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility> // Required for std::pair
#include <stack>

#include "../Rendering/Mesh.h"
#include "../Utils/utils.h"
#include "../Utils/LOG.h"
#include "MessageParser.h"
#include "ItemType.h"
#include "GlobalMappings.h"
#include "FactoryType.h"
#include "GroundType.h"
#include "Event.h"
#include "InteractionInfo.h"
#include "PlayerDirection.h"

#include "ObjectType.h"


//class Animation; 

class Texture; 

class Transform; 

class CameraObject;

class Item;

using Publisher = std::function<void(const std::string&)>; // using 'Alias' = std::function<'returnType'('argType')>




class GameObject {
public:
	virtual std::string GetName() const;

	// client & server 
	virtual uint8_t GetTypeID();
protected:
	void log(LogLevel level, std::string text);

public: 
	Transform* ptrNodeTransform_;
	std::stack<glm::mat4> modelTransformMats_;  
	glm::mat4 prevGridTransform_; 

	std::unordered_map<uint32_t, glm::mat4> gameObjectID2respectiveTransformationMat; 

	float interpolationValue_ = 1; // scales from 0 to 1, > 1 means interpolation is complete

	uint32_t meshID_;  
	uint32_t textureID_;  

	uint32_t parentID_; 

	virtual ~GameObject();



public: 	
	GameObject()
		: ptrNodeTransform_(new Transform()),
		meshID_(0), textureID_(0) {} 

	GameObject(uint32_t meshID, uint32_t textureID) 
		: ptrNodeTransform_(new Transform()),
		meshID_(meshID), textureID_(textureID) {}

	GameObject(uint32_t objID, uint32_t meshID, uint32_t textureID)
		: ptrNodeTransform_(new Transform()),
		meshID_(meshID), textureID_(textureID) {
		this->SetID(objID); 
	}

	uint32_t GetParentID() {
		return parentID_;
	}

	void AddToRenderList(uint32_t gameObjectID, glm::mat4 transformMat) { 
		auto it = gameObjectID2respectiveTransformationMat.find(gameObjectID); 

		if (it == gameObjectID2respectiveTransformationMat.end()) {
			// no duplicates  
			gameObjectID2respectiveTransformationMat[gameObjectID] = transformMat; 
		}
		else {
			log(LOG_ERROR, "::AddToRenderList the transformMatrix of this gameObject has already been registered"); 
		}
	}

	glm::mat4 GetMatrixFromRenderList(uint32_t gameObjectID) {
		auto it = gameObjectID2respectiveTransformationMat.find(gameObjectID);

		if (it == gameObjectID2respectiveTransformationMat.end()) {
			log(LOG_ERROR, "Doesn'tExist" );
			return glm::mat4(1); 
		}
		else {
			return gameObjectID2respectiveTransformationMat[gameObjectID];
		}
	}

	void ClearRenderList() {
		gameObjectID2respectiveTransformationMat.clear(); 
	}

	// client::init
	void SetMeshID(uint32_t id) {
		meshID_ = id; 
	}
	void SetTextureID(uint32_t id) {
		textureID_ = id; 
	}
	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
	virtual void SetTransform(Transform* ptrTransform);

	// server 
	virtual void Update();
	virtual void Update(float deltaTime); 

	//// I think draw will need this no more, since we would have a separate renderer class. 
	//// be ready to remove these methods 
	//virtual void SetTransformMatrixBeforeDraw();
	//virtual void DrawGameObject(CameraObject& cameraObj);

	//// too bad. messages are dealt by other classes. These will no longer be used.  
	//virtual void onEvent(const std::vector<uint8_t> message);
	//virtual void onEvent(InteractionInfo* interactionInfo);

	//// why publish an Item, when you can publish a Customizable message?
	//void PublishItem(InteractionInfo* interactionInfo) {
	//	 EventDispatcher& dispatcher = EventDispatcher::GetInstance(); 
	//	 dispatcher.Publish(interactionInfo); // item is published!
	//}

	void SetID(uint32_t newID) {
		if (hasID_) {
			LOG(LOG_WARNING, "This game object already has an ID. ID will not be altered");
		}
		else {
			hasID_ = true;
			this->objectID_ = newID; 
		}
		return;
	} 

public:
	uint32_t GetID() {
		if (!hasID_) {
			LOG(LOG_ERROR, "This object has never been given an ID"); 
			return 0; 
		} 
		else {
			return this->objectID_; 
		} 
	}

private: 
	bool hasID_ = false;
	uint32_t objectID_ = 0;  
}; 

//
//class GameObject {
//public: 
//	GameObject* parentGameObject; 
//	std::vector<GameObject*> childrenGameObjects;
//
//	Transform* ptrNodeTransform;
//	glm::mat4 modelTransformMat;
//	  
//	GeneralMesh* ptrModel; 
//	Texture* ptrTexture;   
//	// Animation* ptrAnimation;  
//
//    
//	ItemListener itemListener;
//
//	virtual ~GameObject();
//
//	// client & server
//	void RemoveChild(GameObject* child); 
//	void AddChild(GameObject* child); 
//	void SetParent(GameObject* parent); 
//
//	// client & server 
//	virtual uint8_t GetTypeID() { return 0; };
//
//	// client::init
//	void SetMesh(GeneralMesh* ptrModel);
//	void SetTexture(Texture* ptrTexture);
//	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
//	virtual void SetTransform(Transform* ptrTransform);
//
//	// client::routine 
//	virtual void SetTransformMatrixBeforeDraw();
//	virtual void DrawGameObject(CameraObject& cameraObj);
//
//	// server 
//	virtual void Update();
//	virtual void Update(float deltaTime); 
//	virtual void onEvent(const std::vector<uint8_t> message);
//	virtual void onEvent(InteractionInfo* interactionInfo);
//
//	// server 
//	void PublishItem(InteractionInfo* interactionInfo) {
//		 EventDispatcher& dispatcher = EventDispatcher::GetInstance(); 
//		 dispatcher.Publish(interactionInfo); // item is published!
//	}
//
//	// server 
//	void SubscribeItemListener() {
//		itemListener = [this](InteractionInfo* interactionInfo) {
//			LOG(LOG_INFO, "ItemListenerTriggered::Typeid of gameObj on which event is triggered: " + std::string(typeid(*this).name()));
//			this->onEvent(interactionInfo);
//		};
//		
//		// Pass a pointer to the stored lambda
//		EventDispatcher& dispatcher = EventDispatcher::GetInstance();
//		dispatcher.Subscribe(&itemListener);
//		LOG(LOG_INFO, "Subscribing game object as Listener");
//	}
//
//	void SetID(uint32_t newID) {
//		if (hasID) {
//			LOG(LOG_WARNING, "This game object already has an ID. ID will not be altered");
//		}
//		else {
//			hasID = true;
//			this->objectID = newID; 
//		}
//		return;
//	} 
//
//public:
//	uint32_t GetID() {
//		if (!hasID) {
//			LOG(LOG_ERROR, "This object has never been given an ID"); 
//			return 0; 
//		}
//		else {
//			return this->objectID; 
//		}
//	}
//
//private: 
//	bool hasID = false;
//	uint32_t objectID = 0;  
//}; 




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


//#define MAX_STRUCTURE_LENGTH 5
//
//class RotatingGameObject : public GameObject {
//public: 
//	float radian = 0; 
//	glm::vec3 axis = glm::vec3(0, 1, 0); 
//
//	~RotatingGameObject();
//
//	void Update() override;
//};
//
//


class CameraObject : GameObject{
private:
	bool rotationEnabled = true;  // Toggle for rotation
	float rotationAngle = 0.0f;   // Current rotation angle in radians
	float rotationSpeed = 0.5f;    // Rotations per second
	float orbitRadius = 10.0f;     // Distance from center
	float orbitHeight = 2.0f;     // Height of camera above ground

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
private:
	// Method to initialize viewProjectionMatrix
	void InitializeCamera();
};
