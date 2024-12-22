#pragma once
#include <glad/glad.h>
#include <iostream>  
#include "stb_image.h"

class Texture {
public: 
	GLuint textureIndex; 

	
	void SetTextureIndex(GLuint textureIndex) { this->textureIndex = textureIndex; }
	GLuint GetTextureIndex() { return textureIndex; } 

	void LoadandSetTextureIndexFromPath(const std::string& path, bool retry=false);
};
