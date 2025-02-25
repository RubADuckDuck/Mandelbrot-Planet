#pragma once
#include "../Rendering/Mesh.h" 
#include "GameObject.h" 
#include "Event.h"
#include "SystemManager.h"
#include "GameModeController.h"

class GameEngine {
public: 
	std::string GetName() const; 

	asio::io_context* GetIOContext();

	std::thread& GetIOThread();

	void RunIOContextOnIOThread();

	void StopIOThread();


private: 
	void log(LogLevel level, std::string text) {
		LOG(level, GetName() + "::" + text); 
	}

	std::unique_ptr<SystemManager> systemManager;
	std::unique_ptr<GameModeController> modeController;

	InputHandler inputHandler;

	std::unique_ptr<asio::io_context> io_context;
	std::thread io_thread;
public: 

	CameraObject camera;

	std::list<Listener> listeners;
	std::vector<GameObject*> gameObjects; 
	 
	std::vector<Mesh*> reusableMeshes; 
	std::vector<Texture*> resualbeTexutures; 


	GameEngine();

	~GameEngine();

	bool Initialize();

	void Update();
	 
	void Draw();

	uint32_t GetUserID() {

	}

public:
	InputHandler* GetInputHandler();
	GameModeController* GetModeController(); 
	SDL_Window* GetWindow() {
		return systemManager->GetWindow();
	}
};