#include <SDL.h> 
#include <SDL_main.h> 
#include <glad/glad.h>
#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream> 
#include <vector>

#define STB_IMAGE_IMPLEMENTATION

#include "Mesh.h"
#include "GameEngine.h"
#include "TerrainManager.h"

// Globals 
int gScreenHeight = 960;
int gScreenWidth = 1280;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

bool gQuit = false;

// OpenGL Globals
GLuint shaderProgram;

GameEngine gameEngine = GameEngine();


void GetOpenGLVersionInfo() {
	glGetString(GL_RENDERER);
	std::cout << "Vendor" << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer" << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version" << glGetString(GL_VERSION) << std::endl;
	std::cout << "Shading Language" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
} 

void InitializaProgram() {
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL could not init" << std::endl; 
		exit(1); 
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1); 

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); 

	gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL",
		100, 100,
		gScreenWidth, gScreenHeight,
		SDL_WINDOW_OPENGL
	);

	if (gGraphicsApplicationWindow == nullptr) {
		std::cout << "Graphics Window not initialized properly" << std::endl;
		exit(1);
	}

	gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);

	if (gOpenGLContext == nullptr) {
		std::cout << "OpenGL context not created" << std::endl;

		exit(1);
	} 

	// initialize the glad lib 
	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		std::cout << "glad was not initialized" << std::endl; 
		exit(1); 
	} 

	glEnable(GL_DEPTH_TEST);

	GetOpenGLVersionInfo(); 

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	glCullFace(GL_BACK);


	// debugging 
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar* message, const void* userParam) {
			std::cerr << "GL DEBUG: " << message << std::endl;
		}, nullptr);

	// GameEngine ge = GameEngine(); 
	/* 
	for some reason, of which I have no idea, when I Create the game EngineObject, the SDL window crashes. 
	I'm wondering if it is possible, and if so, for what reason?
	*/
	
	// load meshmanager and load model
	std::string objPath = "E:\\repos\\[DuckFishing]\\model\\duck.obj";
	std::string texturePath = "E:/repos/[DuckFishing]/model/texture/duck.png"; 

	GLuint testVertArr;
	glGenVertexArrays(1, &testVertArr);
	std::cout << "Index of Vertex array generated for test purpose: " << testVertArr << std::endl;


	TerrainManager* ptrTerrainManager = new TerrainManager();  

	ptrTerrainManager->SetGameEngine(&gameEngine);
	ptrTerrainManager->CreateAndAddPlayer(objPath, texturePath);
	ptrTerrainManager->SubscribeItemListener();

	{
		ptrTerrainManager->CreateAndAddDroppedItemAt(1, 1, ItemType::PYTHON);
	}
	{
		ptrTerrainManager->BuildFactoryAt(FactoryType::Bakery, 5, 5, 2, 1);
	}
	
	gameEngine.DirectlyAddGameObject(ptrTerrainManager);
}

void debugging() {
	std::cout << gQuit << std::endl; 
}

void MainLoop() {
	const int targetFPS = 60; // Target frame rate
	const float targetFrameTime = 1.0f / targetFPS; // Time per frame in seconds

	while (!gQuit) {
		// Start frame timer
		Uint64 startTicks = SDL_GetPerformanceCounter();


		// debugging();

		// Update game state
		gameEngine.Update();

		if (gameEngine.inputHandler.isQuit()) {
			gQuit = true;
			LOG(LOG_INFO, "GoodBye, Closing SDL");
		}

		// Render game state
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear buffer
		gameEngine.Draw(); // draw on cleared buffer
		SDL_GL_SwapWindow(gGraphicsApplicationWindow); // swap buffer

		// End frame timer and calculate frame time
		Uint64 endTicks = SDL_GetPerformanceCounter();
		float frameTime = (float)(endTicks - startTicks) / SDL_GetPerformanceFrequency();

		// Delay if the frame was rendered too quickly
		if (frameTime < targetFrameTime) {
			SDL_Delay((Uint32)((targetFrameTime - frameTime) * 1000.0f)); // Convert to milliseconds
		}
	}
}

void CleanUp() {
	SDL_DestroyWindow(gGraphicsApplicationWindow);

	SDL_Quit();
}


int main(int argc, char* argv[]) {
	InitializaProgram();

	MainLoop();

	CleanUp();

	return 0;
}