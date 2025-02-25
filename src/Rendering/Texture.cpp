#include "Rendering/Texture.h"
#include "Utils/LOG.h"
#include "ThirdParty/stb_image.h"

void Texture::SetTextureIndex(GLuint textureIndex) { this->textureIndex = textureIndex; }

GLuint Texture::GetTextureIndex() { return textureIndex; }

void Texture::LoadandSetTextureIndexFromPath(const std::string& path, bool retry) {
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

		LOG(LOG_INFO, "Successfully loaded texture from:" + path);
	}
	else {
		LOG(LOG_ERROR, "Failed to load texture from:" + path); 
		if (!retry) {
			this->LoadandSetTextureIndexFromPath("E:/repos/[DuckFishing]/model/texture/Error.png", true);
		}
		
		return; 
	}
	stbi_image_free(data);

	this->SetTextureIndex(resTextureIndex);
}