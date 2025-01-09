#include "GameEngine.h"

GameEngine::GameEngine() {
	camera = CameraObject();
	inputHandler = InputHandler();
}

bool GameEngine::Initialize() {
	// Initialize core systems
	systemManager = std::make_unique<SystemManager>();
	if (!systemManager->InitializeSDLAndOpenGL()) {
		return false;
	}

	// Initialize game modes
	modeController = std::make_unique<GameModeController>(this);
	modeController.get()->Initialize();

	return true;
}

void GameEngine::Update() {
	inputHandler.pollEvents();
	if (inputHandler.isQuit()) {
		ApplicationConfig::SetQuit(true);
		return;
	}

	modeController.get()->Update();
}

void GameEngine::Draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	modeController.get()->Draw();  // Draw current game mode
	SDL_GL_SwapWindow(systemManager->GetWindow());
}

InputHandler* GameEngine::GetInputHandler() { return &inputHandler; }

GameModeController* GameEngine::GetModeController() { return modeController.get(); }

void GameEngine::CreateAndAddGameObject(const std::string& meshPath, std::string& texturePath, GLuint shaderProgram)
{
	GeneralMesh* curMesh = new StaticMesh();
	curMesh->LoadMesh(meshPath);

	Texture* curTexture = new Texture();
	curTexture->LoadandSetTextureIndexFromPath(texturePath);

	Transform* defaultTransform = new Transform();
	defaultTransform->SetScale(glm::vec3(0.2f));
	defaultTransform->SetTranslation(glm::vec3(0, 1.0f, 0));

	GameObject* gameObject = new RotatingGameObject();
	gameObject->SetMesh(curMesh);
	gameObject->SetTexture(curTexture);
	gameObject->SetTransform(defaultTransform);

	this->AddGameObjectToGameEngine(gameObject);
}

void GameEngine::CreateAndAddGameObject(const std::string& meshPath, std::string& texturePath)
{
	GeneralMesh* curMesh = new StaticMesh();
	curMesh->LoadMesh(meshPath);

	Texture* curTexture = new Texture();
	curTexture->LoadandSetTextureIndexFromPath(texturePath);

	Transform* defaultTransform = new Transform();
	defaultTransform->SetScale(glm::vec3(0.2f));
	defaultTransform->SetTranslation(glm::vec3(0, 1.0f, 0));

	GameObject* gameObject = new PlayableObject();
	gameObject->SetMesh(curMesh);
	gameObject->SetTexture(curTexture);
	gameObject->SetTransform(defaultTransform);

	this->AddGameObjectToGameEngine(gameObject);
}

void GameEngine::CreateAndAddGameObjectWithTransform(const std::string& meshPath, const std::string& texturePath, const glm::vec3& translation, const glm::vec3& scale, const glm::vec3& rotationAxis, float rotationAngleRadians) // Angle in radians 
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

void GameEngine::DirectlyAddGameObject(GameObject* newGameObject) {
	AddGameObjectToGameEngine(newGameObject);
}

void GameEngine::ManuallySubscribe(GameObject* gameObj) {
	// Store the lambda in a persistent container
	listeners.emplace_back([gameObj](const std::vector<uint8_t> message) {
		LOG(LOG_INFO, "Typeid of gameObj on which event is triggered: " + std::string(typeid(*gameObj).name()));
		gameObj->onEvent(message);
		});

	// Pass a pointer to the stored lambda
	inputHandler.Subscribe(&listeners.back());
	LOG(LOG_INFO, "Subscribing game object as Listener");

	// Check if the game object is an instance of PlayableObject
	if (PlayableObject* playableObj = dynamic_cast<PlayableObject*>(gameObj)) {
		camera.AddTarget(playableObj); // Add to target list
		LOG(LOG_INFO, "Added PlayableObject to target list: " + std::string(typeid(*playableObj).name()));
	}
}

void GameEngine::AddGameObjectToGameEngine(GameObject* gameObj) {
	//// Store the lambda in a persistent container
	//listeners.emplace_back([gameObj](const std::string& message) {
	//	LOG(LOG_INFO, "Typeid of gameObj on which event is triggered: " + std::string(typeid(*gameObj).name()));
	//	gameObj->onEvent(message);
	//	});

	//// Pass a pointer to the stored lambda
	//inputHandler.Subscribe(&listeners.back());

	//LOG(LOG_INFO, "Subscribing game object as Listener"); 

	gameObjects.push_back(gameObj);

	// Check if the game object is an instance of PlayableObject
	if (PlayableObject* playableObj = dynamic_cast<PlayableObject*>(gameObj)) {
		ManuallySubscribe(playableObj);
	}
}
