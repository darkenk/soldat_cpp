// automatically converted
#include "PolyMap.hpp"
#include "Calc.hpp"
#include "Constants.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include <Tracy.hpp>
#include <map>

namespace
{
PolygonType SerializePolygonType(const std::uint8_t polygonType)
{
    static const std::map<std::uint8_t, PolygonType> convert = {
        {0, poly_type_normal},
        {1, poly_type_only_bullets},
        {2, poly_type_only_player},
        {3, poly_type_doesnt},
        {4, poly_type_ice},
        {5, poly_type_deadly},
        {6, poly_type_bloody_deadly},
        {7, poly_type_hurts},
        {8, poly_type_regenerates},
        {9, poly_type_lava},
        {10, poly_type_red_bullets},
        {11, poly_type_red_player},
        {12, poly_type_blue_bullets},
        {13, poly_type_blue_player},
        {14, poly_type_yellow_bullets},
        {15, poly_type_yellow_player},
        {16, poly_type_green_bullets},
        {17, poly_type_green_player},
        {18, poly_type_bouncy},
        {19, poly_type_explodes},
        {20, poly_type_hurts_flaggers},
        {21, poly_type_only_flaggers},
        {22, poly_type_not_flaggers},
        {23, poly_type_non_flagger_collides},
        {24, poly_type_background},
        {25, poly_type_background_transition}};
    SoldatAssert(convert.contains(polygonType));
    return convert.at(polygonType);
};
} // namespace

void Polymap::initialize()
{
    this->mapid = 0;
    this->name = "";
    this->filename = "";
    this->SectorsDivision = 0;
    this->sectorsnum = 0;
    this->startjet = 0;
    this->grenades = 0;
    this->medikits = 0;
    this->weather = 0;
    this->steps = 0;
    this->polycount = 0;
    this->backpolycount = 0;
    this->collidercount = 0;

    auto idx = 1;
    std::memset(&this->polys[idx], 0, sizeof(this->polys));
    fillchar(&this->backpolys[idx], sizeof(this->backpolys), 0);
    fillchar(&this->perp[idx][idx], sizeof(this->perp), 0);
    fillchar(&this->bounciness[idx], sizeof(this->bounciness), 0);
    fillchar(&this->spawnpoints[idx], sizeof(this->spawnpoints), 0);
    fillchar(&this->collider[idx], sizeof(this->collider), 0);
    fillchar(&this->flagspawn[idx], sizeof(this->flagspawn), 0);

    // BotPath (TWaypoints) defined in Game.pas
    fillchar(&botpath.waypoint[idx], sizeof(botpath.waypoint), 0);
}

void Precompute(tmappolygon &polygon)
{
    auto calc = [](const tmapvertex &p, const tmapvertex &q) {
        if (q.x != p.x)
        {
            return ((q.y - p.y)) / (q.x - p.x);
        }
        return std::numeric_limits<float>::max();
    };
    const auto &v1 = polygon.vertices[0];
    const auto &v2 = polygon.vertices[1];
    const auto &v3 = polygon.vertices[2];
    polygon.bk[0] = calc(v1, v2);
    polygon.bk[1] = calc(v2, v3);
    polygon.bk[2] = calc(v3, v1);
}

