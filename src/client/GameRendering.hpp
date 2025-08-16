#pragma once

#include <string>
#include <cstdint>
#include <string_view>

#include "Gfx.hpp"
#include "shared/mechanics/Sprites.hpp"
/*#include "SDL2.h"*/

struct tgamerenderingparams
{
  std::string interfacename;
};

struct tinterpolationstate
{
  tvector2 camera;
  tvector2 mouse;
  std::array<PascalArray<tvector2, 1, max_sprites>, 24> spritepos;
  PascalArray<tvector2, 1, max_bullets> bulletpos;
  PascalArray<tvector2, 1, max_bullets> bulletvel;
  PascalArray<float, 1, max_bullets> bullethitmul;
  PascalArray<tvector2, 1, max_sparks> sparkpos;
  PascalArray<PascalArray<tvector2, 1, 4>, 1, max_things> thingpos;
};

constexpr std::int32_t font_big = 0;
constexpr std::int32_t font_small = 1;
constexpr std::int32_t font_small_bold = 2;
constexpr std::int32_t font_smallest = 3;
constexpr std::int32_t font_menu = 4;
constexpr std::int32_t font_weapons_menu = 5;
constexpr std::int32_t font_world = 6;
constexpr std::int32_t font_last = font_world;

struct GlobalStateGameRendering
{
  bool dotextureloading(bool finishloading = false);
  bool initgamegraphics();
  std::int32_t fontstylesize(std::int32_t style);
  std::string pngoverride(const std::string &filename);
  std::string pngoverride(const std::string_view &filename);
  void destroygamegraphics();
  void gfxSetGpuDevice(SDL_GPUDevice *device);
  void reloadgraphics();
  void renderframe(double timeelapsed, double framepercent, bool paused);
  void rendergameinfo(const std::string &textstring);
  void setfontstyle(std::int32_t style);
  void setfontstyle(std::int32_t style, float scale);
  void takescreenshot(std::string filename, bool async = true);
  tgamerenderingparams gamerenderingparams = {};
  tgfxspritearray textures = {};

  void loadfonts(); // should be private, but there are tests already

private:
  auto getfontpath(std::string fallback, std::string &fontfile) -> std::string;
  auto getfontpath(std::string fontfile) -> std::string;
  auto getimagescale(const std::string &imagepath) -> float;
  auto getsizeconstraint(std::int32_t id, std::int32_t &w, std::int32_t &h) -> bool;
  void gfxlogcallback(const std::string &s);
  void interpolatestate(float p, tinterpolationstate &s, bool paused);
  void loadinterface();
  void loadinterfacetextures(const std::string interfacename);
  void loadmaintextures();
  void loadmodinfo();
  void restorestate(tinterpolationstate &s);
};

extern GlobalStateGameRendering gGlobalStateGameRendering;
