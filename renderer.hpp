#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "io.hpp"
#include "vertex.hpp"

struct Model
{
  struct Material
  {
    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    glm::float_t transparency;
    glm::float_t shininess;
  };
  Material material;

  std::vector<Vertex> vertices = std::vector<Vertex>();

  GLuint vertexBufferID;
  GLuint vertexArrayObjectIDs;
  GLuint programID;
  GLuint textureID;
  GLuint normalMapID;
};

struct Renderable
{
  std::vector<Model> models = std::vector<Model>();
};
