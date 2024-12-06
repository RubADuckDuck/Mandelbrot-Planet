#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Transform {
public: 
	glm::mat4 translation; 
	glm::mat4 rotation; 
	glm::mat4 scale; 

	Transform();

	glm::mat4 GetTranformMatrix(); 
	void SetTranslation(glm::vec3 x); 
	void SetRotation(float theta, glm::vec3 axis); 
	void SetScale(glm::vec3 x);
};