void Polymap::loaddata(const tmapfile &mapfile)
{
    std::int32_t i;

    this->mapid = mapfile.hash;
    this->sectorsnum = mapfile.sectorsnum;
    this->SetSectorsDivision(mapfile.sectorsdivision);
    this->startjet = 119 * mapfile.startjet / 100; // quickfix bla bla
    this->grenades = mapfile.grenadepacks;
    this->medikits = mapfile.medikits;
    this->weather = mapfile.weather;
    this->steps = mapfile.steps;
    this->polycount = length(mapfile.polygons);
    this->collidercount = length(mapfile.colliders);

    if (this->polycount > 0)
    {
        std::memcpy(&this->polys[1], &mapfile.polygons[0], sizeof(tmappolygon) * this->polycount);
    }

    if (this->collidercount > 0)
    {
        std::memcpy(&this->collider[1], &mapfile.colliders[0],
                    sizeof(tmapcollider) * this->collidercount);
    }

    if (length(mapfile.spawnpoints) > 0)
    {
        std::memcpy(&this->spawnpoints[1], &mapfile.spawnpoints[0],
                    sizeof(tmapspawnpoint) * length(mapfile.spawnpoints));
    }

    if (length(mapfile.waypoints) > 0)
    {
        NotImplemented("map");
        std::memcpy(&botpath.waypoint[1], &mapfile.waypoints[0],
                    sizeof(twaypoint) * length(mapfile.waypoints));
    }

    for (i = 1; i <= this->polycount; i++)
    {
        this->perp[i][1].x = this->polys[i].normals[0].x;
        this->perp[i][1].y = this->polys[i].normals[0].y;
        this->perp[i][2].x = this->polys[i].normals[1].x;
        this->perp[i][2].y = this->polys[i].normals[1].y;
        this->perp[i][3].x = this->polys[i].normals[2].x;
        this->perp[i][3].y = this->polys[i].normals[2].y;

        this->bounciness[i] = vec2length(this->perp[i][3]); // gg

        vec2normalize(this->perp[i][1], this->perp[i][1]);
        vec2normalize(this->perp[i][2], this->perp[i][2]);
        vec2normalize(this->perp[i][3], this->perp[i][3]);

        const auto polytype = SerializePolygonType(this->polys[i].polytype);

        if ((polytype == poly_type_background) || (polytype == poly_type_background_transition))
        {
            this->backpolycount += 1;
            this->backpolys[this->backpolycount] = &this->polys[i];
        }

        Precompute(this->polys[i]);
    }

    Sectors.reserve(mapfile.sectors.size());
    for (const auto &s : mapfile.sectors)
    {
        auto &poly = Sectors.emplace_back().Polys;
        for (const auto &p : s.Polys)
        {
            const tmapvertex &a = polys[p].vertices[0];
            const tmapvertex &b = polys[p].vertices[1];
            const tmapvertex &c = polys[p].vertices[2];
            poly.emplace_back(p, SerializePolygonType(polys[p].polytype), a, b, c);
        }
    }

    for (i = 1; i <= length(mapfile.spawnpoints); i++)
    {
        if ((std::abs(this->spawnpoints[i].x) >= 2000000) ||
            (std::abs(this->spawnpoints[i].y) >= 2000000))
            this->spawnpoints[i].active = false;

        if (this->spawnpoints[i].active)
        {
            if ((flagspawn[1] == 0) && (this->spawnpoints[i].team == 5))
                flagspawn[1] = i;

            if ((flagspawn[2] == 0) && (this->spawnpoints[i].team == 6))
                flagspawn[2] = i;
        }
    }

    for (i = 1; i < length(mapfile.waypoints); i++)
    {
        if ((std::abs(botpath.waypoint[i].x) >= 2000000) ||
            (std::abs(botpath.waypoint[i].y) >= 2000000))
        {
            botpath.waypoint[i].active = false;
        }
    }
}

bool Polymap::loadmap(const tmapfile &mapfile)
{
    initialize();
    this->filename = mapfile.filename;
    this->loaddata(mapfile);
    this->name = mapfile.mapname;
    this->mapinfo = mapfile.mapinfo;
    return true;
}

bool Polymap::loadmap(const tmapinfo &map)
{
    tmapfile mapfile;

    bool result = false;
    this->initialize();

    if (loadmapfile(map, mapfile))
    {
        this->filename = map.name;
        this->loaddata(mapfile);
        this->name = map.name;
        this->mapinfo = map;
        result = true;
    }
    return result;
}

bool Polymap::loadmap(const tmapinfo &map, bool bgforce, std::uint32_t bgcolortop,
                      std::uint32_t bgcolorbtm)
{
    tmapfile mapfile;

    bool result;
    if (this->filename == map.name)
    {
        result = true;
        return result;
    }

    result = false;
    this->initialize();

    if (loadmapfile(map, mapfile))
    {
        this->filename = map.name;
        this->loaddata(mapfile);
        this->name = map.name;
        this->mapinfo = map;

        if (this->loadgraphics != nullptr)
        {
            this->loadgraphics(mapfile, bgforce, mapcolor(bgcolortop), mapcolor(bgcolorbtm));
        }
        result = true;
    }
    return result;
}

