#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "ecs.hpp"
#include "position.hpp"

struct LightSource
{
  glm::float32 power;
  glm::vec3 color;
  glm::mat4 worldToLightSpace;
  GLuint depthMapID;
  GLsizei depthMapWidth;
  GLsizei depthMapHeight;
};

glm::float32 localLightInfluence(ecs::Entity lightSource, Position localPosition)
{
  if(not lightSource.hasComponents<LightSource>())
  {
    return 0.0;
  }
  if(lightSource.hasComponents<Position>())
  {
    glm::float32 distance = glm::distance(localPosition.coordinates, lightSource.getComponent<Position>().coordinates);
    return lightSource.getComponent<LightSource>().power/pow(distance, 2.0);
  }
  else
  {
    //TODO
    return 0.0;
  }
}
