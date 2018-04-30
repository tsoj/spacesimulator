#pragma once

#include <glm/glm.hpp>

#include "position.hpp"

namespace Camera
{
  Position position;
  glm::vec3 viewDirection;
  glm::vec3 cameraUp;
  glm::float32 fieldOfView; // degrees
}
