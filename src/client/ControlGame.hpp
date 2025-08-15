#pragma once

union SDL_Event;
struct GlobalStateControlGame
{
  void gameinput(SDL_Event &event);
  bool votekickreasontype = false;
};

extern GlobalStateControlGame gGlobalStateControlGame;
