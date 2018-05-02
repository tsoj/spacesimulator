#pragma once

#include <iostream>

#include <GL/glew.h>

#include "lodepng.hpp"

struct Texture
{
	Texture(uint32_t width, uint32_t height, std::vector<unsigned char> image) :
		width(width), height(height), image(image)
	{}
	uint32_t width, height;
	std::vector<unsigned char> image;

  static Texture generateTexture(const char* filePath)
  {
  	std::vector<unsigned char> image;
    unsigned int width, height;
    auto error = lodepng::decode(image, width, height, filePath);
    if(error)
  	{
  		throw std::runtime_error("Can't load image ["+std::string(filePath)+"]: decoder error " + std::to_string(error) + ": " + lodepng_error_text(error));
  	}

  	return Texture(width, height, image);
  }

  GLuint loadGlTexture()
  {
  	GLuint textureID;
  	glGenTextures(1, &textureID);
  	glBindTexture(GL_TEXTURE_2D, textureID);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->image.data());
  	//enable mipmapping
  	glGenerateMipmap(GL_TEXTURE_2D);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.3f);
  	return textureID;
  }
};
const Texture defaultTexture = Texture(1, 1, {255, 255, 255, 255});
const Texture defaultNormalMap = Texture(1, 1, {128, 128, 255, 255});
