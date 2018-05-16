#include "renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "renderable.hpp"
#include "light.hpp"
#include "ecs.hpp"
#include "camera.hpp"
#include "position.hpp"
#include "orientation.hpp"
#include "window.hpp"
#include "renderTexture.hpp"

const int MAX_NUM_LIGHTS = 10;
const GLuint SHADOW_WIDTH=2048, SHADOW_HEIGHT=2048;
const int NUM_CASCADES = 2;
const float CASCADE_DISTANCES[NUM_CASCADES] = {30.0, 90.0};

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

glm::mat4 getModelToWorld(ecs::Entity entity)
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
  return modelTranslation * modelRotation;
}

void getDataForLightPerspective(
  glm::float32* angle,
  glm::float32* farPlane,
  glm::vec3* lightLookAt,
  glm::vec3* lightUp,
  glm::vec3 cameraViewFrustumCoords[8],
  glm::vec3 cameraPosition,
  glm::vec3 cameraLookAt,
  glm::vec3 lightPosition
)
{
  *lightUp = glm::cross(cameraLookAt-cameraPosition, cameraPosition-lightPosition);
  *lightLookAt = glm::normalize(
    cameraViewFrustumCoords[0]+
    cameraViewFrustumCoords[1]+
    cameraViewFrustumCoords[2]+
    cameraViewFrustumCoords[3]+
    cameraViewFrustumCoords[4]+
    cameraViewFrustumCoords[5]+
    cameraViewFrustumCoords[6]+
    cameraViewFrustumCoords[7]-
    lightPosition*8.0f
  );
  glm::float32 biggestAngle = 0.0;
  *farPlane = 0.0;

  for(size_t i = 0; i<8; i++)
  {
    glm::float32 currentAngle =
      glm::acos(
        glm::abs(glm::dot(*lightLookAt, cameraViewFrustumCoords[i]-lightPosition))/
        (glm::length(*lightLookAt)*glm::length(cameraViewFrustumCoords[i]-lightPosition))
      );
    if(currentAngle>biggestAngle)
    {
      biggestAngle = currentAngle;
    }
    glm::float32 distance = glm::distance(lightPosition, cameraViewFrustumCoords[i]);
    if(distance>*farPlane)
    {
      *farPlane = distance;
    }
  }
  *angle = 2.0*biggestAngle;
}

int getLightData(
  Position localPosition,
  glm::vec3 lightPosition[MAX_NUM_LIGHTS],
  glm::float32 lightPower[MAX_NUM_LIGHTS],
  glm::vec3 lightColor[MAX_NUM_LIGHTS],
  glm::mat4 worldToLight[NUM_CASCADES][MAX_NUM_LIGHTS],
  GLuint depthMapID[NUM_CASCADES][MAX_NUM_LIGHTS]
)
{
  static ecs::Entity influentialLights[MAX_NUM_LIGHTS];
  int numLights = getMostInfluentialLights(localPosition, influentialLights);

  glCullFace(GL_BACK);

  static auto genFramebuffer = [](){GLuint ret; glGenFramebuffers(1, &ret); return ret;};
  static GLuint depthMapFramebufferID = genFramebuffer();
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFramebufferID);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glViewport(0, 0, SHADOW_WIDTH, SHADOW_WIDTH);

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

    //TODO: multiple maps for different scales of view frustums
    glm::float32 angle;
    glm::float32 farPlane;
    glm::vec3 lightLookAt;
    glm::vec3 lightUp;


    int width, height;
    glfwGetFramebufferSize(Window::window, &width, &height);
    glm::float32 aspectRatio = GLfloat(width)/GLfloat(height);

    glm::vec3 h_v = glm::normalize(Camera::viewDirection);
    glm::vec3 w_v = glm::normalize(glm::cross(Camera::cameraUp, Camera::viewDirection));
    glm::vec3 u_v = glm::normalize(glm::cross(w_v, Camera::viewDirection));

    for(int cascadeIndex = 0; cascadeIndex<NUM_CASCADES; cascadeIndex++)
    {
      glm::float32 h_a = CASCADE_DISTANCES[cascadeIndex];
      glm::float32 u_a = glm::tan(Camera::fieldOfView/2.0)*h_a;
      glm::float32 w_a = u_a*aspectRatio;

      glm::vec3 cameraViewFrustumCoords[8];
      if(cascadeIndex==0)
      {
        cameraViewFrustumCoords[0] = Camera::position.coordinates;
        cameraViewFrustumCoords[1] = Camera::position.coordinates;
        cameraViewFrustumCoords[2] = Camera::position.coordinates;
        cameraViewFrustumCoords[3] = Camera::position.coordinates;
        cameraViewFrustumCoords[4] = Camera::position.coordinates + h_v*h_a + u_v*u_a + w_v*w_a;
        cameraViewFrustumCoords[5] = Camera::position.coordinates + h_v*h_a - u_v*u_a + w_v*w_a;
        cameraViewFrustumCoords[6] = Camera::position.coordinates + h_v*h_a + u_v*u_a - w_v*w_a;
        cameraViewFrustumCoords[7] = Camera::position.coordinates + h_v*h_a - u_v*u_a - w_v*w_a;
      }
      else
      {
        glm::float32 h_a_m = CASCADE_DISTANCES[cascadeIndex-1];
        glm::float32 u_a_m = glm::tan(Camera::fieldOfView/2.0)*h_a_m;
        glm::float32 w_a_m = u_a_m*aspectRatio;
        cameraViewFrustumCoords[0] = Camera::position.coordinates + h_v*h_a_m + u_v*u_a_m + w_v*w_a_m;
        cameraViewFrustumCoords[1] = Camera::position.coordinates + h_v*h_a_m - u_v*u_a_m + w_v*w_a_m;
        cameraViewFrustumCoords[2] = Camera::position.coordinates + h_v*h_a_m + u_v*u_a_m - w_v*w_a_m;
        cameraViewFrustumCoords[3] = Camera::position.coordinates + h_v*h_a_m - u_v*u_a_m - w_v*w_a_m;
        cameraViewFrustumCoords[4] = Camera::position.coordinates + h_v*h_a + u_v*u_a + w_v*w_a;
        cameraViewFrustumCoords[5] = Camera::position.coordinates + h_v*h_a - u_v*u_a + w_v*w_a;
        cameraViewFrustumCoords[6] = Camera::position.coordinates + h_v*h_a + u_v*u_a - w_v*w_a;
        cameraViewFrustumCoords[7] = Camera::position.coordinates + h_v*h_a - u_v*u_a - w_v*w_a;
      }

      getDataForLightPerspective(
        &angle,
        &farPlane,
        &lightLookAt,
        &lightUp,
        cameraViewFrustumCoords,
        Camera::position.coordinates,
        Camera::position.coordinates + Camera::viewDirection,
        lightPosition[i]
      );
      worldToLight[cascadeIndex][i] =
        glm::perspective(angle, GLfloat(SHADOW_WIDTH)/GLfloat(SHADOW_HEIGHT), 1.0f, farPlane) *
        glm::lookAt(lightPosition[i], lightLookAt, lightUp);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapID[cascadeIndex][i], 0);
      glClear(GL_DEPTH_BUFFER_BIT);
      for(auto entity : ecs::Iterator<Renderable>())
      {
        glm::mat4 modelToWorld = getModelToWorld(entity);

        for(auto& model : entity.getComponent<Renderable>().models)
        {
          glBindVertexArray(model.vertexArrayObjectID);

    			glUseProgram(Light::depthMapProgramID);

          glUniformMatrix4fv(
  					Light::modelToWorld_UniformLocation,
  					1, GL_FALSE, &(modelToWorld[0][0])
  				);
  				glUniformMatrix4fv(
  					Light::worldToProjection_UniformLocation,
  					1, GL_FALSE, &(worldToLight[cascadeIndex][i][0][0])
  				);

          glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
        }
      }
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glCullFace(GL_FRONT);

  return numLights;
}

