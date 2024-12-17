#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"
#include "utils.h"

//class Animation; 

class Texture; 

class Transform; 

class CameraObject;


class GameObject {
public: 
	GeneralMesh* ptrModel;
	Texture* ptrTexture;  
	//Animation* ptrAnimation;  
	Transform* ptrTransform;  
	GLuint shaderProgram; 

	glm::mat4 GetModelMatrixFromTransform();


	void SetMesh(GeneralMesh* ptrModel) { this->ptrModel = ptrModel; }
	void SetTexture(Texture* ptrTexture) { this->ptrTexture = ptrTexture; }
	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
	void SetTransform(Transform* ptrTransform) { this->ptrTransform = ptrTransform; }
	void SetShaderProgram(GLuint arg_shaderProgram) { this->shaderProgram = arg_shaderProgram; }

	virtual void Update() {};
	virtual void DrawGameObject(CameraObject& cameraObj); 
	virtual void onEvent(const std::string& message) {};
};


class CameraObject : GameObject{
public: 
	bool showCamera; 
	glm::mat4 viewProjectionMatrix; 

	// Default Constructor
	CameraObject()
		: showCamera(true) // Initialize showCamera to true by default
	{
		InitializeCamera();
	}

	// Function to set the viewProjectionMatrix
	void SetViewProjMat(float fov = 45.0f, float aspectRatio = 800.0f / 600.0f, float nearPlane = 0.1f, float farPlane = 100.0f) {
		// Define camera parameters
		glm::vec3 position(0.0f, 10.0f, 10.0f); // High angle position
		glm::vec3 target(0.0f, 0.0f, 0.0f);      // Looking at origin
		glm::vec3 up(0.0f, 1.0f, 0.0f);          // Up vector

		// Create view matrix
		glm::mat4 view = glm::lookAt(position, target, up);

		// Create projection matrix
		glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

		// Combine view and projection matrices
		viewProjectionMatrix = projection * view;
	}

	// Getter for viewProjectionMatrix
	const glm::mat4& GetViewProjMat() const {
		return viewProjectionMatrix;
	}

	void SetViewProjMat() {};
	glm::mat4& GetviewProjMat() { return viewProjectionMatrix; }
	glm::vec3& GetGlobalCameraPosition() { 
		glm::vec3 temp = glm::vec3(1); 
		return temp;
	};

	void DrawGameObject(CameraObject& cameraObj) override {
		if (this->showCamera && this != &cameraObj) {
			GameObject::DrawGameObject(cameraObj);
		}
		return;
	}
private:
	// Method to initialize viewProjectionMatrix
	void InitializeCamera() {
		// Define camera parameters
		glm::vec3 position(0.0f, 10.0f, 10.0f); // High angle position
		glm::vec3 target(0.0f, 0.0f, 0.0f);      // Looking at origin
		glm::vec3 up(0.0f, 1.0f, 0.0f);          // Up vector

		// Create view matrix using glm::lookAt
		glm::mat4 view = glm::lookAt(position, target, up);

		// Define projection parameters
		float fov = 45.0f;                       // Field of view in degrees
		float aspectRatio = 800.0f / 600.0f;     // Aspect ratio (adjust as needed)
		float nearPlane = 0.1f;                  // Near clipping plane
		float farPlane = 1000.0f;                  // Far clipping plane

		// Create projection matrix using glm::perspective
		glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

		std::cout << "==============(Camera's View and Projection Matrix)==============" << std::endl;
		std::cout << "View Matrix: " << std::endl << view << std::endl; 
		std::cout << "Proj Matrix: " << std::endl << projection << std::endl;
		std::cout << "==================================================================" << std::endl;

		// Combine view and projection matrices
		viewProjectionMatrix = projection * view; 
	}
};