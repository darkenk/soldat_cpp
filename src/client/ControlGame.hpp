#pragma once

#include <string>
#include <cinttypes>
#include <SDL3/SDL_events.h>


union SDL_Event;
struct GlobalStateControlGame
{
  void gameinput(SDL_Event &event);
  bool votekickreasontype = false;

private:
  auto chatkeydown(std::uint8_t keymods, SDL_Keycode keycode) -> bool;
  auto filterchattext(const std::string &str1) -> std::string;
  auto keydown(SDL_KeyboardEvent &keyevent) -> bool;
  auto keyup(SDL_KeyboardEvent &keyevent) -> bool;
  auto menukeydown(std::uint8_t keymods, SDL_Scancode keycode) -> bool;
  void clearchattext();
};

extern GlobalStateControlGame gGlobalStateControlGame;
