#pragma once

#include <string>
#include <cstdint>
#include <string_view>

#include "Gfx.hpp"
/*#include "SDL2.h"*/

struct tgamerenderingparams
{
  std::string interfacename;
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
  tgamerenderingparams gamerenderingparams;
  tgfxspritearray textures;
};

extern GlobalStateGameRendering gGlobalStateGameRendering;
