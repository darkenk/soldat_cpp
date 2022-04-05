#pragma once

#include <string>

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

extern tgamerenderingparams gamerenderingparams;
extern tgfxspritearray textures;

bool initgamegraphics();
void reloadgraphics();
void destroygamegraphics();
void renderframe(double timeelapsed, double framepercent, bool paused);
void rendergameinfo(const std::string &textstring);
bool dotextureloading(bool finishloading = false);
void setfontstyle(std::int32_t style);
void setfontstyle(std::int32_t style, float scale);
std::int32_t fontstylesize(std::int32_t style);
void takescreenshot(std::string filename, bool async = true);
std::string pngoverride(const std::string_view &filename);
std::string pngoverride(const std::string &filename);
