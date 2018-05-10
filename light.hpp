#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ecs.hpp"
#include "position.hpp"
#include "shader.hpp"

struct Light
{
  glm::float32 power;
  glm::vec3 color;

  inline static GLuint depthMapProgramID = -1;
  inline static GLuint modelToWorld_UniformLocation = -1;
  inline static GLuint worldToProjection_UniformLocation = -1;
  static void init()
  {
    Light::depthMapProgramID = compileShaders("shader/depthMap.vert", "shader/depthMap.frag");
    Light::modelToWorld_UniformLocation = glGetUniformLocation(Light::depthMapProgramID, "modelToWorld");
    Light::worldToProjection_UniformLocation = glGetUniformLocation(Light::depthMapProgramID, "worldToProjection");
  }

  static glm::float32 localLightInfluence(ecs::Entity light, Position localPosition)
  {
    if(not light.hasComponents<Light>())
    {
      return 0.0;
    }
    if(light.hasComponents<Position>())
    {
      glm::float32 distance = glm::distance(localPosition.coordinates, light.getComponent<Position>().coordinates);
      return light.getComponent<Light>().power/pow(distance, 2.0);
    }
    else
    {
      //TODO
      return 0.0;
    }
  }
};