static bool LineInPoly(const tvector2 &a, const tvector2 &b, const PolyMapSector::Poly &poly,
                       tvector2 &v, const float ak)
{
    ZoneScopedN("LineInPoly");

    auto calc = [&](const PolyMapSector::Vertex &p, const PolyMapSector::Vertex &q) {
        if (b.x != a.x && q.x != p.x)
        {
            float bk = ((q.y - p.y)) / (q.x - p.x);
            if (ak != bk)
            {
                float am = a.y - ak * a.x;
                float bm = p.y - bk * p.x;
                v.x = ((bm - am)) / (ak - bk);
                v.y = ak * v.x + am;

                if ((v.x > std::min(p.x, q.x)) && (v.x < std::max(p.x, q.x)) &&
                    (v.x > std::min(a.x, b.x)) && (v.x < std::max(a.x, b.x)))
                {
                    return true;
                }
            }
        }
        else if (b.x == a.x && q.x != p.x)
        {
            float bk = ((q.y - p.y)) / (q.x - p.x);
            float bm = p.y - bk * p.x;
            v.x = a.x;
            v.y = bk * v.x + bm;

            if ((v.x > std::min(p.x, q.x)) && (v.x < std::max(p.x, q.x)) &&
                (v.y > std::min(a.y, b.y)) && (v.y < std::max(a.y, b.y)))
            {
                return true;
            }
        }
        else if (q.x == p.x && b.x != a.x)
        {
            float am = a.y - ak * a.x;
            v.x = p.x;
            v.y = ak * v.x + am;

            if ((v.y > std::min(p.y, q.y)) && (v.y < std::max(p.y, q.y)) &&
                (v.x > std::min(a.x, b.x)) && (v.x < std::max(a.x, b.x)))
            {
                return true;
            }
        }
        return false;
    };

    const auto &v1 = poly.Vertices[0];
    const auto &v2 = poly.Vertices[1];
    const auto &v3 = poly.Vertices[2];
    return calc(v1, v2) or calc(v2, v3) or calc(v3, v1);
}

bool Polymap::pointinpolyedges(float x, float y, std::int32_t i)
{
    tvector2 u;
    float d;

    bool pointinpolyedges_result = false;

    auto &polygon = polys[i];

    u.x = x - polygon.vertices[0].x;
    u.y = y - polygon.vertices[0].y;
    d = perp[i][1].x * u.x + perp[i][1].y * u.y;
    if (d < 0)
    {
        return pointinpolyedges_result;
    }

    u.x = x - polygon.vertices[1].x;
    u.y = y - polygon.vertices[1].y;
    d = perp[i][2].x * u.x + perp[i][2].y * u.y;
    if (d < 0)
    {
        return pointinpolyedges_result;
    }

    u.x = x - polygon.vertices[2].x;
    u.y = y - polygon.vertices[2].y;
    d = perp[i][3].x * u.x + perp[i][3].y * u.y;
    if (d < 0)
    {
        return pointinpolyedges_result;
    }

    pointinpolyedges_result = true;
    return pointinpolyedges_result;
}

bool Polymap::pointinpoly(const tvector2 &p, const tmappolygon &poly)
{
    ZoneScopedN("pointinpoly");

    /*
      FIXME(skoskav): Explain what is going on here. Description from StackOverflow:

    Is the point p to the left of or to the right of both the lines AB and AC?
        If true, it can't be inside. If false, it is at least inside the "cones"
        that satisfy the condition. Now since we know that a point inside a trigon
        (triangle) must be to the same side of AB as BC (and also CA), we check if
                                                       they differ. If they do, p can't possibly be
        inside, otherwise p must be inside.

            Some keywords in the calculations are line half-planes and the determinant
                (2x2 cross product).
                Perhaps a more pedagogical way is probably to think of it as a point being
                    inside iff it's to the same side (left or right) to each of the lines AB,
                BC and CA.
                    */
    const auto &a = poly.vertices[0];
    const auto &b = poly.vertices[1];
    const auto &c = poly.vertices[2];

    float ap_x = p.x - a.x;
    float ap_y = p.y - a.y;

    bool p_ab = (b.x - a.x) * ap_y - (b.y - a.y) * ap_x > 0;
    bool p_ac = (c.x - a.x) * ap_y - (c.y - a.y) * ap_x > 0;

    if (p_ac == p_ab)
    {
        return false;
    }

    // p_bc <> p_ab
    if (((c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x) > 0) != p_ab)
    {
        return false;
    }
    return true;
}

