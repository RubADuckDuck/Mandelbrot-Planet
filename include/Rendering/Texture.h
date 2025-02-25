#pragma once
#include <glad/glad.h>
#include <iostream>  

class Texture {
public: 
	GLuint textureIndex; 

	
	void SetTextureIndex(GLuint textureIndex);
	GLuint GetTextureIndex();

	void LoadandSetTextureIndexFromPath(const std::string& path, bool retry=false);
};
