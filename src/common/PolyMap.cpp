// automatically converted
#include "PolyMap.hpp"
#include "Calc.hpp"
#include "Constants.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include <Tracy.hpp>

void Polymap::initialize()
{
    std::int32_t i, j;

    this->mapid = 0;
    this->name = "";
    this->filename = "";
    this->sectorsdivision = 0;
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
    fillchar(&this->polytype[idx], sizeof(this->polytype), 0);
    fillchar(&this->perp[idx][idx], sizeof(this->perp), 0);
    fillchar(&this->bounciness[idx], sizeof(this->bounciness), 0);
    fillchar(&this->spawnpoints[idx], sizeof(this->spawnpoints), 0);
    fillchar(&this->collider[idx], sizeof(this->collider), 0);
    fillchar(&this->flagspawn[idx], sizeof(this->flagspawn), 0);

    for (i = this->sectors.StartIdx(); i < this->sectors.EndIdx(); i++)
    {
        for (j = this->sectors[i].StartIdx(); j < this->sectors[i].EndIdx(); j++)
        {
            NotImplemented(NITag::MAP);
            this->sectors[i][j].polys.clear();
        }
    }
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
    const auto &v1 = polygon.vertices[1];
    const auto &v2 = polygon.vertices[2];
    const auto &v3 = polygon.vertices[3];
    polygon.bk[0] = calc(v1, v2);
    polygon.bk[1] = calc(v2, v3);
    polygon.bk[2] = calc(v3, v1);
}