void renderer()
{
  static glm::vec3 lightPosition[MAX_NUM_LIGHTS];
  static glm::float32 lightPower[MAX_NUM_LIGHTS];
  static glm::vec3 lightColor[MAX_NUM_LIGHTS];
  static glm::mat4 worldToLight[NUM_CASCADES][MAX_NUM_LIGHTS];
  static GLuint depthMapID[NUM_CASCADES][MAX_NUM_LIGHTS];

  static auto initDepthMaps = [](GLuint depthMapID[NUM_CASCADES][MAX_NUM_LIGHTS])
  {
    for(size_t i = 0; i< MAX_NUM_LIGHTS; i++)
    {
      for(int cascadeIndex = 0; cascadeIndex<NUM_CASCADES; cascadeIndex++)
      {
        glGenTextures(1, &depthMapID[cascadeIndex][i]);
    	  glBindTexture(GL_TEXTURE_2D, depthMapID[cascadeIndex][i]);
    	  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      }
    }
    return 0;
  };
  static char _____c = initDepthMaps(depthMapID);

  int numLights = getLightData(
    Camera::position,
    lightPosition, lightPower, lightColor,
    worldToLight,
    depthMapID
  );

  int width, height;
  glfwGetFramebufferSize(Window::window, &width, &height);
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 worldToProjection =
    glm::perspective(glm::radians(Camera::fieldOfView), GLfloat(width)/GLfloat(height), 0.5f, 1000.0f) *
    glm::lookAt(Camera::position.coordinates, Camera::position.coordinates + Camera::viewDirection, Camera::cameraUp);

  for(auto entity : ecs::Iterator<Renderable>())
  {
    glm::mat4 modelToWorld = getModelToWorld(entity);

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
      glUniformMatrix4fv(
      	glGetUniformLocation(model.programID, "worldToLight[0]"),
      	numLights, GL_FALSE, &(worldToLight[0][0][0][0])
      );
      glUniformMatrix4fv(
      	glGetUniformLocation(model.programID, "worldToLight[1]"),
      	numLights, GL_FALSE, &(worldToLight[1][0][0][0])
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

      glUniform1fv(
      	glGetUniformLocation(model.programID, "cascadeDistances"),
        NUM_CASCADES,	CASCADE_DISTANCES
      );

      int textureCounter = 0;
      glUniform1i(model.diffuseTexture_UniformLocation, textureCounter);
      glActiveTexture(GL_TEXTURE0 + textureCounter++);
      glBindTexture(GL_TEXTURE_2D, model.diffuseTextureID);

      glUniform1i(model.normalMap_UniformLocation, textureCounter);
      glActiveTexture(GL_TEXTURE0 + textureCounter++);
      glBindTexture(GL_TEXTURE_2D, model.normalMapID);

      for(int cascadeIndex = 0; cascadeIndex<NUM_CASCADES; cascadeIndex++)
      {
        GLint tmp[MAX_NUM_LIGHTS];
        for(GLint i = 0; i<MAX_NUM_LIGHTS; i++)
        {
          tmp[i] = textureCounter;

          glActiveTexture(GL_TEXTURE0 + textureCounter++);
    			glBindTexture(GL_TEXTURE_2D, depthMapID[cascadeIndex][i]);
        }
        glUniform1iv(glGetUniformLocation(model.programID, (std::string("depthMap[")+std::to_string(cascadeIndex)+std::string("]")).c_str()),  MAX_NUM_LIGHTS,  tmp);
      }


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
