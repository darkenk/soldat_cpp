#pragma once

struct GlobalStateControlGame
{
  bool votekickreasontype;
};

extern GlobalStateControlGame gGlobalStateControlGame;

union SDL_Event;
void gameinput(SDL_Event &event);
