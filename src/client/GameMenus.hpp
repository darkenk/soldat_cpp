#pragma once

#include <cstdint>
#include <string>
#include <vector>

typedef struct tgamebutton *pgamebutton;
struct tgamebutton
{
  bool active;
  std::int32_t x1, y1, x2, y2;
  std::string caption;
};

typedef struct tgamemenu *pgamemenu;
struct tgamemenu
{
  bool active;
  std::int32_t x, y;
  std::int32_t w, h;
  std::vector<tgamebutton> button;
};

struct GlobalStateGameMenus
{
  bool gamemenuaction(pgamemenu menu, std::int32_t buttonindex);
  bool gamemenuclick();
  void gamemenumousemove();
  void gamemenushow(pgamemenu menu, bool show = true);
  void initgamemenus();
  std::vector<tgamemenu> gamemenu = {};
  pgamemenu hoveredmenu = {};
  pgamebutton hoveredbutton = {};
  std::int32_t hoveredbuttonindex = {};
  pgamemenu escmenu = {};
  pgamemenu teammenu = {};
  pgamemenu limbomenu = {};
  pgamemenu kickmenu = {};
  pgamemenu mapmenu = {};
  std::int32_t kickmenuindex = 0;
  std::int32_t mapmenuindex = 0;

private:
  bool limbowasactive{};
  void hideall();
  void initbutton(pgamemenu menu, std::int32_t button, const std::string &caption, std::int32_t x,
                  std::int32_t y, std::int32_t w, std::int32_t h, bool active = true);
};

extern GlobalStateGameMenus gGlobalStateGameMenus;
