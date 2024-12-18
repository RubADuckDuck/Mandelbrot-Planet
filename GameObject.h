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

	

	virtual ~GameObject();

	void SetMesh(GeneralMesh* ptrModel);
	void SetTexture(Texture* ptrTexture);
	//void SetAnimation(Animation* ptrAnimation) { this->ptrTexture = ptrTexture; }
	void SetTransform(Transform* ptrTransform);

	virtual void Update();
	virtual glm::mat4 GetModelMatrixFromTransform();
	virtual void DrawGameObject(CameraObject& cameraObj); 
	virtual void onEvent(const std::string& message);
}; 

class RotatingGameObject : public GameObject {
public: 
	float radian = 0; 
	glm::vec3 axis = glm::vec3(0, 1, 0); 

	~RotatingGameObject();

	void Update() override;
};


class CameraObject : GameObject{
public: 
	bool showCamera; 
	glm::mat4 viewProjectionMatrix; 

	// Default Constructor
	CameraObject();

	// Function to set the viewProjectionMatrix
	void SetViewProjMat(float fov, float aspectRatio, float nearPlane, float farPlane);

	// Getter for viewProjectionMatrix
	const glm::mat4& GetViewProjMat() const;

	void SetViewProjMat();
	glm::mat4& GetviewProjMat();
	glm::vec3& GetGlobalCameraPosition();

	void DrawGameObject(CameraObject& cameraObj) override;
private:
	// Method to initialize viewProjectionMatrix
	void InitializeCamera();
};