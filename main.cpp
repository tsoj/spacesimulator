#include <iostream>
#include <chrono>

#include "window.hpp"
#include "ecs.hpp"
#include "input.hpp"
#include "gamestate.hpp"

void hi(){}

int main()
{
  ecs::SystemManager::addSystem(Input::catchInput, ecs::Duration());
  ecs::SystemManager::addSystem(Gamestate::exitGame);

  Window::initWindow();
  while(true)
  {
    ecs::SystemManager::runSystems();
  }
  return 0;
}