bool Polymap::PointInPoly(const tvector2 &p, const PolyMapSector::Poly &poly)
{
    ZoneScopedN("PointInPoly");

    /*
      FIXME(skoskav): Explain what is going on here. Description from StackOverflow:

    Is the point p to the left of or to the right of both the lines AB and AC?
        If true, it can't be inside. If false, it is at least inside the "cones"
        that satisfy the condition. Now since we know that a point inside a trigon
        (triangle) must be to the same side of AB as BC (and also CA), we check if
                                                       they differ. If they do, p can't possibly be
    inside, otherwise p must be inside.

            Some keywords in the calculations are line half-planes and the determinant
                (2x2 cross product).
                Perhaps a more pedagogical way is probably to think of it as a point being
                    inside iff it's to the same side (left or right) to each of the lines AB,
                BC and CA.
                    */
    const auto &a = poly.Vertices[0];
    const auto &b = poly.Vertices[1];
    const auto &c = poly.Vertices[2];

    float ap_x = p.x - a.x;
    float ap_y = p.y - a.y;

    bool p_ab = (b.x - a.x) * ap_y - (b.y - a.y) * ap_x > 0;
    bool p_ac = (c.x - a.x) * ap_y - (c.y - a.y) * ap_x > 0;

    if (p_ac == p_ab)
    {
        return false;
    }

    // p_bc <> p_ab
    if (((c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x) > 0) != p_ab)
    {
        return false;
    }
    return true;
}

tvector2 Polymap::closestperpendicular(std::int32_t j, const tvector2 &pos, float &d,
                                       std::int32_t &n)
{
    std::array<float, 3> px, py;
    tvector2 p1, p2;
    float d1, d2, d3;
    std::int32_t edgev1, edgev2;

    tvector2 result;
    auto &polygon = polys[j];
    px[0] = polygon.vertices[0].x;
    py[0] = polygon.vertices[0].y;

    px[1] = polygon.vertices[1].x;
    py[1] = polygon.vertices[1].y;

    px[2] = polygon.vertices[2].x;
    py[2] = polygon.vertices[2].y;

    // find closest edge
    p1.x = px[0];
    p1.y = py[0];

    p2.x = px[1];
    p2.y = py[1];

    d1 = pointlinedistance(p1, p2, pos);
    d = d1;
    edgev1 = 1;
    edgev2 = 2;

    p1.x = px[1];
    p1.y = py[1];

    p2.x = px[2];
    p2.y = py[2];

    d2 = pointlinedistance(p1, p2, pos);

    if (d2 < d1)
    {
        edgev1 = 2;
        edgev2 = 3;
        d = d2;
    }

    p1.x = px[2];
    p1.y = py[2];

    p2.x = px[0];
    p2.y = py[0];

    d3 = pointlinedistance(p1, p2, pos);

    if ((d3 < d2) && (d3 < d1))
    {
        edgev1 = 3;
        edgev2 = 1;
        d = d3;
    }

    if ((edgev1 == 1) && (edgev2 == 2))
    {
        result = perp[j][1];
        n = 1;
    }

    if ((edgev1 == 2) && (edgev2 == 3))
    {
        result = perp[j][2];
        n = 2;
    }

    if ((edgev1 == 3) && (edgev2 == 1))
    {
        result = perp[j][3];
        n = 3;
    }
    return result;
}

template <typename T, typename K>
bool has(const T &arr, const K &value)
{
    return std::find(arr.begin(), arr.end(), value);
}

bool Polymap::collisiontest(const tvector2 &pos, tvector2 &perpvec, bool isflag)
{
    ZoneScopedN("CollisionTest");
    static constexpr std::array<std::int32_t, 9> excluded1 = {1, 2, 3, 11, 13, 15, 17, 24, 25};
    static constexpr std::array<std::int32_t, 3> excluded2 = {21, 22, 23};
    std::int32_t b = 0;
    float d = 0.0f;

    const auto &sector = GetSector(pos);

    if (!sector.IsValid())
    {
        return false;
    }
    for (const auto &w : sector.GetPolys())
    {
        if (!(has(excluded1, w.Type)) && (isflag || !(has(excluded2, w.Type))))
        {
            if (PointInPoly(pos, w))
            {
                perpvec = closestperpendicular(w.Index, pos, d, b);
                vec2scale(perpvec, perpvec, 1.5 * d);
                return true;
            }
        }
    }
    return false;
}

