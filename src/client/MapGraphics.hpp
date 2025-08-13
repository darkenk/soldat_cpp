#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <format>
#include <string>

#include "Gfx.hpp"
#include "common/MapFile.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include "common/misc/SafeType.hpp"

struct tmapgraphics
{
  std::string filename;
  tmapinfo mapinfo;
  bool bgforce;
  std::array<tmapcolor, 2> bgforcedcolor;
  tgfxvertexbuffer *vertexbuffer = nullptr;
  tgfxindexbuffer *indexbuffer = nullptr;
  std::vector<tgfxtexture *> textures;
  std::vector<tgfxtexture *> edgetextures;
  tgfxspritesheet *spritesheet;
  std::vector<pgfxsprite> animations;
  std::vector<pgfxdrawcommand> animationscmd;
  std::vector<tgfxvertex> animationsbuffer;
  std::vector<std::int32_t> animduration;
  std::array<std::vector<tgfxdrawcommand>, 3> props;
  tgfxsprite minimap;
  float minimapscale;
  tvector2 minimapoffset{};
  tgfxcolor bgcolortop, bgcolorbtm;
  std::int32_t background, backgroundcount;
  std::array<std::vector<tgfxdrawcommand>, 2> edges;
  std::array<std::vector<tgfxdrawcommand>, 2> polys;
};

struct GlobalStateMapGraphics
{
  auto gettexturetargetscale(tmapfile &mapfile, tgfximage *image) -> float;
  void destroymapgraphics();
  void loadmapgraphics(tmapfile &mapfile, bool bgforce, tmapcolor bgcolortop, tmapcolor bgcolorbtm);
  void renderminimap(float x, float y, std::uint8_t alpha);
  void renderprops(std::int32_t level);
  void settexturefilter(tgfxtexture *texture, bool allowmipmaps);
  void updateprops(double t);
  void worldtominimap(float x, float y, MyFloat &ox, MyFloat &oy);
  tmapgraphics mapgfx;
};

extern GlobalStateMapGraphics gGlobalStateMapGraphics;
