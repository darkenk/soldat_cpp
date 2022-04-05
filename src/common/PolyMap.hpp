#pragma once

#include "MapFile.hpp"
#include "Util.hpp"
#include "Vector.hpp"
#include "Waypoints.hpp"
#include "misc/PortUtilsSoldat.hpp"

// Polygon constants go here
// ...

enum PolygonType : std::uint16_t
{
  poly_type_normal = 0,
  poly_type_only_bullets = 1,
  poly_type_only_player = 2,
  poly_type_doesnt = 3,
  poly_type_ice = 4,
  poly_type_deadly = 5,
  poly_type_bloody_deadly = 6,
  poly_type_hurts = 7,
  poly_type_regenerates = 8,
  poly_type_lava = 9,
  poly_type_red_bullets = 10,
  poly_type_red_player = 11,
  poly_type_blue_bullets = 12,
  poly_type_blue_player = 13,
  poly_type_yellow_bullets = 14,
  poly_type_yellow_player = 15,
  poly_type_green_bullets = 16,
  poly_type_green_player = 17,
  poly_type_bouncy = 18,
  poly_type_explodes = 19,
  poly_type_hurts_flaggers = 20,
  poly_type_only_flaggers = 21,
  poly_type_not_flaggers = 22,
  poly_type_non_flagger_collides = 23,
  poly_type_background = 24,
  poly_type_background_transition = 25
};

struct PolyMapSector
{
  struct Vertex
  {
    Vertex(const tmapvertex &v) : x{v.x}, y{v.y}
    {
    }
    float x;
    float y;
  };

  struct Poly
  {
    Poly(std::uint16_t idx, PolygonType type, const tmapvertex &a, const tmapvertex &b,
         const tmapvertex &c)
      : Index{idx}, Type{type}, Vertices{a, b, c}
    {
    }
    const std::uint16_t Index;
    const PolygonType Type;
    const std::array<Vertex, 3> Vertices;
  };
  using TPolys = std::vector<Poly>;
  TPolys Polys;
};

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

class Polymap
{
public:
  std::uint32_t mapid;
  tmapinfo mapinfo;
  std::string name;
  std::string filename;
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
  PascalArray<PascalArray<tvector2, 1, 3>, 1, max_polys> perp;
  PascalArray<float, 1, max_polys> bounciness;

  PascalArray<tmapspawnpoint, 1, max_spawnpoints> spawnpoints;
  PascalArray<tmapcollider, 1, max_spawnpoints> collider;
  PascalArray<std::int32_t, 1, 2> flagspawn;
  tloadmapgraphics loadgraphics;

public:
  struct Sector
  {
    inline bool IsValid() const
    {
      return Polys != nullptr;
    }

    inline const PolyMapSector::TPolys &GetPolys() const
    {
      return *Polys;
    }

    static inline Sector CreateInvalid()
    {
      return {nullptr};
    }

  private:
    Sector(PolyMapSector::TPolys *polys) : Polys{polys} {};
    PolyMapSector::TPolys *Polys;

    friend class Polymap;
  };

  Polymap(twaypoints &botpath) : botpath{botpath} {};
  bool loadmap(const tmapfile &mapfile);
  bool loadmap(const tmapinfo &map);
  bool loadmap(const tmapinfo &map, bool bgforce, std::uint32_t bgcolortop,
               std::uint32_t bgcolorbtm);
  bool pointinpolyedges(float x, float y, std::int32_t i);
  bool pointinpoly(const tvector2 &p, const tmappolygon &poly);
  bool PointInPoly(const tvector2 &p, const PolyMapSector::Poly &poly);
  tvector2 closestperpendicular(std::int32_t j, const tvector2 &pos, float &d, std::int32_t &n);
  bool collisiontest(const tvector2 &pos, tvector2 &perpvec, bool isflag = false);
  bool collisiontestexcept(const tvector2 &pos, tvector2 &perpvec, std::int32_t c);
  bool raycast(const tvector2 &a, const tvector2 &b, float &distance, float maxdist,
               bool player = false, bool flag = false, bool bullet = true,
               bool checkcollider = false, std::uint8_t team = 0);

  void checkoutofbounds(MyFloat &x, MyFloat &y);
  void checkoutofbounds(std::int16_t &x, std::int16_t &y);

  static bool ShouldTestPolygonWithRay(const uint8_t polygonType, const bool npcol,
                                       const bool nbcol, const bool flag, const uint8_t team);
  void SetSectorsDivision(std::int32_t sectorsdivision);
  std::int32_t GetSectorsDivision()
  {
    return SectorsDivision;
  }

  Sector GetSector(const tvector2 &pos);

private:
  void initialize();
  void loaddata(const tmapfile &mapfile);
  twaypoints &botpath;
  bool RayCastOpt(const tvector2 &a, const tvector2 &b, float &distance, float maxdist,
                  bool player = false, bool flag = false, bool bullet = true,
                  bool checkcollider = false, std::uint8_t team = 0);

  struct SectorCoord
  {
    std::int32_t x;
    std::int32_t y;
    inline bool IsValid() const
    {
      return x != std::numeric_limits<std::int32_t>::max() &&
             y != std::numeric_limits<std::int32_t>::max();
    }
    static inline SectorCoord CreateInvalid()
    {
      return {std::numeric_limits<std::int32_t>::max(), std::numeric_limits<std::int32_t>::max()};
    }
  };

  SectorCoord GetSectorCoord(const tvector2 &pos);
  SectorCoord GetSectorCoordUnsafe(const tvector2 &pos);
  std::int32_t GetIndex(const SectorCoord &s);

  float PositionToSectorScale;
  float MapHalfSize;

  std::int32_t SectorsDivision;
  std::vector<PolyMapSector> Sectors;
};

using tpolymap = Polymap;
