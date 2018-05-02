#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ecs.hpp"
#include "position.hpp"
struct Light
{
  glm::float32 power;
  glm::vec3 color;
  glm::mat4 worldToLightSpace;
  GLuint depthMapID;
  GLsizei depthMapWidth;
  GLsizei depthMapHeight;

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
