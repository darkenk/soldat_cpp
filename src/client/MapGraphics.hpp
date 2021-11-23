#pragma once

#include "Gfx.hpp"
#include "shared/MapFile.hpp"
#include "common/Util.hpp"
#include "shared/Vector.hpp"

#include <array>
#include <vector>

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
    tvector2 minimapoffset;
    tgfxcolor bgcolortop, bgcolorbtm;
    std::int32_t background, backgroundcount;
    std::array<std::vector<tgfxdrawcommand>, 2> edges;
    std::array<std::vector<tgfxdrawcommand>, 2> polys;
};

extern tmapgraphics mapgfx;

void loadmapgraphics(tmapfile &mapfile, bool bgforce, tmapcolor bgcolortop, tmapcolor bgcolorbtm);
void destroymapgraphics();
void updateprops(double t);
void renderprops(std::int32_t level);
void renderminimap(float x, float y, std::uint8_t alpha);
void worldtominimap(float x, float y, MyFloat &ox, MyFloat &oy);
void settexturefilter(tgfxtexture *texture, bool allowmipmaps);
