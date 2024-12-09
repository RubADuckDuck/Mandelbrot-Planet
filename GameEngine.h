#pragma once
#include "Mesh.h" 
#include "GameObject.h" 
#include "Event.h"

class GameEngine {
public: 
	
	std::vector<GameObject*> gameObjects; 
	 
	std::vector<Mesh*> reusableMeshes; 
	std::vector<Texture*> resualbeTexutures; 

	InputHandler inputHandler; 

	GameEngine() {
		inputHandler = InputHandler();
	} 

	void update() {
		inputHandler.pollEvents(); 
		GameObject* curGameObjPtr; 

		for (int i = 0; i < gameObjects.size(); i++) {
			curGameObjPtr = gameObjects[i]; 
			curGameObjPtr->Update();
		}
	}

	void CreateAndAddGameObject(std::string& meshPath, std::string& texturePath) { // no need for specification of coordinate? how odd for a gameengine 
		GeneralMesh* curMesh = new StaticMesh();  
		curMesh->LoadMesh(meshPath); 

		Texture* curTexture = new Texture(); 
		curTexture->LoadandSetTextureIndexFromPath(texturePath);  

		Transform* defaultTransform = new Transform();  

		GameObject* gameObject = new GameObject(); 
		gameObject->SetMesh(curMesh); 
		gameObject->SetTexture(curTexture); 
		gameObject->SetTransform(defaultTransform); 

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