#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Mesh; 
class Texture; 
class Animation; 

class Transform; 


class GameObject {
public: 
	Mesh* mesh;  
	Texture* texture;  
	Animation* animation;  
	Transform* transform;  

	glm::mat4 GetModelMatrixFromTransform();
	void DrawGameObject(glm::mat4 view, glm::mat4 proj); 
};