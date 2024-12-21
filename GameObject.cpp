#include "GameObject.h"
#include "Mesh.h"       // Assuming Mesh class handles vertex data and drawing
#include "Texture.h"    // Assuming Texture class handles texture binding
#include "Transform.h"
#include "Animation.h"  // Assuming Animation class handles animation

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



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


GameObject::~GameObject() {
	std::cout << "GameObject Destructor" << std::endl;
}


void GameObject::SetMesh(GeneralMesh* ptrModel) { this->ptrModel = ptrModel; }
void GameObject::SetTexture(Texture* ptrTexture) { this->ptrTexture = ptrTexture; }
//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
void GameObject::SetTransform(Transform* ptrTransform) { this->ptrTransform = ptrTransform; }

void GameObject::Update() {};
glm::mat4 GameObject::GetModelMatrixFromTransform() {
	return ptrTransform->GetTransformMatrix();
}
void GameObject::DrawGameObject(CameraObject& cameraObj) {
	glm::mat4 transformMat = GetModelMatrixFromTransform();

	// draw mesh
	ptrModel->Render(cameraObj, transformMat, ptrTexture);
}
void GameObject::onEvent(const std::string& message) {};


RotatingGameObject::~RotatingGameObject() {
	std::cout << "RotatingGameObject Destructor" << std::endl;
}

void RotatingGameObject::Update() {
	ptrTransform->SetRotation(radian, axis);
	radian = radian + 0.1;
}


// Playable ---------------------------
void PlayableObject::onEvent(const std::string& message) {
	glm::vec3 curTranslation = ptrTransform->GetTranslation();
	glm::vec3 temp = glm::vec3(0);


	if (message == "w_up") {
		temp = glm::vec3(0, 0, -1);
	}
	else if (message == "s_up") {
		temp = glm::vec3(0, 0, 1);
	}
	else if (message == "d_up") {
		temp = glm::vec3(1, 0, 0);
	}
	else if (message == "a_up") {
		temp = glm::vec3(-1, 0, 0);
	}
	else {

	}
	curTranslation = curTranslation + temp;
	ptrTransform->SetTranslation(curTranslation);
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

void CameraObject::AddTarget(GameObject* targetGameObj) {
	if (targetGameObj && targetGameObj->ptrTransform) {
		targetGameObjects.push_back(targetGameObj);
		std::cout << "Added target object at: " << targetGameObj->ptrTransform->GetTranslation() << std::endl;
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

void CameraObject::Update() {
	angle = angle + 1;

	if (targetGameObjects.empty()) {
		return; // No targets, no update needed
	}

	glm::vec3 totalTranslation(0.0f);

	// Calculate the average translation of target objects
	for (const auto& targetGameObj : targetGameObjects) {
		if (targetGameObj && targetGameObj->ptrTransform) {
			totalTranslation += targetGameObj->ptrTransform->GetTranslation();
		}
	}

	glm::vec3 averageTranslation = totalTranslation / static_cast<float>(targetGameObjects.size());

	// Compute the desired camera position
	glm::vec3 desiredTargetPosition = averageTranslation;

	// Smoothly interpolate the current position towards the desired position
	target = glm::mix(target, desiredTargetPosition, 0.1f); // LERP with factor 0.1f for smoothing

	// Set target for the camera (useful for maintaining focus)
	glm::vec3 v(0.0f, 10.0f, 10.0f);
	position = target + rotateVector(v, angle); 

	// Update view-projection matrix
	SetViewProjMat();
}

void CameraObject::DrawGameObject(CameraObject& cameraObj) {
	if (this->showCamera && this != &cameraObj) {
		GameObject::DrawGameObject(cameraObj);
	}
	return;
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