bool Polymap::collisiontestexcept(const tvector2 &pos, tvector2 &perpvec, std::int32_t c)
{
    static constexpr std::array<std::int32_t, 6> excluded = {1, 2, 3, 11, 24, 25};
    std::int32_t b = 0;
    float d = 0.0f;

    bool collisiontestexcept_result = false;
    const auto sector = GetSector(pos);

    if (!sector.IsValid())
    {
        return false;
    }
    for (const auto &w : sector.GetPolys())
    {
        if ((w.Index != c) && !(has(excluded, w.Type)))
        {
            if (PointInPoly(pos, w))
            {
                perpvec = closestperpendicular(w.Index, pos, d, b);
                vec2scale(perpvec, perpvec, 1.5 * d);
                collisiontestexcept_result = true;
                break;
            }
        }
    }
    return collisiontestexcept_result;
}
#ifdef NoOverflowCheck /*$Q+*/ /*$UNDEF NoOverflowCheck*/
#endif

bool Polymap::ShouldTestPolygonWithRay(const std::uint8_t polygonType, const bool npcol,
                                       const bool nbcol, const bool flag, const std::uint8_t team)
{
    switch (polygonType)
    {
    case poly_type_normal:
        return true;
    case poly_type_only_bullets:
        return !nbcol;
    case poly_type_only_player:
        return !npcol;
    case poly_type_doesnt:
    case poly_type_background:
    case poly_type_background_transition:
        return false;
    case poly_type_red_bullets:
        return !((team != Constants::TEAM_ALPHA) || nbcol);
    case poly_type_red_player:
        return !((team != Constants::TEAM_ALPHA) || npcol);
    case poly_type_blue_bullets:
        return !((team != Constants::TEAM_BRAVO) || nbcol);
    case poly_type_blue_player:
        return !((team != Constants::TEAM_BRAVO) || npcol);
    case poly_type_yellow_bullets:
        return !((team != Constants::TEAM_CHARLIE) || nbcol);
    case poly_type_yellow_player:
        return !((team != Constants::TEAM_CHARLIE) || npcol);
    case poly_type_green_bullets:
        return !((team != Constants::TEAM_DELTA) || nbcol);
    case poly_type_green_player:
        return !((team != Constants::TEAM_DELTA) || npcol);
    case poly_type_only_flaggers:
        return !(!flag || npcol);
    case poly_type_not_flaggers:
        return !(flag || npcol);
    case poly_type_non_flagger_collides:
        return !(!flag || npcol || nbcol);
    default:;
    }
    return true;
}

Polymap::SectorCoord Polymap::GetSectorCoordUnsafe(const tvector2 &pos)
{
    const std::int32_t kx = std::roundf((pos.x) * PositionToSectorScale);
    const std::int32_t ky = std::roundf((pos.y) * PositionToSectorScale);
    return {kx, ky};
}

std::int32_t Polymap::GetIndex(const SectorCoord &s)
{
    return (s.x + sectorsnum) * (2 * sectorsnum + 1) + (s.y + sectorsnum);
}

Polymap::SectorCoord Polymap::GetSectorCoord(const tvector2 &pos)
{
    if ((pos.x >= MapHalfSize || pos.x <= -MapHalfSize) ||
        (pos.y >= MapHalfSize || pos.y <= -MapHalfSize))
    {
        return SectorCoord::CreateInvalid();
    }
    return GetSectorCoordUnsafe(pos);
}

void Polymap::SetSectorsDivision(int32_t sectorsdivision)
{
    SectorsDivision = sectorsdivision;
    PositionToSectorScale = 1.0f / sectorsdivision;
    MapHalfSize = (float)sectorsdivision * ((float)sectorsnum + 0.5f);
}

Polymap::Sector Polymap::GetSector(const tvector2 &pos)
{
    const auto &coord = GetSectorCoord(pos);
    if (!coord.IsValid())
    {
        return Sector::CreateInvalid();
    }
    return {&Sectors[GetIndex(coord)].Polys};
}

