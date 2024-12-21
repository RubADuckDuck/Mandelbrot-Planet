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



//
//class MeshManager {
//public: 
//	std::vector<Mesh> meshes; 
//
//	MeshManager() {
//		std::cout << "MeshManager Created" << std::endl; 
//	} 
//
//	~MeshManager() { 
//		Mesh temp; 
//		for (int i = 0; i < this->meshes.size(); i++) {
//			meshes.pop_back(); 
//		}
//
//		std::cout << "MeshManager Destroyed" << std::endl;
//	}
//
//	void AddMesh(std::string path) {
//		
//		Mesh newMesh = this->LoadModel(path);  
//
//		meshes.push_back(newMesh); 
//
//	}
//
//	GLuint LoadTexture(const std::string& path) {
//		GLuint textureIndex;
//		glGenTextures(1, &textureIndex);
//		glBindTexture(GL_TEXTURE_2D, textureIndex);
//
//		// Load texture using your favorite library (e.g., stb_image)
//		int width, height, nrChannels;
//		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
//		if (data) {
//			GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
//			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//			glGenerateMipmap(GL_TEXTURE_2D);
//		}
//		else {
//			std::cerr << "Failed to load texture: " << path << std::endl;
//		}
//		stbi_image_free(data);
//
//		return textureIndex;
//	}
//
//	GLuint LoadMaterials(const aiScene* scene) {
//		for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
//			aiMaterial* material = scene->mMaterials[i];
//
//			aiString texPath;
//			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
//				std::string textureFile = texPath.C_Str();
//
//				// Call the LoadTexture function here
//				GLuint textureIndex = LoadTexture(textureFile);
//
//				return textureIndex; 
//			}
//		}
//	} 
//
//	Mesh LoadModel(const std::string& path) {
//		Mesh newMesh = Mesh(); 
//
//		GLuint VAO; 
//		GLuint VBO; 
//		GLuint EBO; 
//
//		GLuint textureIndex; 
//		int indexCount = 0; // bit confused if it's number of verts or faces 
//
//		Assimp::Importer importer;
//		const aiScene* scene = importer.ReadFile(
//			path,
//			aiProcess_Triangulate |
//			aiProcess_JoinIdenticalVertices |
//			aiProcess_FlipUVs |  // Corrects UV mapping if needed
//			aiProcess_CalcTangentSpace
//		);
//
//		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
//			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
//			exit(1);
//		}
//
//		std::vector<float> vertices;
//		std::vector<unsigned int> indices;
//
//		// Process only the first mesh (for simplicity)
//		aiMesh* mesh = scene->mMeshes[0];
//
//		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//			// Position
//			aiVector3D vertex = mesh->mVertices[i];
//			vertices.push_back(vertex.x);
//			vertices.push_back(vertex.y);
//			vertices.push_back(vertex.z);
//
//			// Normal
//			aiVector3D normal = mesh->mNormals[i];
//			vertices.push_back(normal.x);
//			vertices.push_back(normal.y);
//			vertices.push_back(normal.z);
//
//			// Texture Coordinates
//			if (mesh->mTextureCoords[0]) { // Check if the mesh has texture coordinates
//				aiVector3D texCoord = mesh->mTextureCoords[0][i];
//				vertices.push_back(texCoord.x);
//				vertices.push_back(texCoord.y); 
//				std::cout << texCoord.x << "," << texCoord.y << std::endl;
//			}
//			else {
//				vertices.push_back(0.0f); // Default texture coordinate
//				vertices.push_back(0.0f);
//				std::cout << "No UV value" << std::endl;
//			}
//		}
//
//		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//			aiFace face = mesh->mFaces[i];
//			for (unsigned int j = 0; j < face.mNumIndices; j++) {
//				indices.push_back(face.mIndices[j]);
//				indexCount++;
//			} 
//
//			
//		}
//
//		// Generate buffers
//		glGenVertexArrays(1, &VAO);
//		glGenBuffers(1, &VBO);
//		glGenBuffers(1, &EBO);
//
//		glBindVertexArray(VAO);
//
//		glBindBuffer(GL_ARRAY_BUFFER, VBO);
//		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
//
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
//
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Position
//		glEnableVertexAttribArray(0);
//
//		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
//		glEnableVertexAttribArray(1);
//
//		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Texture Coordinates
//		glEnableVertexAttribArray(2);
//
//		glBindVertexArray(0);
//
//		newMesh.VAO = VAO; 
//		newMesh.VBO = VBO; 
//		newMesh.EBO = EBO; 
//
//		newMesh.indexCount = indexCount; 
//
//		// load texture from material
//		textureIndex = LoadMaterials(scene); 
//
//		newMesh.textureIndex = textureIndex; 
//
//		return newMesh; 
//	}
//
//	void DrawModel(float deltaTime) {
//
//		static float rotationAngle = 0.0f;
//		rotationAngle += 50.0f * deltaTime;
//
//		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
//		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
//		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
//		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gScreenWidth / gScreenHeight, 0.1f, 100.0f);
//
//
//		glUseProgram(shaderProgram);
//
//		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
//		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
//		GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
//		GLuint textureLoc = glGetUniformLocation(shaderProgram, "texture_diffuse");
//
//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//
//		for (const auto& mesh : this->meshes) {
//			glBindVertexArray(mesh.VAO);
//
//			// bind texture 
//			glActiveTexture(GL_TEXTURE0);
//			glBindTexture(GL_TEXTURE_2D, mesh.textureIndex);
//			glUniform1i(textureLoc, 0);
//
//			// render the mesh
//			glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
//
//			glBindVertexArray(0);
//		} 
//
//		glUseProgram(0);
//	}
//};
//
//class Camera {
//public:
//	MeshManager* ptrMeshManager;
//
//	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
//	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gScreenWidth / gScreenHeight, 0.1f, 100.0f);
//
//	Camera() {
//		std::cout << "Camera initiated" << std::endl;
//	}
//
//	Camera(MeshManager* ptrMeshMng) {
//		this->ptrMeshManager = ptrMeshMng;
//	}
//
//	~Camera() { 
//		std::cout << "Camera Destroyed" << std::endl;
//		// delete& ptrMeshManager;
//	}
//
//	void SetMeshManager(MeshManager* ptrMeshMng) {
//		this->ptrMeshManager = ptrMeshMng;
//	}
//
//	MeshManager* GetManager(void) {
//		return this->ptrMeshManager;
//	}
//
//	void DrawModel(float deltaTime) {
//
//		static float rotationAngle = 0.0f;
//		rotationAngle += 50.0f * deltaTime;
//
//		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
//		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
//
//
//		glUseProgram(shaderProgram);
//
//		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
//		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
//		GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
//		GLuint textureLoc = glGetUniformLocation(shaderProgram, "texture_diffuse");
//
//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(this->view));
//		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(this->projection));
//
//		for (const auto& mesh : this->ptrMeshManager->meshes) {
//			glBindVertexArray(mesh.VAO);
//
//			// bind texture 
//			glActiveTexture(GL_TEXTURE0);
//			glBindTexture(GL_TEXTURE_2D, mesh.textureIndex);
//			glUniform1i(textureLoc, 0);
//
//			// render the mesh
//			glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
//
//			glBindVertexArray(0);
//		}
//	}
//};
//
//MeshManager meshManager = MeshManager(); 
//Camera camera = Camera(&meshManager);



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

	gameEngine.CreateAndAddGameObject(
		objPath,
		texturePath
	);

	TerrainManager* ptrTerrainObj = new TerrainManager();  
	gameEngine.DirectlyAddGameObject(ptrTerrainObj);
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