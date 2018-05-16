#include <iostream>
#include <chrono>

#include <glm/gtc/matrix_transform.hpp>

#include "window.hpp"
#include "ecs.hpp"
#include "input.hpp"
#include "gamestate.hpp"
#include "renderer.hpp"
#include "renderable.hpp"
#include "light.hpp"
#include "orientation.hpp"
#include "camera.hpp"

void rotate()
{
  for(auto entity : ecs::Iterator<Orientation>())
  {
    entity.getComponent<Orientation>().rotationMatrix =
      glm::rotate(glm::mat4(), glm::radians(1.0f), glm::vec3(0.0, 1.0, 0.0)) * entity.getComponent<Orientation>().rotationMatrix;
  }
}

void moveLight()
{
  static std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  float delta = (float)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
  for(auto entity : ecs::Iterator<Light, Position>())
  {
    entity.getComponent<Position>().coordinates.x = 20.0*glm::sin(delta/500.0);
  }
}

void moveSpacePlaneAway()
{
  for(auto entity : ecs::Iterator<Position, Renderable>())
  {
    entity.getComponent<Position>().coordinates.z += -0.1;
  }
}

int main()
{
  Window::init();
  Light::init();

  ecs::SystemManager::addSystem(Input::catchInput, std::chrono::milliseconds(0));
  ecs::SystemManager::addSystem(Gamestate::exitGame);
  ecs::SystemManager::addSystem(renderer, std::chrono::milliseconds(0));
  ecs::SystemManager::addSystem(rotate, std::chrono::milliseconds(10));
  ecs::SystemManager::addSystem(moveSpacePlaneAway, std::chrono::milliseconds(10));

  auto spaceship = ecs::Entity::createEntity();
  spaceship.createComponent<Renderable>();
  spaceship.createComponent<Position>();
  spaceship.createComponent<Orientation>();
  spaceship.getComponent<Renderable>().init("model/spaceboat.obj", "shader/phong.vert", "shader/phong.frag");
  spaceship.getComponent<Position>().coordinates = glm::vec3(0.0, -2.0, -20.0);
  spaceship.getComponent<Orientation>().rotationMatrix = glm::rotate(glm::mat4(), glm::radians(200.0f), glm::vec3(0.0, 1.0, 0.0));

  auto spaceship2 = ecs::Entity::createEntity();
  spaceship2.createComponent<Renderable>();
  spaceship2.createComponent<Position>();
  spaceship2.createComponent<Orientation>();
  spaceship2.getComponent<Renderable>().init("model/spaceboat.obj", "shader/phong.vert", "shader/phong.frag");
  spaceship2.getComponent<Position>().coordinates = glm::vec3(6.0, 2.0, -20.0);
  spaceship2.getComponent<Orientation>().rotationMatrix = glm::rotate(glm::mat4(), glm::radians(300.0f), glm::vec3(0.0, 1.0, 0.0));

  auto plane = ecs::Entity::createEntity();
  plane.createComponent<Renderable>();
  plane.createComponent<Position>();
  plane.createComponent<Orientation>();
  plane.getComponent<Renderable>().init("model/plane.obj", "shader/phong.vert", "shader/phong.frag");
  plane.getComponent<Position>().coordinates = glm::vec3(0.0, -5.0, -20.0);
  plane.getComponent<Orientation>().rotationMatrix = glm::mat4(1.0);

  auto light = ecs::Entity::createEntity();
  light.createComponent<Light>();
  light.createComponent<Position>();
  light.getComponent<Light>().color = glm::vec3(1.0, 0.0, 0.0);
  light.getComponent<Light>().power = 1000000.0;
  light.getComponent<Position>().coordinates = glm::vec3(0.0, 1000.0, -20.0);

  auto light2 = ecs::Entity::createEntity();
  light2.createComponent<Light>();
  light2.createComponent<Position>();
  light2.getComponent<Light>().color = glm::vec3(0.0, 1.0, 0.0);
  light2.getComponent<Light>().power = 1000000.0;
  light2.getComponent<Position>().coordinates = glm::vec3(0.0, 500.0, 1000.0);

  /*auto light3 = ecs::Entity::createEntity();
  light3.createComponent<Light>();
  light3.createComponent<Position>();
  light3.getComponent<Light>().color = glm::vec3(0.0, 0.0, 1.0);
  light3.getComponent<Light>().power = 10000.0;
  light3.getComponent<Position>().coordinates = glm::vec3(10.0, 0.0, -60.0);*/

  Camera::cameraUp = glm::vec3(0.0, 1.0, 0.0);
  Camera::position = Position{glm::vec3(0.0, 0.0, 0.0)};
  Camera::viewDirection = glm::vec3(0.0, 0.0, -1.0);
  Camera::fieldOfView = 70.0;

  while(true)
  {
    ecs::SystemManager::runSystems();
  }
  //TODO: proper deinitialization
  return 0;
}
