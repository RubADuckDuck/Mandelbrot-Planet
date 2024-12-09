#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"

//class Texture; 
//class Animation; 

class Transform; 

class CameraObject;


class GameObject {
public: 
	GeneralMesh* ptrModel;
	//Texture* ptrTexture;  
	//Animation* ptrAnimation;  
	Transform* ptrTransform;  

	glm::mat4 GetModelMatrixFromTransform();


	void SetMesh(GeneralMesh* ptrModel) { this->ptrModel = ptrModel; }
	//void SetTexture(Texture* ptrTexture) { this->ptrTexture = ptrTexture; }
	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
	void SetTransform(Transform* ptrTransform) { this->ptrTransform = ptrTransform; }

	virtual void DrawGameObject(CameraObject& cameraObj);
};


class CameraObject : GameObject{
public: 
	bool showCamera; 
	glm::mat4 viewProjectionMatrix; 

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
};