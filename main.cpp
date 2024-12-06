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
#include "stb_image.h"
#include "Mesh.h"

// Globals 
int gScreenHeight = 480;
int gScreenWidth = 640;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;


bool gQuit = false;

// OpenGL Globals
GLuint shaderProgram;






class MeshManager {
public: 
	std::vector<Mesh> meshes; 

	MeshManager() {
		std::cout << "MeshManager Created" << std::endl; 
	} 

	~MeshManager() { 
		Mesh temp; 
		for (int i = 0; i < this->meshes.size(); i++) {
			meshes.pop_back(); 
		}

		std::cout << "MeshManager Destroyed" << std::endl;
	}

	void AddMesh(std::string path) {
		
		Mesh newMesh = this->LoadModel(path);  

		meshes.push_back(newMesh); 

	}

	GLuint LoadTexture(const std::string& path) {
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Load texture using your favorite library (e.g., stb_image)
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cerr << "Failed to load texture: " << path << std::endl;
		}
		stbi_image_free(data);

		return textureID;
	}

	GLuint LoadMaterials(const aiScene* scene) {
		for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
			aiMaterial* material = scene->mMaterials[i];

			aiString texPath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
				std::string textureFile = texPath.C_Str();

				// Call the LoadTexture function here
				GLuint textureID = LoadTexture(textureFile);

				return textureID; 
			}
		}
	} 

	Mesh LoadModel(const std::string& path) {
		Mesh newMesh = Mesh(); 

		GLuint VAO; 
		GLuint VBO; 
		GLuint EBO; 

		GLuint textureID; 
		int indexCount = 0; // bit confused if it's number of verts or faces 

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			path,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_FlipUVs |  // Corrects UV mapping if needed
			aiProcess_CalcTangentSpace
		);

		if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
			std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
			exit(1);
		}

		std::vector<float> vertices;
		std::vector<unsigned int> indices;

		// Process only the first mesh (for simplicity)
		aiMesh* mesh = scene->mMeshes[0];

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			// Position
			aiVector3D vertex = mesh->mVertices[i];
			vertices.push_back(vertex.x);
			vertices.push_back(vertex.y);
			vertices.push_back(vertex.z);

			// Normal
			aiVector3D normal = mesh->mNormals[i];
			vertices.push_back(normal.x);
			vertices.push_back(normal.y);
			vertices.push_back(normal.z);

			// Texture Coordinates
			if (mesh->mTextureCoords[0]) { // Check if the mesh has texture coordinates
				aiVector3D texCoord = mesh->mTextureCoords[0][i];
				vertices.push_back(texCoord.x);
				vertices.push_back(texCoord.y); 
				std::cout << texCoord.x << "," << texCoord.y << std::endl;
			}
			else {
				vertices.push_back(0.0f); // Default texture coordinate
				vertices.push_back(0.0f);
				std::cout << "No UV value" << std::endl;
			}
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
				indexCount++;
			} 

			
		}

		// Generate buffers
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // Position
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Normal
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Texture Coordinates
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);

		newMesh.VAO = VAO; 
		newMesh.VBO = VBO; 
		newMesh.EBO = EBO; 

		newMesh.indexCount = indexCount; 

		// load texture from material
		textureID = LoadMaterials(scene); 

		newMesh.textureID = textureID; 

		return newMesh; 
	}

	void DrawModel(float deltaTime) {

		static float rotationAngle = 0.0f;
		rotationAngle += 50.0f * deltaTime;

		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gScreenWidth / gScreenHeight, 0.1f, 100.0f);


		glUseProgram(shaderProgram);

		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
		GLuint textureLoc = glGetUniformLocation(shaderProgram, "texture_diffuse");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		for (const auto& mesh : this->meshes) {
			glBindVertexArray(mesh.VAO);

			// bind texture 
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.textureID);
			glUniform1i(textureLoc, 0);

			// render the mesh
			glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
		}
	}
};

class Camera {
public:
	MeshManager* ptrMeshManager;

	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)gScreenWidth / gScreenHeight, 0.1f, 100.0f);

	Camera() {
		std::cout << "Camera initiated" << std::endl;
	}

	Camera(MeshManager* ptrMeshMng) {
		this->ptrMeshManager = ptrMeshMng;
	}

	~Camera() { 
		std::cout << "Camera Destroyed" << std::endl;
		// delete& ptrMeshManager;
	}

	void SetMeshManager(MeshManager* ptrMeshMng) {
		this->ptrMeshManager = ptrMeshMng;
	}

	MeshManager* GetManager(void) {
		return this->ptrMeshManager;
	}

	void DrawModel(float deltaTime) {

		static float rotationAngle = 0.0f;
		rotationAngle += 50.0f * deltaTime;

		glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));


		glUseProgram(shaderProgram);

		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
		GLuint textureLoc = glGetUniformLocation(shaderProgram, "texture_diffuse");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(this->view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(this->projection));

		for (const auto& mesh : this->ptrMeshManager->meshes) {
			glBindVertexArray(mesh.VAO);

			// bind texture 
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh.textureID);
			glUniform1i(textureLoc, 0);

			// render the mesh
			glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);

			glBindVertexArray(0);
		}
	}
};

MeshManager meshManager = MeshManager(); 
Camera camera = Camera(&meshManager);


// Shader sources
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords; 

out vec2 TexCoords; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() { 

	// Pass texture coordinates to the fragment shader
    TexCoords = aTexCoords; 

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse;

void main() {
    FragColor = texture(texture_diffuse, TexCoords);
	// Visualize the texture coordinates as colors
    // FragColor = vec4(1, 1, 0.0, 1.0);
}
)"; 

GLuint CompileShader(GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	// Check for compilation errors
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "Shader Compilation Error:\n" << infoLog << std::endl;
		exit(1);
	}
	return shader;
}

GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource) {
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Check for linking errors
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cout << "Shader Linking Error:\n" << infoLog << std::endl;
		exit(1);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
} 










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

	shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

	

	GetOpenGLVersionInfo(); 

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// load meshmanager and load model
	std::string objPath = "E:\\repos\\[DuckFishing]\\model\\duck.obj";
	meshManager.AddMesh(objPath);
}



void Input() {
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			std::cout << "Goodbye" << std::endl;

			gQuit = true;
		}
	}
}


void PreDraw() {

}

void Draw() {

}

void MainLoop() { 
	while (!gQuit) {
		Input(); 

		static Uint64 previousTicks = SDL_GetPerformanceCounter();
		Uint64 currentTicks = SDL_GetPerformanceCounter();
		float deltaTime = (float)(currentTicks - previousTicks) / SDL_GetPerformanceFrequency();
		previousTicks = currentTicks;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/*PreDraw();
		Draw();*/ 

		camera.DrawModel(deltaTime); 

		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
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