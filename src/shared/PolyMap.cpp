// automatically converted
#include "PolyMap.hpp"
#include "Calc.hpp"
#include "Console.hpp"
#include "Constants.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"

extern tconsole mainconsole;
extern tpolymap map;

/*#include "Classes.h"*/
/*#include "SysUtils.h"*/
/*#include "Server.h"*/
/*#include "Client.h"*/
/*#include "Math.h"*/
/*#include "Calc.h"*/
/*#include "Game.h"*/
/*#include "Constants.h"*/

void tpolymap::initialize()
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

void tpolymap::loaddata(tmapfile &mapfile)
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
        if ((fabs(this->spawnpoints[i].x) >= 2000000) || (fabs(this->spawnpoints[i].y) >= 2000000))
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

bool tpolymap::loadmap(const tmapinfo &map)
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
        GetMainConsole().console(mapfile.mapname, game_message_color);
        result = true;
    }
    return result;
}
#ifndef SERVER
bool tpolymap::loadmap(tmapinfo map, bool bgforce, std::uint32_t bgcolortop,
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

#ifndef SERVER
        if (this->loadgraphics != nullptr)
        {
            this->loadgraphics(mapfile, bgforce, mapcolor(bgcolortop), mapcolor(bgcolorbtm));
        }
#endif

        GetMainConsole().console(mapfile.mapname, game_message_color);
        result = true;
    }
    return result;
}
#endif
bool tpolymap::lineinpoly(const tvector2 a, tvector2 b, std::int32_t poly, tvector2 &v)
{
    std::int32_t i, j;
    float ak, am, bk, bm;

    bool result = false;

    for (i = 1; i <= 3; i++)
    {
        if (i == 3)
            j = 1;
        else
            j = i + 1;

        tmapvertex &p = polys[poly].vertices[i];
        tmapvertex &q = polys[poly].vertices[j];

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

bool tpolymap::pointinpolyedges(float x, float y, std::int32_t i)
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

bool tpolymap::pointinpoly(const tvector2 p, const tmappolygon &poly)
{
    float ap_x;
    float ap_y;
    bool p_ab;
    bool p_ac;

    bool pointinpoly_result = false;

    /*
      FIXME(skoskav): Explain what is going on here. Description from StackOverflow:

      Is the point p to the left of or to the right of both the lines AB and AC?
      If true, it can't be inside. If false, it is at least inside the "cones"
      that satisfy the condition. Now since we know that a point inside a trigon
      (triangle) must be to the same side of AB as BC (and also CA), we check if
      they differ. If they do, p can't possibly be inside, otherwise p must be
      inside.

      Some keystd::uint64_ts in the calculations are line half-planes and the determinant
      (2x2 cross product).
      Perhaps a more pedagogical way is probably to think of it as a point being
      inside iff it's to the same side (left or right) to each of the lines AB,
      BC and CA.
    */

    NotImplemented(NITag::MAP);
    const tmapvertex &a = poly.vertices[1];
    const tmapvertex &b = poly.vertices[2];
    const tmapvertex &c = poly.vertices[3];

    ap_x = p.x - a.x;
    ap_y = p.y - a.y;

    p_ab = (b.x - a.x) * ap_y - (b.y - a.y) * ap_x > 0;
    p_ac = (c.x - a.x) * ap_y - (c.y - a.y) * ap_x > 0;

    if (p_ac == p_ab)
    {
        return pointinpoly_result;
    }

    // p_bc <> p_ab
    if (((c.x - b.x) * (p.y - b.y) - (c.y - b.y) * (p.x - b.x) > 0) != p_ab)
    {
        return pointinpoly_result;
    }

    pointinpoly_result = true;
    return pointinpoly_result;
}

tvector2 tpolymap::closestperpendicular(std::int32_t j, tvector2 pos, float &d, std::int32_t &n)
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

bool tpolymap::collisiontest(tvector2 pos, tvector2 &perpvec, bool isflag)
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

bool tpolymap::collisiontestexcept(tvector2 pos, tvector2 &perpvec, std::int32_t c)
{
    std::array<std::int32_t, 6> excluded = {1, 2, 3, 11, 24, 25};
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

bool tpolymap::raycast(const tvector2 a, tvector2 b, float &distance, float maxdist, bool player,
                       bool flag, bool bullet, bool checkcollider, std::uint8_t team)
{
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
            for (p = 1; p < sectors[i][j].polys.size(); p++)
            {
                auto &ref = sectors[i][j];
                w = ref.polys[p];

                testcol = true;
                if (((polytype[w] == poly_type_red_bullets) && ((team != team_alpha) || nbcol)) ||
                    ((polytype[w] == poly_type_red_player) && ((team != team_alpha) || npcol)))
                    testcol = false;
                if (((polytype[w] == poly_type_blue_bullets) && ((team != team_bravo) || nbcol)) ||
                    ((polytype[w] == poly_type_blue_player) && ((team != team_bravo) || npcol)))
                    testcol = false;
                if (((polytype[w] == poly_type_yellow_bullets) &&
                     ((team != team_charlie) || nbcol)) ||
                    ((polytype[w] == poly_type_yellow_player) && ((team != team_charlie) || npcol)))
                    testcol = false;
                if (((polytype[w] == poly_type_green_bullets) && ((team != team_delta) || nbcol)) ||
                    ((polytype[w] == poly_type_green_player) && ((team != team_delta) || npcol)))
                    testcol = false;
                if (((!flag || npcol) && (polytype[w] == poly_type_only_flaggers)) ||
                    ((flag || npcol) && (polytype[w] == poly_type_not_flaggers)))
                    testcol = false;
                if ((!flag || npcol || nbcol) && (polytype[w] == poly_type_non_flagger_collides))
                    testcol = false;
                if (((polytype[w] == poly_type_only_bullets) && nbcol) ||
                    ((polytype[w] == poly_type_only_player) && npcol) ||
                    (polytype[w] == poly_type_doesnt) || (polytype[w] == poly_type_background) ||
                    (polytype[w] == poly_type_background_transition))
                    testcol = false;
                if (testcol)
                {
                    if (pointinpoly(a, polys[w]))
                    {
                        distance = 0;
                        raycast_result = true;
                        return raycast_result;
                    }
                    if (lineinpoly(a, b, w, d))
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

// this should go inside TPolyMap, used only from Net.pas it seems

void checkoutofbounds(MyFloat &x, MyFloat &y)
{
    if (x < (10 * (-map.sectorsnum * map.sectorsdivision) + 50))
        x = 1;
    else if (x > (10 * (map.sectorsnum * map.sectorsdivision) - 50))
        x = 1;

    if (y < (10 * (-map.sectorsnum * map.sectorsdivision) + 50))
        y = 1;
    else if (y > (10 * (map.sectorsnum * map.sectorsdivision) - 50))
        y = 1;
}

void checkoutofbounds(std::int16_t &x, std::int16_t &y)
{
    if (x < (10 * (-map.sectorsnum * map.sectorsdivision) + 50))
        x = 1;
    else if (x > (10 * (map.sectorsnum * map.sectorsdivision) - 50))
        x = 1;

    if (y < (10 * (-map.sectorsnum * map.sectorsdivision) + 50))
        y = 1;
    else if (y > (10 * (map.sectorsnum * map.sectorsdivision) - 50))
        y = 1;
}
