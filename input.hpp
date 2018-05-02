#pragma once

#include "window.hpp"
#include "ecs.hpp"
#include "gamestate.hpp"

namespace Input
{
  void catchInput()
  {
    glfwPollEvents();
    if(glfwWindowShouldClose(Window::window))
    {
      ecs::SystemManager::throwEvent(Gamestate::ChangeGamestate{Gamestate::Gamestate::Exiting});
    }
  }
}
