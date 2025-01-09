#pragma once
#include "Mesh.h" 
#include "GameObject.h" 
#include "Event.h"
#include "SystemManager.h"
#include "GameModeController.h"

class GameEngine {
private: 
	std::unique_ptr<SystemManager> systemManager;
	std::unique_ptr<GameModeController> modeController;

public: 


	CameraObject camera;

	std::list<Listener> listeners;
	std::vector<GameObject*> gameObjects; 
	 
	std::vector<Mesh*> reusableMeshes; 
	std::vector<Texture*> resualbeTexutures; 

	InputHandler inputHandler; 

	GameEngine();

	bool Initialize();

	void Update();
	 
	void Draw();

public:
	InputHandler* GetInputHandler();
	GameModeController* GetModeController();

public:

	void CreateAndAddGameObject( // I recommed not to use this. Shaders are no longer manually passed.
		const std::string& meshPath, 
		std::string& texturePath,
		GLuint shaderProgram
		);

	void CreateAndAddGameObject(
		const std::string& meshPath,
		std::string& texturePath
	);

	void CreateAndAddGameObjectWithTransform( 
		const std::string& meshPath, 
		const std::string& texturePath, 
		const glm::vec3& translation = glm::vec3(0.0f), 
		const glm::vec3& scale = glm::vec3(1.0f), 
		const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f), 
		float rotationAngleRadians = 0.0f);

	void DirectlyAddGameObject(GameObject* newGameObject);
	
	void ManuallySubscribe(GameObject* gameObj);
private: 
	void AddGameObjectToGameEngine(GameObject* gameObj);
};