void Polymap::loaddata(tmapfile &mapfile)
{
    std::int32_t i, j, k;

    this->mapid = mapfile.hash;
    this->sectorsdivision = mapfile.sectorsdivision;
    this->sectorsnum = mapfile.sectorsnum;
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
        NotImplemented(NITag::MAP);
        std::memcpy(&botpath.waypoint[1], &mapfile.waypoints[0],
                    sizeof(twaypoint) * length(mapfile.waypoints));
    }

    for (i = 1; i <= this->polycount; i++)
    {
        this->polytype[i] = this->polys[i].polytype;

        this->perp[i][1].x = this->polys[i].normals[1].x;
        this->perp[i][1].y = this->polys[i].normals[1].y;
        this->perp[i][2].x = this->polys[i].normals[2].x;
        this->perp[i][2].y = this->polys[i].normals[2].y;
        this->perp[i][3].x = this->polys[i].normals[3].x;
        this->perp[i][3].y = this->polys[i].normals[3].y;

        this->bounciness[i] = vec2length(this->perp[i][3]); // gg

        vec2normalize(this->perp[i][1], this->perp[i][1]);
        vec2normalize(this->perp[i][2], this->perp[i][2]);
        vec2normalize(this->perp[i][3], this->perp[i][3]);

        if ((this->polytype[i] == poly_type_background) ||
            (this->polytype[i] == poly_type_background_transition))
        {
            this->backpolycount += 1;
            this->backpolys[this->backpolycount] = &this->polys[i];
        }

        Precompute(this->polys[i]);
    }

    k = 0;

    for (i = -this->sectorsnum; i <= this->sectorsnum; i++)
    {
        for (j = -this->sectorsnum; j <= this->sectorsnum; j++)
        {
            if (length(mapfile.sectors[k].polys) > 0)
            {
                NotImplemented(NITag::MAP);
                setlength(this->sectors[i][j].polys, length(mapfile.sectors[k].polys) + 1);
                std::memcpy(&this->sectors[i][j].polys[1], &mapfile.sectors[k].polys[0],
                            sizeof(std::uint16_t) * length(mapfile.sectors[k].polys));
            }

            k += 1;
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

bool LineInPolyOpt(const tvector2 &a, const tvector2 &b, const tmappolygon &poly, tvector2 &v,
                   const float ak)
{
    ZoneScopedN("LineInPolyOpt");

    auto calc = [&](const tmapvertex &p, const tmapvertex &q, const float bk) {
        if (b.x != a.x && q.x != p.x)
        {
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

    const auto &v1 = poly.vertices[1];
    const auto &v2 = poly.vertices[2];
    const auto &v3 = poly.vertices[3];
    return calc(v1, v2, poly.bk[0]) or calc(v2, v3, poly.bk[1]) or calc(v3, v1, poly.bk[2]);
}

bool Polymap::lineinpoly(const tvector2 &a, const tvector2 &b, const tmappolygon &poly, tvector2 &v)
{
    ZoneScopedN("LineInPoly");
    std::int32_t i, j;
    float ak, am, bk, bm;

    bool result = false;

    for (i = 1; i <= 3; i++)
    {
        if (i == 3)
            j = 1;
        else
            j = i + 1;

        const tmapvertex &p = poly.vertices[i];
        const tmapvertex &q = poly.vertices[j];

        if ((b.x != a.x) || (q.x != p.x))
        {
            if (b.x == a.x)
            {
                bk = ((q.y - p.y)) / (q.x - p.x);
                bm = p.y - bk * p.x;
                v.x = a.x;
                v.y = bk * v.x + bm;

                if ((v.x > std::min(p.x, q.x)) && (v.x < std::max(p.x, q.x)) &&
                    (v.y > std::min(a.y, b.y)) && (v.y < std::max(a.y, b.y)))
                {
                    result = true;
                    return result;
                }
            }
            else if (q.x == p.x)
            {
                ak = ((b.y - a.y)) / (b.x - a.x);
                am = a.y - ak * a.x;
                v.x = p.x;
                v.y = ak * v.x + am;

                if ((v.y > std::min(p.y, q.y)) && (v.y < std::max(p.y, q.y)) &&
                    (v.x > std::min(a.x, b.x)) && (v.x < std::max(a.x, b.x)))
                {
                    result = true;
                    return result;
                }
            }
            else
            {
                ak = ((b.y - a.y)) / (b.x - a.x);
                bk = ((q.y - p.y)) / (q.x - p.x);

                if (ak != bk)
                {
                    am = a.y - ak * a.x;
                    bm = p.y - bk * p.x;
                    v.x = ((bm - am)) / (ak - bk);
                    v.y = ak * v.x + am;

                    if ((v.x > std::min(p.x, q.x)) && (v.x < std::max(p.x, q.x)) &&
                        (v.x > std::min(a.x, b.x)) && (v.x < std::max(a.x, b.x)))
                    {
                        result = true;
                        return result;
                    }
                }
            }
        }
    }
    return result;
}

bool Polymap::pointinpolyedges(float x, float y, std::int32_t i)
{
    tvector2 u;
    float d;

    bool pointinpolyedges_result = false;

    u.x = x - polys[i].vertices[1].x;
    u.y = y - polys[i].vertices[1].y;
    d = perp[i][1].x * u.x + perp[i][1].y * u.y;
    if (d < 0)
    {
        return pointinpolyedges_result;
    }

    u.x = x - polys[i].vertices[2].x;
    u.y = y - polys[i].vertices[2].y;
    d = perp[i][2].x * u.x + perp[i][2].y * u.y;
    if (d < 0)
    {
        return pointinpolyedges_result;
    }

    u.x = x - polys[i].vertices[3].x;
    u.y = y - polys[i].vertices[3].y;
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
    ZoneScopedN("PointInPoly");

    /*
      FIXME(skoskav): Explain what is going on here. Description from StackOverflow:

      Is the point p to the left of or to the right of both the lines AB and AC?
      If true, it can't be inside. If false, it is at least inside the "cones"
      that satisfy the condition. Now since we know that a point inside a trigon
      (triangle) must be to the same side of AB as BC (and also CA), we check if
      they differ. If they do, p can't possibly be inside, otherwise p must be
      inside.

      Some keywords in the calculations are line half-planes and the determinant
      (2x2 cross product).
      Perhaps a more pedagogical way is probably to think of it as a point being
      inside iff it's to the same side (left or right) to each of the lines AB,
      BC and CA.
    */
    const tmapvertex &a = poly.vertices[1];
    const tmapvertex &b = poly.vertices[2];
    const tmapvertex &c = poly.vertices[3];

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

tvector2 Polymap::closestperpendicular(std::int32_t j, tvector2 pos, float &d, std::int32_t &n)
{
    std::array<float, 3> px, py;
    tvector2 p1, p2;
    float d1, d2, d3;
    std::int32_t edgev1, edgev2;

    tvector2 result;
    px[0] = polys[j].vertices[1].x;
    py[0] = polys[j].vertices[1].y;

    px[1] = polys[j].vertices[2].x;
    py[1] = polys[j].vertices[2].y;

    px[2] = polys[j].vertices[3].x;
    py[2] = polys[j].vertices[3].y;

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
    std::array<std::int32_t, 9> excluded1 = {1, 2, 3, 11, 13, 15, 17, 24, 25};
    std::array<std::int32_t, 3> excluded2 = {21, 22, 23};
    std::int32_t j, w;
    std::int32_t b = 0;
    float d = 0.0f;
    std::int32_t kx, ky;

    bool collisiontest_result = false;
    kx = round((pos.x) / sectorsdivision);
    ky = round((pos.y) / sectorsdivision);

    if ((kx > -sectorsnum) && (kx < sectorsnum) && (ky > -sectorsnum) && (ky < sectorsnum))
    {
        for (j = 1; j < sectors[kx][ky].polys.size(); j++)
        {
            w = sectors[kx][ky].polys[j];

            if (!(has(excluded1, polytype[w])) && (isflag || !(has(excluded2, polytype[w]))))
            {
                if (pointinpoly(pos, polys[w]))
                {
                    perpvec = closestperpendicular(w, pos, d, b);
                    vec2scale(perpvec, perpvec, 1.5 * d);
                    collisiontest_result = true;
                    break;
                }
            }
        }
    }
    return collisiontest_result;
}

bool Polymap::collisiontestexcept(const tvector2 &pos, tvector2 &perpvec, std::int32_t c)
{
    constexpr std::array<std::int32_t, 6> excluded = {1, 2, 3, 11, 24, 25};
    std::int32_t j, w;
    std::int32_t b = 0;
    float d = 0.0f;
    std::int32_t kx, ky;

    bool collisiontestexcept_result = false;
    kx = round((pos.x) / sectorsdivision);
    ky = round((pos.y) / sectorsdivision);

    if ((kx > -sectorsnum) && (kx < sectorsnum) && (ky > -sectorsnum) && (ky < sectorsnum))
    {
        for (j = 0; j < sectors[kx][ky].polys.size(); j++)
        {
            w = sectors[kx][ky].polys[j];

            if ((w != c) && !(has(excluded, polytype[w])))
            {
                if (pointinpoly(pos, polys[w]))
                {
                    perpvec = closestperpendicular(w, pos, d, b);
                    vec2scale(perpvec, perpvec, 1.5 * d);
                    collisiontestexcept_result = true;
                    break;
                }
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

bool Polymap::RayCastOpt(const tvector2 &a, const tvector2 &b, float &distance, float maxdist,
                         bool player, bool flag, bool bullet, bool checkcollider, std::uint8_t team)
{
    ZoneScopedN("PolyMap::RayCastOpt");
    std::int32_t i, j, ax, ay, bx, by;
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

    float s = 1.0f / sectorsdivision;

    ax = round((std::min(a.x, b.x)) * s);
    ay = round((std::min(a.y, b.y)) * s);
    bx = round((std::max(a.x, b.x)) * s);
    by = round((std::max(a.y, b.y)) * s);

    if ((ax > max_sectorz) || (bx < min_sectorz) || (ay > max_sectorz) || (by < min_sectorz))
    {
        return false;
    }

    ax = std::max(min_sectorz, ax);
    ay = std::max(min_sectorz, ay);
    bx = std::min(max_sectorz, bx);
    by = std::min(max_sectorz, by);

    npcol = !player;
    nbcol = !bullet;

    float ak = std::numeric_limits<float>::max();
    if (b.x != a.x)
    {
        ak = ((b.y - a.y)) / (b.x - a.x);
    }

    for (i = ax; i <= bx; i++)
    {
        for (j = ay; j <= by; j++)
        {
            ZoneScopedN("CheckSector");
            auto &polygons = sectors[i][j].polys;
            for (auto p = 1U; p < polygons.size(); p++)
            {
                ZoneScopedN("CheckPolygon");
                auto const &w = polygons[p];
                if (ShouldTestPolygonWithRay(polytype[w], npcol, nbcol, flag, team))
                {
                    auto &polygon = polys[w];
                    if (pointinpoly(a, polygon))
                    {
                        distance = 0.f;
                        return true;
                    }
                    if (LineInPolyOpt(a, b, polygon, d, ak))
                    {
                        tvector2 c = vec2subtract(d, a);
                        distance = vec2length(c);
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

bool Polymap::RayCastOld(const tvector2 &a, const tvector2 &b, float &distance, float maxdist,
                         bool player, bool flag, bool bullet, bool checkcollider, uint8_t team)
{
    ZoneScopedN("PolyMap::RayCast");
    std::int32_t i, j, ax, ay, bx, by, p, w;
    tvector2 c, d;
    bool testcol;
    bool npcol, nbcol;
    float e, f, g, h, r;

    bool raycast_result = false;
    distance = vec2length(vec2subtract(a, b));
    if (distance > maxdist)
    {
        distance = 9999999;
        raycast_result = true;
        return raycast_result;
    }

    ax = round((std::min(a.x, b.x)) / sectorsdivision);
    ay = round((std::min(a.y, b.y)) / sectorsdivision);
    bx = round((std::max(a.x, b.x)) / sectorsdivision);
    by = round((std::max(a.y, b.y)) / sectorsdivision);

    if ((ax > max_sectorz) || (bx < min_sectorz) || (ay > max_sectorz) || (by < min_sectorz))
    {
        return raycast_result;
    }

    ax = std::max(min_sectorz, ax);
    ay = std::max(min_sectorz, ay);
    bx = std::min(max_sectorz, bx);
    by = std::min(max_sectorz, by);

    npcol = !player;
    nbcol = !bullet;

    for (i = ax; i <= bx; i++)
    {
        for (j = ay; j <= by; j++)
        {
            ZoneScopedN("CheckSector");
            for (p = 1; p < sectors[i][j].polys.size(); p++)
            {
                ZoneScopedN("CheckPolygon");
                auto &ref = sectors[i][j];
                w = ref.polys[p];
                const auto &polygonType = polytype[w];

                testcol = true;
                if (polygonType == poly_type_normal)
                {
                }
                else if (((polygonType == poly_type_only_bullets) && nbcol) ||
                         ((polygonType == poly_type_only_player) && npcol) ||
                         (polygonType == poly_type_doesnt) ||
                         (polygonType == poly_type_background) ||
                         (polygonType == poly_type_background_transition))
                {
                    testcol = false;
                }
                else if (((polygonType == poly_type_red_bullets) &&
                          ((team != Constants::TEAM_ALPHA) || nbcol)) ||
                         ((polygonType == poly_type_red_player) &&
                          ((team != Constants::TEAM_ALPHA) || npcol)))
                {
                    testcol = false;
                }
                else if (((polygonType == poly_type_blue_bullets) &&
                          ((team != Constants::TEAM_BRAVO) || nbcol)) ||
                         ((polygonType == poly_type_blue_player) &&
                          ((team != Constants::TEAM_BRAVO) || npcol)))
                {
                    testcol = false;
                }
                else if (((polygonType == poly_type_yellow_bullets) &&
                          ((team != Constants::TEAM_CHARLIE) || nbcol)) ||
                         ((polygonType == poly_type_yellow_player) &&
                          ((team != Constants::TEAM_CHARLIE) || npcol)))
                {
                    testcol = false;
                }
                else if (((polygonType == poly_type_green_bullets) &&
                          ((team != Constants::TEAM_DELTA) || nbcol)) ||
                         ((polygonType == poly_type_green_player) &&
                          ((team != Constants::TEAM_DELTA) || npcol)))
                {
                    testcol = false;
                }
                else if (((!flag || npcol) && (polygonType == poly_type_only_flaggers)) ||
                         ((flag || npcol) && (polygonType == poly_type_not_flaggers)))
                {
                    testcol = false;
                }
                else if ((!flag || npcol || nbcol) &&
                         (polygonType == poly_type_non_flagger_collides))
                {
                    testcol = false;
                }

                if (testcol)
                {
                    if (pointinpoly(a, polys[w]))
                    {
                        distance = 0;
                        raycast_result = true;
                        return raycast_result;
                    }
                    if (lineinpoly(a, b, polys[w], d))
                    {
                        c = vec2subtract(d, a);
                        distance = vec2length(c);
                        raycast_result = true;
                        return raycast_result;
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
    auto retOld = RayCastOld(a, b, dOld, maxdist, player, flag, bullet, checkcollider, team);
    Assert(retOpt == retOld);
    Assert(std::abs(dOld - distance) < 0.001f);
#endif
    return retOpt;
}

// this should go inside TPolyMap, used only from Net.pas it seems
void Polymap::checkoutofbounds(MyFloat &x, MyFloat &y)
{
    if (x < (10 * (-sectorsnum * sectorsdivision) + 50))
        x = 1;
    else if (x > (10 * (sectorsnum * sectorsdivision) - 50))
        x = 1;

    if (y < (10 * (-sectorsnum * sectorsdivision) + 50))
        y = 1;
    else if (y > (10 * (sectorsnum * sectorsdivision) - 50))
        y = 1;
}

void Polymap::checkoutofbounds(std::int16_t &x, std::int16_t &y)
{
    if (x < (10 * (-sectorsnum * sectorsdivision) + 50))
        x = 1;
    else if (x > (10 * (sectorsnum * sectorsdivision) - 50))
        x = 1;

    if (y < (10 * (-sectorsnum * sectorsdivision) + 50))
        y = 1;
    else if (y > (10 * (sectorsnum * sectorsdivision) - 50))
        y = 1;
}
