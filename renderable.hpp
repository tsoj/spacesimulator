#pragma once

#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "io.hpp"
#include "vertex.hpp"
#include "loadObj.hpp"
#include "shader.hpp"
#include "texture.hpp"

struct Model
{
  glm::vec3 ambientColor;
  glm::vec3 diffuseColor;
  glm::vec3 specularColor;
  glm::float_t transparency;
  glm::float_t shininess;

  GLuint ambientTextureID;
  GLuint diffuseTextureID;
  GLuint specularColorTextureID;
  GLuint specularHighlightTextureID;
  GLuint alphaTextureID;
  GLuint normalMapID;

  std::vector<Vertex> vertices = std::vector<Vertex>();

  GLuint vertexBufferID;
  GLuint vertexArrayObjectID;
  GLuint programID;

  GLuint modelToWorld_UniformLocation;
  GLuint worldToProjection_UniformLocation;
  GLuint cameraPosition_UniformLocation;
  GLuint numLights_UniformLocation;
  GLuint worldToLight_UniformLocation;
  GLuint depthMap_UniformLocation;
  GLuint worldToLight1_UniformLocation;
  GLuint depthMap1_UniformLocation;
  GLuint lightPosition_UniformLocation;
  GLuint lightPower_UniformLocation;
  GLuint lightColor_UniformLocation;
  GLuint ambientColor_UniformLocation;
  GLuint diffuseColor_UniformLocation;
  GLuint specularColor_UniformLocation;
  GLuint transparency_UniformLocation;
  GLuint shininess_UniformLocation;
  GLuint diffuseTexture_UniformLocation;
  GLuint normalMap_UniformLocation;
};

struct Renderable
{
  void init(std::string modelFile, std::string vertShaderFile, std::string fragShaderFile)
  {
    Obj obj = Obj::loadObj(modelFile);
    models.clear();
    models.resize(obj.objects.size());
    for(size_t i = 0; i<obj.objects.size(); i++)
    {
      models[i].programID = compileShaders(vertShaderFile, fragShaderFile);

      models[i].ambientColor = obj.objects[i].mtl.ambientColor;
      models[i].diffuseColor = obj.objects[i].mtl.diffuseColor;
      models[i].specularColor = obj.objects[i].mtl.specularColor;
      models[i].transparency = obj.objects[i].mtl.transparency;
      models[i].shininess = obj.objects[i].mtl.shininess;

      //TODO:models[i].ambientTextureID = Texture::generateTexture(obj.objects[i].mtl.ambientTexture.c_str()).loadGlTexture();
      models[i].diffuseTextureID = Texture::generateTexture(obj.objects[i].mtl.diffuseTexture.c_str()).loadGlTexture();
      //TODO:models[i].specularColorTextureID = Texture::generateTexture(obj.objects[i].mtl.specularColorTexture.c_str()).loadGlTexture();
      //TODO:models[i].specularHighlightTextureID = Texture::generateTexture(obj.objects[i].mtl.specularHighlightTexture.c_str()).loadGlTexture();
      //TODO:models[i].alphaTextureID = Texture::generateTexture(obj.objects[i].mtl.alphaTexture.c_str()).loadGlTexture();
      models[i].normalMapID = Texture::generateTexture(obj.objects[i].mtl.normalMap.c_str()).loadGlTexture();

      models[i].vertices = std::vector<Vertex>(obj.objects[i].vertices);

			glGenVertexArrays(1, &models[i].vertexArrayObjectID);
			glBindVertexArray(models[i].vertexArrayObjectID);

			glGenBuffers(1, &models[i].vertexBufferID);
			glBindBuffer(GL_ARRAY_BUFFER, models[i].vertexBufferID);

			glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * models[i].vertices.size() , models[i].vertices.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(
				0,                  											// attribute
				3,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, position)					// array buffer offset
			);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(
				1,                  											// attribute
				3,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, normal)						// array buffer offset
			);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(
				2,                  											// attribute
				3,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, tangent)					// array buffer offset
			);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(
				3,                  											// attribute
				2,     																		// size
				GL_FLOAT,          												// type
				GL_FALSE,																	// normalized
				sizeof(Vertex),														// stride
				(void*)offsetof(Vertex, textureCoordinate)// array buffer offset
			);

      models[i].modelToWorld_UniformLocation = glGetUniformLocation(models[i].programID, "modelToWorld");
      models[i].worldToProjection_UniformLocation = glGetUniformLocation(models[i].programID, "worldToProjection");
      models[i].cameraPosition_UniformLocation = glGetUniformLocation(models[i].programID, "cameraPosition_WorldSpace");
      models[i].numLights_UniformLocation = glGetUniformLocation(models[i].programID, "numLights");
      models[i].worldToLight_UniformLocation = glGetUniformLocation(models[i].programID, "worldToLight");
      models[i].depthMap_UniformLocation = glGetUniformLocation(models[i].programID, "depthMap");
      models[i].worldToLight1_UniformLocation = glGetUniformLocation(models[i].programID, "worldToLight1");
      models[i].depthMap1_UniformLocation = glGetUniformLocation(models[i].programID, "depthMap1");
      models[i].lightPosition_UniformLocation = glGetUniformLocation(models[i].programID, "lightPosition_WorldSpace");
      models[i].lightPower_UniformLocation = glGetUniformLocation(models[i].programID, "lightPower");
      models[i].lightColor_UniformLocation = glGetUniformLocation(models[i].programID, "lightColor");
      models[i].ambientColor_UniformLocation = glGetUniformLocation(models[i].programID, "ambientColor");
      models[i].diffuseColor_UniformLocation = glGetUniformLocation(models[i].programID, "diffuseColor");
      models[i].specularColor_UniformLocation = glGetUniformLocation(models[i].programID, "specularColor");
      models[i].transparency_UniformLocation = glGetUniformLocation(models[i].programID, "transparency");
      models[i].shininess_UniformLocation = glGetUniformLocation(models[i].programID, "shininess");
      models[i].diffuseTexture_UniformLocation = glGetUniformLocation(models[i].programID, "diffuseTexture");
      models[i].normalMap_UniformLocation = glGetUniformLocation(models[i].programID, "normalMap");
    }
  }

  std::vector<Model> models = std::vector<Model>();
};
