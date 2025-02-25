#include "Core/GameObject.h"
#include "Rendering/Mesh.h"       // Assuming Mesh class handles vertex data and drawing
#include "Rendering/Texture.h"    // Assuming Texture class handles texture binding
#include "Core/Transform.h"
#include "Core/Animation.h"  // Assuming Animation class handles animation
#include "Core/Item.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Network/NetworkMessage.h"



//
//// Calculate the Model Matrix from the Transform object
//glm::mat4 GameObject::GetModelMatrixFromTransform() {
//	if (this->transform){
//		return this->transform->GetTranformMatrix(); // Use the transform to generate the model matrix
//	}
//	else {
//		return glm::mat4(1.0f); // Return identity matrix if transform is null
//	}
//}
//
//// Draw the GameObject
//void GameObject::DrawGameObject(glm::mat4 view, glm::mat4 proj) {
//	if (!mesh || !texture) return; // Ensure essential components are present
//
//	glm::mat4 model = GetModelMatrixFromTransform(); // Get the Model Matrix
//	glm::mat4 mvp = proj * view * model;             // Compute the MVP matrix
//
//	texture->Bind();        // Bind the texture
//	mesh->Draw(mvp);        // Pass the MVP matrix and draw the mesh
//}
//
//
//void GameObject::DrawGameObject(glm::mat4 view, glm::mat4 proj) {
//
//
//	glUseProgram(shaderProgram);
//
//	GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
//	GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
//	GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
//	GLuint textureLoc = glGetUniformLocation(shaderProgram, "texture_diffuse");
//
//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//
//	for (const auto& mesh : this->meshes) {
//		glBindVertexArray(mesh.VAO);
//
//		// bind texture 
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, mesh.textureIndex);
//		glUniform1i(textureLoc, 0);
//
//		// render the mesh
//		glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
//
//		glBindVertexArray(0);
//	}
//}
//
//}


std::string GameObject::GetName() const { return "GameObject"; }

// client & server 

uint8_t GameObject::GetTypeID() { return 0; }

void GameObject::log(LogLevel level, std::string text) {
	LOG(level, GetName() + "::" + text);
}

GameObject::~GameObject() {
	std::cout << "GameObject Destructor" << std::endl;
}


void GameObject::SetTransform(Transform* ptrTransform) {
	this->ptrNodeTransform_ = ptrTransform; 
}



void GameObject::Update() {
};

void GameObject::Update(float deltaTime) {
	return; 
}


// CameraObject -------------------------------------------------------

// Default Constructor
CameraObject::CameraObject()
	: showCamera(true) // Initialize showCamera to true by default
{
	InitializeCamera();
}

// Function to set the viewProjectionMatrix
void CameraObject::SetViewProjMat() {
	// Create view matrix
	glm::mat4 view = glm::lookAt(position, target, up);

	// Create projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

	// Combine view and projection matrices
	viewProjectionMatrix = projection * view;
}

// Getter for viewProjectionMatrix
const glm::mat4& CameraObject::GetViewProjMat() const {
	return viewProjectionMatrix;
}

glm::mat4& CameraObject::GetviewProjMat() { 
	return viewProjectionMatrix; 
}
glm::vec3& CameraObject::GetGlobalCameraPosition() {
	glm::vec3 temp = glm::vec3(1);
	return temp;
};

void CameraObject::AddTarget(GameObject* targetGameObj) { // todo: ptr Transforms are not used
	if (targetGameObj && targetGameObj->modelTransformMat_!=glm::mat4(0.0f)) {
		targetGameObjects.push_back(targetGameObj);
		std::cout << "Added target object at: " << targetGameObj->modelTransformMat_ << std::endl;
	}
	else {
		std::cerr << "Invalid target object or missing transform!" << std::endl;
	}
}

glm::vec3 rotateVector(const glm::vec3& v, float theta) {
	// Create a rotation matrix around the Y-axis
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));

	// Convert the vector to a 4D vector for matrix multiplication
	glm::vec4 rotatedVector = rotationMatrix * glm::vec4(v, 1.0f);

	// Return the rotated vector (convert back to 3D)
	return glm::vec3(rotatedVector);
}

void CameraObject::Update() { // todo: ptrTransforms are not used anymore
	bool doUpdate = false; 

	if (doUpdate) {
		glm::vec3 prevPosition = position; 
		glm::vec3 prevTarget = target; 
		glm::vec3 prevUp = up; 


		int distance = 50;
		// Define camera parameters
		position = glm::vec3(5.0f, distance * 1.0f,- distance * 1.0f); // High angle position
		target = glm::vec3(0.0f, 0.0f, 0.0f);      // Looking at origin
		up = glm::vec3(0.0f, 1.0f, 0.0f);          // Up vector

		glm::mat4 totalTransform(0.0f);

		// Calculate the average translation of target objects
		for (const auto& targetGameObj : targetGameObjects) {
			if (targetGameObj && targetGameObj->modelTransformMat_ != glm::mat4(0.0f)) {
				totalTransform += targetGameObj->modelTransformMat_;
			}
		}

		glm::mat4 averageTransform = totalTransform / static_cast<float>(targetGameObjects.size()); 

		position = glm::vec3(averageTransform * glm::vec4(position, 1));
		target = glm::vec3(averageTransform * glm::vec4(target, 1)); 
		

		// Extract the rotation component from the average transformation matrix
		glm::mat3 rotationMatrix = glm::mat3(averageTransform);

		// Apply the rotation to the up vector
		up = glm::normalize(rotationMatrix * up);

		// Lerp factor (t), between 0 and 1
		float t = 0.01f; // Example value, update this based on your logic (e.g., time)

		// Apply linear interpolation (lerp) for smooth transitions
		position = glm::mix(prevPosition, position, t);
		target = glm::mix(prevTarget, target, t);
		up = glm::mix(prevUp, up, t);
	}

	// Update view-projection matrix
	SetViewProjMat();
}

// Method to initialize viewProjectionMatrix
void CameraObject::InitializeCamera() {
	// Define camera parameters
	position = glm::vec3(0.0f, 10.0f, 10.0f); // High angle position
	target = glm::vec3(0.0f, 0.0f, 0.0f);      // Looking at origin
	up = glm::vec3(0.0f, 1.0f, 0.0f);          // Up vector

	// Create view matrix using glm::lookAt
	glm::mat4 view = glm::lookAt(position, target, up);

	// Define projection parameters
	fov = 45.0f;                       // Field of view in degrees
	aspectRatio = 800.0f / 600.0f;     // Aspect ratio (adjust as needed)
	nearPlane = 0.1f;                  // Near clipping plane
	farPlane = 1000.0f;                  // Far clipping plane

	// Create projection matrix using glm::perspective
	glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

	std::cout << "==============(Camera's View and Projection Matrix)==============" << std::endl;
	std::cout << "View Matrix: " << std::endl << view << std::endl;
	std::cout << "Proj Matrix: " << std::endl << projection << std::endl;
	std::cout << "==================================================================" << std::endl;

	// Combine view and projection matrices
	viewProjectionMatrix = projection * view;
}