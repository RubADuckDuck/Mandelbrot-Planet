#include "GameEngine.h"

std::string GameEngine::GetName() const { return "GameEngine"; }

asio::io_context* GameEngine::GetIOContext() {
	return io_context.get();
}

std::thread& GameEngine::GetIOThread() {
	return io_thread;
}

void GameEngine::RunIOContextOnIOThread() {
	log(LOG_INFO, "Run IO context on IO thread");
	io_thread = std::thread([this]() {
		io_context->run();
		});
}

void GameEngine::StopIOThread() {
	// Stop the event loop
	log(LOG_INFO, "Stop IO context on IO thread");
	io_context->stop();

	// Wait for the thread to finish
	if (io_thread.joinable()) {
		io_thread.join();
	}
}

GameEngine::GameEngine() {
	
	
	
	camera = CameraObject();
	inputHandler = InputHandler();

}

GameEngine::~GameEngine() {
	StopIOThread();
}

bool GameEngine::Initialize() {

	LOG(LOG_INFO, GetName() + "::Initializing Application Configuration");
	ApplicationConfig::Initialize(720, 480);  

	// Initialize core systems
	LOG(LOG_INFO, GetName() + "::Initializing System");
	systemManager = std::make_unique<SystemManager>();
	if (!systemManager->InitializeSDLAndOpenGL()) {
		return false;
	}

	// Initialize game modes
	LOG(LOG_INFO, GetName() + "::Initializing GameModes and GameModeController");
	modeController = std::make_unique<GameModeController>(this);
	modeController.get()->Initialize();

	// LOG(LOG_INFO, GetName() + "::Initializing io_context");
	log(LOG_INFO, "Initializing io_context"); 
	io_context = std::make_unique<asio::io_context>();


	log(LOG_INFO, "Initializing FPS tracker");
	deltaTime = 0.0f;
	fps = 0.0f;
	fpsUpdateInterval = 0.5f;  // Update FPS display twice per second
	fpsTimer = 0.0f;
	frameCount = 0;
	lastFrameTime = std::chrono::steady_clock::now();

	return true;
}

void GameEngine::Update() {
	// Calculate delta time
	auto currentTime = std::chrono::steady_clock::now();
	deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
	lastFrameTime = currentTime;

	// Update FPS calculation
	fpsTimer += deltaTime;
	frameCount++;

	// Update FPS value every fpsUpdateInterval seconds
	if (fpsTimer >= fpsUpdateInterval) {
		fps = static_cast<float>(frameCount) / fpsTimer;  // Calculate average FPS

		// Log the current FPS
		log(LOG_INFO, "FPS: " + std::to_string(static_cast<int>(fps)));

		// Reset counters
		fpsTimer = 0.0f;
		frameCount = 0;
	}

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


