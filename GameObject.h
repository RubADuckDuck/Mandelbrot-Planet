#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Mesh.h"

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