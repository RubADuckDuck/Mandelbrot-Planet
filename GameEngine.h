#pragma once
#include "Mesh.h" 
#include "GameObject.h" 
#include "Event.h"

class GameEngine {
public: 

	CameraObject camera;
	
	std::vector<GameObject*> gameObjects; 
	 
	std::vector<Mesh*> reusableMeshes; 
	std::vector<Texture*> resualbeTexutures; 

	InputHandler inputHandler; 

	GameEngine() {
		camera = CameraObject(); 
		inputHandler = InputHandler();
	} 

	void Update() {
		inputHandler.pollEvents(); 
		GameObject* curGameObjPtr; 

		for (int i = 0; i < gameObjects.size(); i++) {
			curGameObjPtr = gameObjects[i]; 
			curGameObjPtr->Update();
		}
	} 

	void Draw() {
		for (GameObject* ptrGameObj : gameObjects) {
			ptrGameObj->DrawGameObject(camera);
		}
	}

	void CreateAndAddGameObject(
		const std::string& meshPath, 
		std::string& texturePath,
		GLuint shaderProgram
		) 
	{  
		GeneralMesh* curMesh = new StaticMesh();  
		curMesh->LoadMesh(meshPath); 

		Texture* curTexture = new Texture(); 
		curTexture->LoadandSetTextureIndexFromPath(texturePath);  

		Transform* defaultTransform = new Transform();  

		GameObject* gameObject = new RotatingGameObject(); 
		gameObject->SetMesh(curMesh); 
		gameObject->SetTexture(curTexture); 
		gameObject->SetTransform(defaultTransform); 

		this->AddGameObjectToGameEngine(gameObject);
	} 

	void CreateAndAddGameObject(
		const std::string& meshPath,
		std::string& texturePath
	)
	{
		GeneralMesh* curMesh = new StaticMesh();
		curMesh->LoadMesh(meshPath);

		Texture* curTexture = new Texture();
		curTexture->LoadandSetTextureIndexFromPath(texturePath);

		Transform* defaultTransform = new Transform();

		GameObject* gameObject = new RotatingGameObject();
		gameObject->SetMesh(curMesh);
		gameObject->SetTexture(curTexture);
		gameObject->SetTransform(defaultTransform);

		this->AddGameObjectToGameEngine(gameObject);
	}

	void CreateAndAddGameObjectWithTransform( 
		const std::string& meshPath, 
		const std::string& texturePath, 
		const glm::vec3& translation = glm::vec3(0.0f), 
		const glm::vec3& scale = glm::vec3(1.0f), 
		const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f), 
		float rotationAngleRadians = 0.0f) // Angle in radians 
	{
		// Load mesh
		GeneralMesh* curMesh = new StaticMesh();
		curMesh->LoadMesh(meshPath);

		// Load texture
		Texture* curTexture = new Texture();
		curTexture->LoadandSetTextureIndexFromPath(texturePath);

		// Create and set up transform
		Transform* customTransform = new Transform();
		customTransform->SetTranslation(translation);
		customTransform->SetScale(scale);
		customTransform->SetRotation(rotationAngleRadians, rotationAxis);

		// Create the game object
		GameObject* gameObject = new GameObject();
		gameObject->SetMesh(curMesh);
		gameObject->SetTexture(curTexture);
		gameObject->SetTransform(customTransform);

		// Add to game engine
		this->AddGameObjectToGameEngine(gameObject);
	}
	
private: 
	void AddGameObjectToGameEngine(GameObject* gameObj) {
		Listener curListener; 
		curListener = [gameObj](const std::string& message) {
			gameObj->onEvent(message);
		}; 

		gameObjects.push_back(gameObj);
	}
};