#pragma once

#include <glm/glm.hpp>

#include "position.hpp"

namespace Camera
{
  inline Position position;
  inline glm::vec3 viewDirection;
  inline glm::vec3 cameraUp;
  inline glm::float32 fieldOfView; // degrees
}
