#pragma once

#include "MapFile.hpp"
#include "Util.hpp"
#include "Vector.hpp"
#include "Waypoints.hpp"
#include "misc/PortUtilsSoldat.hpp"

// Polygon constants go here
// ...
const std::int32_t poly_type_normal = 0;
const std::int32_t poly_type_only_bullets = 1;
const std::int32_t poly_type_only_player = 2;
const std::int32_t poly_type_doesnt = 3;
const std::int32_t poly_type_ice = 4;
const std::int32_t poly_type_deadly = 5;
const std::int32_t poly_type_bloody_deadly = 6;
const std::int32_t poly_type_hurts = 7;
const std::int32_t poly_type_regenerates = 8;
const std::int32_t poly_type_lava = 9;
const std::int32_t poly_type_red_bullets = 10;
const std::int32_t poly_type_red_player = 11;
const std::int32_t poly_type_blue_bullets = 12;
const std::int32_t poly_type_blue_player = 13;
const std::int32_t poly_type_yellow_bullets = 14;
const std::int32_t poly_type_yellow_player = 15;
const std::int32_t poly_type_green_bullets = 16;
const std::int32_t poly_type_green_player = 17;
const std::int32_t poly_type_bouncy = 18;
const std::int32_t poly_type_explodes = 19;
const std::int32_t poly_type_hurts_flaggers = 20;
const std::int32_t poly_type_only_flaggers = 21;
const std::int32_t poly_type_not_flaggers = 22;
const std::int32_t poly_type_non_flagger_collides = 23;
const std::int32_t poly_type_background = 24;
const std::int32_t poly_type_background_transition = 25;

// The poly will interact as an "only players collide" poly
const std::int32_t background_normal = 0;

// The poly will interact as a "doesn't collide" poly
const std::int32_t background_transition = 1;

// Whether a background poly is being interacted with is currently unknown and
// must be found out
const std::int32_t background_poly_unknown = -2;

// No background poly is currently being interacted with
const std::int32_t background_poly_none = -1;

typedef void (*tloadmapgraphics)(tmapfile &mapfile, bool bgforce, tmapcolor bgcolortop,
                                 tmapcolor bgcolorbtm);

extern twaypoints botpath;

class tpolymap
{
  public:
    std::uint32_t mapid;
    tmapinfo mapinfo;
    std::string name;
    std::string filename;
    std::int32_t sectorsdivision;
    std::int32_t sectorsnum;
    std::int32_t startjet;
    std::uint8_t grenades;
    std::uint8_t medikits;
    std::uint8_t weather;
    std::uint8_t steps;
    std::int32_t polycount;
    std::int32_t backpolycount;
    std::int32_t collidercount;
    PascalArray<tmappolygon, 1, max_polys> polys;
    PascalArray<pmappolygon, 1, max_polys> backpolys;
    PascalArray<std::uint8_t, 1, max_polys> polytype;
    PascalArray<PascalArray<tvector2, 1, 3>, 1, max_polys> perp;
    PascalArray<float, 1, max_polys> bounciness;
    PascalArray<PascalArray<tmapsector, min_sectorz, max_sectorz>, min_sectorz, max_sectorz>
        sectors;
    PascalArray<tmapspawnpoint, 1, max_spawnpoints> spawnpoints;
    PascalArray<tmapcollider, 1, max_spawnpoints> collider;
    PascalArray<std::int32_t, 1, 2> flagspawn;
    tloadmapgraphics loadgraphics;
    bool loadmap(const tmapinfo &map);
#ifndef SERVER
    bool loadmap(tmapinfo map, bool bgforce, std::uint32_t bgcolortop, std::uint32_t bgcolorbtm);
    ;
#endif
    bool lineinpoly(const tvector2 a, tvector2 b, std::int32_t poly, tvector2 &v);
    bool pointinpolyedges(float x, float y, std::int32_t i);
    bool pointinpoly(const tvector2 p, const tmappolygon &poly);
    tvector2 closestperpendicular(std::int32_t j, tvector2 pos, float &d, std::int32_t &n);
    bool collisiontest(tvector2 pos, tvector2 &perpvec, bool isflag = false);
    bool collisiontestexcept(tvector2 pos, tvector2 &perpvec, std::int32_t c);
    bool raycast(const tvector2 a, tvector2 b, float &distance, float maxdist, bool player = false,
                 bool flag = false, bool bullet = true, bool checkcollider = false,
                 std::uint8_t team = 0);

  private:
    void initialize();
    void loaddata(tmapfile &mapfile);
};

void checkoutofbounds(MyFloat &x, MyFloat &y);
void checkoutofbounds(const int16_t &x, const int16_t &y);
