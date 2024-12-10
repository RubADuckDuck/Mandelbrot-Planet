#include "Texture.h"


void Texture::LoadandSetTextureIndexFromPath(const std::string& path) {
	GLuint resTextureIndex;
	glGenTextures(1, &resTextureIndex);
	glBindTexture(GL_TEXTURE_2D, resTextureIndex);

	// Load texture using your favorite library (e.g., stb_image)
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cerr << "Failed to load texture: " << path << std::endl;
	}
	stbi_image_free(data);

	this->SetTextureIndex(resTextureIndex);
}