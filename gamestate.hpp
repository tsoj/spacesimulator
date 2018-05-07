#pragma once

#include <iostream>

namespace Gamestate
{
  enum Gamestate {Running, Exiting};

  struct ChangeGamestate
  {
    Gamestate newGamestate;
  };

  inline void exitGame(const ChangeGamestate& changeGamestate)
  {
    if(changeGamestate.newGamestate == Gamestate::Exiting)
    {
      exit(0);
    }
  }
}
