#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "renderable.hpp"
#include "light.hpp"
#include "ecs.hpp"
#include "camera.hpp"
#include "position.hpp"
#include "orientation.hpp"
#include "window.hpp"


const int MAX_NUM_LIGHTS = 10;

int getMostInfluentialLights(Position localPosition, ecs::Entity influentialLights[MAX_NUM_LIGHTS])
{
  static glm::float32 localLightPower[MAX_NUM_LIGHTS];
  for(auto& localLightPower : localLightPower)
  {
    localLightPower = 0.0;
  }

  static void* tmp = malloc(MAX_NUM_LIGHTS*(sizeof(ecs::Entity)>=sizeof(glm::float32) ? sizeof(ecs::Entity) : sizeof(glm::float32)));
  int numLights = 0;
  for(auto entity : ecs::Iterator<Light>())
  {
    glm::float32 currentLocalLightPower = Light::localLightInfluence(entity, localPosition);
    for(size_t i = 0; i<MAX_NUM_LIGHTS; i++)
    {
      if(localLightPower[i]<currentLocalLightPower)
      {
        if(i<MAX_NUM_LIGHTS-1)
        {
          memcpy(tmp, &localLightPower[i], (MAX_NUM_LIGHTS-i-1)*sizeof(glm::float32));
          memcpy(&localLightPower[i+1], tmp, (MAX_NUM_LIGHTS-i-1)*sizeof(glm::float32));

          memcpy(tmp, &influentialLights[i], (MAX_NUM_LIGHTS-i-1)*sizeof(ecs::Entity));
          memcpy(&influentialLights[i+1], tmp, (MAX_NUM_LIGHTS-i-1)*sizeof(ecs::Entity));
        }
        localLightPower[i] = currentLocalLightPower;
        influentialLights[i] = entity;
        numLights++;
        break;
      }
    }
  }
  return numLights>=MAX_NUM_LIGHTS?MAX_NUM_LIGHTS:numLights;
}

int getLightData(
  Position localPosition,
  glm::vec3 lightPosition[MAX_NUM_LIGHTS],
  glm::float32 lightPower[MAX_NUM_LIGHTS],
  glm::vec3 lightColor[MAX_NUM_LIGHTS],
  glm::mat4 worldToLightSpace[MAX_NUM_LIGHTS],
  GLuint depthMapID[MAX_NUM_LIGHTS]
)
{
  static ecs::Entity influentialLights[MAX_NUM_LIGHTS];
  int numLights = getMostInfluentialLights(localPosition, influentialLights);

  for(int i = 0; i<numLights; i++)
  {
    ecs::Entity& entity = influentialLights[i];
    if(entity.hasComponents<Position>())
    {
      lightPosition[i] = entity.getComponent<Position>().coordinates;
    }
    else
    {
      lightPosition[i] = Camera::position.coordinates;
    }

    lightPower[i] = entity.getComponent<Light>().power;
    lightColor[i] = entity.getComponent<Light>().color;
  }
  return numLights;
}

void renderer()
{

  static glm::vec3 lightPosition[MAX_NUM_LIGHTS];
  static glm::float32 lightPower[MAX_NUM_LIGHTS];
  static glm::vec3 lightColor[MAX_NUM_LIGHTS];
  static glm::mat4 worldToLightSpace[MAX_NUM_LIGHTS];
  static GLuint depthMapID[MAX_NUM_LIGHTS];

  int numLights = getLightData(
    Camera::position,
    lightPosition, lightPower, lightColor,
    worldToLightSpace, depthMapID
  );

  int width, height;
  glfwGetFramebufferSize(Window::window, &width, &height);
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 worldToProjection =
    glm::perspective(glm::radians(Camera::fieldOfView), GLfloat(width)/GLfloat(height), 0.1f, 100.0f) *
    glm::lookAt(Camera::position.coordinates, Camera::position.coordinates + Camera::viewDirection, Camera::cameraUp);

  for(auto entity : ecs::Iterator<Renderable>())
  {
    glm::mat4 modelRotation;
    if(entity.hasComponents<Orientation>())
    {
      modelRotation = entity.getComponent<Orientation>().rotationMatrix;
    }
    else
    {
      modelRotation = glm::mat4(1.0);
    }

    glm::mat4 modelTranslation;
    if(entity.hasComponents<Position>())
    {
      modelTranslation = glm::translate(glm::mat4(), entity.getComponent<Position>().coordinates);
    }
    else
    {
      modelTranslation = glm::mat4(1.0);
    }

    glm::mat4 modelToWorld = modelTranslation * modelRotation;

    for(auto& model : entity.getComponent<Renderable>().models)
    {
      glBindVertexArray(model.vertexArrayObjectID);

			glUseProgram(model.programID);

      glUniformMatrix4fv(
      	model.modelToWorld_UniformLocation,
      	1, GL_FALSE, &(modelToWorld[0][0])
      );
      glUniformMatrix4fv(
      	model.worldToProjection_UniformLocation,
      	1, GL_FALSE, &(worldToProjection[0][0])
      );
      glUniform3fv(
      	model.cameraPosition_UniformLocation,
      	1, &Camera::position.coordinates[0]
      );
      glUniform1i(
      	model.numLights_UniformLocation,
      	numLights
      );
      glUniform3fv(
      	model.lightPosition_UniformLocation,
      	numLights, &lightPosition[0][0]
      );
      glUniform1fv(
      	model.lightPower_UniformLocation,
      	numLights, lightPower
      );
      glUniform3fv(
      	model.lightColor_UniformLocation,
      	numLights, &lightColor[0][0]
      );
      glUniform3fv(
      	model.ambientColor_UniformLocation,
      	1, &model.ambientColor[0]
      );
      glUniform3fv(
      	model.diffuseColor_UniformLocation,
      	1, &model.diffuseColor[0]
      );
      glUniform3fv(
      	model.specularColor_UniformLocation,
      	1, &model.specularColor[0]
      );
      glUniform1f(
      	model.transparency_UniformLocation,
      	model.transparency
      );
      glUniform1f(
      	model.shininess_UniformLocation,
      	model.shininess
      );

      glUniform1i(model.diffuseTexture_UniformLocation, 1);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, model.diffuseTextureID);

      glUniform1i(model.normalMap_UniformLocation, 5);
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, model.normalMapID);

      glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
    }
  }
  glfwSwapBuffers(Window::window);

  static std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
  static size_t frames = 0;
  frames++;
  std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
  if(std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count() >= 1000)
  {
    std::cout<< 1000.0/double(frames) << " ms/frame" << std::endl;;
    frames = 0;
    lastTime = std::chrono::high_resolution_clock::now();
  }
}