bool Polymap::RayCastOpt(const tvector2 &a, const tvector2 &b, float &distance, float maxdist,
                         bool player, bool flag, bool bullet, bool checkcollider, std::uint8_t team)
{
    ZoneScopedN("PolyMap::RayCastOpt");
    std::int32_t i, j;
    tvector2 d;
    bool npcol, nbcol;
    float e, f, g, h, r;

    bool raycast_result = false;
    distance = vec2length(vec2subtract(a, b));
    if (distance > maxdist)
    {
        distance = 9999999;
        return true;
    }

    const auto c1 = GetSectorCoordUnsafe(a);
    const auto c2 = GetSectorCoordUnsafe(b);

    const auto kx = std::clamp(c1.x, -sectorsnum, sectorsnum);
    const auto ky = std::clamp(c1.y, -sectorsnum, sectorsnum);
    const auto zx = std::clamp(c2.x, -sectorsnum, sectorsnum);
    const auto zy = std::clamp(c2.y, -sectorsnum, sectorsnum);

    const auto ax = std::min(kx, zx);
    const auto ay = std::min(ky, zy);
    const auto bx = std::max(kx, zx);
    const auto by = std::max(ky, zy);

    npcol = !player;
    nbcol = !bullet;

    float ak = std::numeric_limits<float>::max();

    const auto lineWidth = 2 * sectorsnum + 1;
    const auto endx = (bx + sectorsnum) * lineWidth;
    for (i = (ax + sectorsnum) * lineWidth; i <= endx; i += lineWidth)
    {
        const auto jend = i + by + sectorsnum;
        for (j = i + ay + sectorsnum; j <= jend; j++)
        {
            ZoneScopedN("CheckSector");
            auto &polygons = Sectors[j].Polys;
            for (auto const &w : polygons)
            {
                ZoneScopedN("CheckPolygon");
                if (ShouldTestPolygonWithRay(w.Type, npcol, nbcol, flag, team))
                {
                    if (PointInPoly(a, w))
                    {
                        distance = 0.f;
                        return true;
                    }
                    if (ak == std::numeric_limits<float>::max() && b.x != a.x)
                    {
                        ak = ((b.y - a.y)) / (b.x - a.x);
                    }
                    if (LineInPoly(a, b, w, d, ak))
                    {
                        distance = vec2length(vec2subtract(d, a));
                        return true;
                    }
                }
            }
        }
    }

    if (checkcollider)
    {
        ZoneScopedN("CheckCollider");
        // check if vector crosses any colliders
        // |A*x + B*y + C| / Sqrt(A^2 + B^2) < r
        e = a.y - b.y;
        f = b.x - a.x;
        g = a.x * b.y - a.y * b.x;
        h = sqrt(e * e + f * f);
        for (i = 1; i <= collidercount; i++)
        {
            if (collider[i].active)
            {
                if (std::abs(e * collider[i].x + f * collider[i].y + g) / h <= collider[i].radius)
                {
                    r = sqrdist(a.x, a.y, b.x, b.y) + collider[i].radius * collider[i].radius;
                    if (sqrdist(a.x, a.y, collider[i].x, collider[i].y) <= r)
                        if (sqrdist(b.x, b.y, collider[i].x, collider[i].y) <= r)
                        {
                            raycast_result = false;
                            break;
                        }
                }
            }
        }
    }
    return raycast_result;
}

bool Polymap::raycast(const tvector2 &a, const tvector2 &b, float &distance, float maxdist,
                      bool player, bool flag, bool bullet, bool checkcollider, std::uint8_t team)
{

    auto retOpt = RayCastOpt(a, b, distance, maxdist, player, flag, bullet, checkcollider, team);
#if 0
    float dOld;
    auto retOld = RayCast(a, b, dOld, maxdist, player, flag, bullet, checkcollider, team);
    SoldatAssert(retOpt == retOld);
    SoldatAssert(std::abs(dOld - distance) < 0.001f);
#endif
    return retOpt;
}

// this should go inside TPolyMap, used only from Net.pas it seems
void Polymap::checkoutofbounds(MyFloat &x, MyFloat &y)
{
    if (x < (10 * (-sectorsnum * SectorsDivision) + 50))
        x = 1;
    else if (x > (10 * (sectorsnum * SectorsDivision) - 50))
        x = 1;

    if (y < (10 * (-sectorsnum * SectorsDivision) + 50))
        y = 1;
    else if (y > (10 * (sectorsnum * SectorsDivision) - 50))
        y = 1;
}

void Polymap::checkoutofbounds(std::int16_t &x, std::int16_t &y)
{
    if (x < (10 * (-sectorsnum * SectorsDivision) + 50))
        x = 1;
    else if (x > (10 * (sectorsnum * SectorsDivision) - 50))
        x = 1;

    if (y < (10 * (-sectorsnum * SectorsDivision) + 50))
        y = 1;
    else if (y > (10 * (sectorsnum * SectorsDivision) - 50))
        y = 1;
}
