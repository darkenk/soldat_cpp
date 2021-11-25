#pragma once

#include "Waypoints.hpp"
#include "Util.hpp"
#include "Vector.hpp"
#include <array>
#include <vector>

constexpr std::int32_t max_polys = 5000;
constexpr std::int32_t min_sector = -25;
constexpr std::int32_t max_sector = 25;
constexpr std::int32_t min_sectorz = -35;
constexpr std::int32_t max_sectorz = 35;
constexpr std::int32_t tilesector = 3;
constexpr std::int32_t min_tile = min_sector * tilesector;
constexpr std::int32_t max_tile = max_sector * tilesector;
constexpr std::int32_t max_props = 500;
constexpr std::int32_t max_spawnpoints = 255;
constexpr std::int32_t max_colliders = 128;

typedef std::array<std::uint8_t, 4> tmapcolor; // [r,g,b,a]
typedef tmapcolor *pmapcolor;

typedef struct tmapvertex *pmapvertex;
struct tmapvertex
{
    MyFloat x = 0.0f;
    MyFloat y = 0.0f;
    MyFloat z = 0.0f;
    MyFloat rhw = 0.0f;
    tmapcolor color;
    MyFloat u = 0.0f;
    MyFloat v = 0.0f;
};

typedef struct tmappolygon *pmappolygon;
struct tmappolygon
{
    PascalArray<tmapvertex, 1, 3> vertices;
    PascalArray<tvector3, 1, 3> normals;
    std::uint8_t polytype = 0;
    std::uint8_t textureindex = 0;
};

typedef struct tmapsector *pmapsector;
struct tmapsector
{
    std::vector<std::uint16_t> polys;
};

typedef struct tmapprop *pmapprop;
struct tmapprop
{
    bool active;
    std::uint32_t style;
    std::int32_t width, height;
    float x, y;
    float rotation;
    float scalex, scaley;
    std::uint8_t alpha;
    tmapcolor color;
    std::uint8_t level;
};

typedef struct tmapscenery *pmapscenery;
struct tmapscenery
{
    std::string filename;
    std::int32_t date;
};

typedef struct tmapcollider *pmapcollider;
struct tmapcollider
{
    bool active;
    float x, y;
    float radius;
};

typedef struct tmapspawnpoint *pmapspawnpoint;
struct tmapspawnpoint
{
    bool active;
    std::int32_t x, y, team;
};

typedef struct tmapfile *pmapfile;
struct tmapfile
{
    std::string filename;
    tmapinfo mapinfo;
    std::uint32_t hash;
    std::int32_t version;
    std::string mapname;
    std::vector<std::string> textures;
    tmapcolor bgcolortop;
    tmapcolor bgcolorbtm;
    std::int32_t startjet;
    std::uint8_t grenadepacks;
    std::uint8_t medikits;
    std::uint8_t weather;
    std::uint8_t steps;
    std::int32_t randomid;
    std::vector<tmappolygon> polygons;
    std::int32_t sectorsdivision;
    std::int32_t sectorsnum;
    std::vector<tmapsector> sectors;
    std::vector<tmapprop> props;
    std::vector<tmapscenery> scenery;
    std::vector<tmapcollider> colliders;
    std::vector<tmapspawnpoint> spawnpoints;
    std::vector<twaypoint> waypoints;
};

bool loadmapfile(const tmapinfo &mapinfo, tmapfile &map);
tmapcolor mapcolor(std::uint32_t color);
bool ispropactive(tmapfile &map, std::int32_t index);
