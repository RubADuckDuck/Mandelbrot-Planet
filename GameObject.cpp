#include "GameObject.h"
#include "Mesh.h"       // Assuming Mesh class handles vertex data and drawing
#include "Texture.h"    // Assuming Texture class handles texture binding
#include "Transform.h"
#include "Animation.h"  // Assuming Animation class handles animation
#include "Item.h"

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
void GameObject::onEvent(InteractionInfo* interactionInfo) {
	GameObject* who = interactionInfo->who; 
	Item* item = interactionInfo->item;
	int y = interactionInfo->yCoord; 
	int x = interactionInfo->xCoord; 

	if (!item) {
		// todo: log who did it
		LOG(LOG_INFO, "Null item Dropped at: (" + std::to_string(y) + ", " + std::to_string(x) + ")");
	}
}

RotatingGameObject::~RotatingGameObject() {
	std::cout << "RotatingGameObject Destructor" << std::endl;
}

void RotatingGameObject::Update() {
	ptrTransform->SetRotation(radian, axis);
	radian = radian + 0.1;
}

// Playable ---------------------------
void PlayableObject::onEvent(const std::string& message) {
	int curDirectionInt = 0; 

	if (message == "s_up") {
		direction = Direction::DOWN;
		// yCoord = yCoord + 1;

		curDirectionInt = static_cast<int>(direction) + orientation;
		direction = static_cast<Direction>(curDirectionInt);
		RequestWalk();
	}
	else if (message == "w_up") {
		direction = Direction::UP;
		// yCoord = yCoord - 1;

		curDirectionInt = static_cast<int>(direction) + orientation;
		direction = static_cast<Direction>(curDirectionInt);
		RequestWalk();
	}
	else if (message == "d_up") {
		direction = Direction::RIGHT;
		// xCoord = xCoord + 1;

		curDirectionInt = static_cast<int>(direction) + orientation;
		direction = static_cast<Direction>(curDirectionInt);
		RequestWalk();
	}
	else if (message == "a_up") {
		direction = Direction::LEFT;
		// xCoord = xCoord - 1;

		curDirectionInt = static_cast<int>(direction) + orientation;
		direction = static_cast<Direction>(curDirectionInt);
		RequestWalk();
	}
	else if (message== "space_up") {
		LOG(LOG_INFO, "Publishing Item from Player");

		DropItem();
	}



	if (yCoord < 0) {
		yCoord = 0;
	} 
	if (xCoord < 0) {
		xCoord = 0;
	}
}

void PlayableObject::DrawGameObject(CameraObject& cameraObj) {
	GameObject::DrawGameObject(cameraObj); // draw this object
	if (heldItem) {
		heldItem->DrawGameObject(cameraObj); // draw held object
	}
}

void PlayableObject::DropItem() {
	Item* temp = heldItem; 
	heldItem = nullptr; 

	// remember to delete after use 
	InteractionInfo* newInteraction = new InteractionInfo(); 
	newInteraction->item = temp;
	newInteraction->who = this; 
	newInteraction->yCoord = this->yCoord; 
	newInteraction->xCoord = this->xCoord;

	PublishItem(newInteraction);

	// this will have to be fixed once events are excuted at different times but for now
	delete newInteraction;
}

void PlayableObject::RequestWalk() {
	// We don't drop item... yet
	
	InteractionInfo* newInteraction = new InteractionInfo(); 
	newInteraction->item = heldItem; 
	newInteraction->who = this; 
	newInteraction->yCoord = this->yCoord; 
	newInteraction->xCoord = this->xCoord; 
	newInteraction->goingWhere = this->direction; 

	PublishItem(newInteraction); 

	delete newInteraction;
}

void PlayableObject::Walk() {
	// Assume the object has xCoord and yCoord representing its position
	switch (this->direction) {
	case Direction::UP:
		yCoord--; // Move up (decrease y)
		break;
	case Direction::DOWN:
		yCoord++; // Move down (increase y)
		break;
	case Direction::LEFT:
		xCoord--; // Move left (decrease x)
		break;
	case Direction::RIGHT:
		xCoord++; // Move right (increase x)
		break;
	case Direction::IDLE:
		// Do nothing, the object remains in place
		break;
	default:
		// Handle unexpected direction (optional)
		break;
	}
}

void PlayableObject::PickUpItem(Item* item) {
	heldItem = item;
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
	
	{
		glm::vec3 prevPosition = position; 
		glm::vec3 prevTarget = target; 
		glm::vec3 prevUp = up; 

		// Define camera parameters
		position = glm::vec3(5.0f, 20.0f, 20.0f); // High angle position
		target = glm::vec3(0.0f, 0.0f, 0.0f);      // Looking at origin
		up = glm::vec3(0.0f, 1.0f, 0.0f);          // Up vector

		glm::mat4 totalTransform(0.0f);

		// Calculate the average translation of target objects
		for (const auto& targetGameObj : targetGameObjects) {
			if (targetGameObj && targetGameObj->ptrTransform) {
				totalTransform += targetGameObj->ptrTransform->GetTransformMatrix();
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