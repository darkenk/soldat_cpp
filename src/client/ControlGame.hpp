#pragma once

union SDL_Event;
struct GlobalStateControlGame
{
  void gameinput(SDL_Event &event);
  bool votekickreasontype;
};

extern GlobalStateControlGame gGlobalStateControlGame;
