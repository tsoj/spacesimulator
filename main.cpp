#include <iostream>
#include <chrono>

#include "window.hpp"
#include "ecs.hpp"
#include "input.hpp"
#include "gamestate.hpp"
#include "renderer.hpp"
#include "renderable.hpp"

void hi(){}

int main()
{
  Window::initWindow();

  ecs::SystemManager::addSystem(Input::catchInput, std::chrono::milliseconds(0));
  ecs::SystemManager::addSystem(Gamestate::exitGame);
  ecs::SystemManager::addSystem(renderer, std::chrono::milliseconds(0));

  auto spaceship = ecs::Entity::createEntity();
  spaceship.createComponent<Renderable>();
  spaceship.createComponent<Position>();
  spaceship.createComponent<Orientation>();
  spaceship.getComponent<Renderable>().init("spaceboat.obj", "shader.vert", "shader.frag");
  spaceship.getComponent<Position>().coordinates = glm::vec3(0.0, 5.0, -20.0);
  spaceship.getComponent<Orientation>().rotationMatrix = glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));

  auto plane = ecs::Entity::createEntity();
  plane.createComponent<Renderable>();
  plane.createComponent<Position>();
  plane.createComponent<Orientation>();
  plane.getComponent<Renderable>().init("Plane.obj", "shader.vert", "shader.frag");
  plane.getComponent<Position>().coordinates = glm::vec3(0.0, -5.0, -20.0);
  plane.getComponent<Orientation>().rotationMatrix = glm::mat4(1.0);

  auto light = ecs::Entity::createEntity();
  light.createComponent<Light>();
  light.createComponent<Position>();
  light.getComponent<Light>().color = glm::vec3(1.0, 0.0, 0.0);
  light.getComponent<Light>().power = 10000.0;
  light.getComponent<Light>().worldToLightSpace = glm::mat4(1.0);//TODO:
  light.getComponent<Position>().coordinates = glm::vec3(0.0, 20.0, -20.0);


  auto light2 = ecs::Entity::createEntity();
  light2.createComponent<Light>();
  light2.createComponent<Position>();
  light2.getComponent<Light>().color = glm::vec3(0.0, 1.0, 0.0);
  light2.getComponent<Light>().power = 1000.0;
  light2.getComponent<Light>().worldToLightSpace = glm::mat4(1.0);//TODO:
  light2.getComponent<Position>().coordinates = glm::vec3(0.0, 0.0, -20.0);

  Camera::cameraUp = glm::vec3(0.0, 1.0, 0.0);
  Camera::position = Position{glm::vec3(0.0, 0.0, 0.0)};
  Camera::viewDirection = glm::vec3(0.0, 0.0, -1.0);
  Camera::fieldOfView = 70.0;

  while(true)
  {
    ecs::SystemManager::runSystems();
  }
  return 0;
}
