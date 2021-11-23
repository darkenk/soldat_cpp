// automatically converted

#include "Bullets.hpp"

#ifdef SERVER
#include "../../server/Server.hpp"
#include "../network/NetworkServer.hpp"
#include "../network/NetworkServerBullet.hpp"
#else
#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameRendering.hpp"
#include "../../client/Gfx.hpp"
#include "../../client/Sound.hpp"
#include "../network/NetworkClientBullet.hpp"
#endif
#include "../Calc.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../gfx.hpp"
#include "common/Logging.hpp"
#include "common/Util.hpp"
#include <limits>
#include <numbers>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

using std::numbers::pi;

template <Config::Module M>
std::int32_t createbullet(tvector2 spos, tvector2 svelocity, std::uint8_t snum, std::int32_t sowner,
                          std::uint8_t n, float hitm, bool net, bool mustcreate,
                          std::uint16_t seed) // Seed = -1
{
    std::int32_t i, j;
    float mass;
    std::int16_t weaponindex;
    std::uint8_t sstyle;

    std::int32_t result;
    result = -1;

#ifdef SERVER
    LogTraceG("CreateBullet {} {} {}", snum, sowner, n);
#endif

#ifndef SERVER
    if (demoplayer.active())
        if (!mustcreate)
            return result;
#endif

    weaponindex = weaponnumtoindex(snum);
    sstyle = guns[weaponindex].bulletstyle;

    if (!mustcreate && (sowner > 0))
    {
        if (
#ifndef SERVER
            (sowner != mysprite) &&
#endif
            (sprite[sowner].player->controlmethod != bot) &&
            ((sprite[sowner].weapon.fireinterval > fireinterval_net) ||
             (((sstyle == bullet_style_fragnade) || (sstyle == bullet_style_clusternade))
#ifndef SERVER
              && false
#endif
              )))
        {
#ifndef SERVER
            if (bulletcansend(spos.x, spos.y, camerafollowsprite, svelocity.x))
#endif
            {
                return result;
            }
        }
    }

#ifdef SERVER
    LogTraceG("CreateBullet 2");
#endif

    if (n == 255)
    {
        for (i = 1; i <= max_bullets + 1; i++)
        {
            if (i == max_bullets + 1)
            {
                result = -1;
                return result;
            }
            if (!bullet[i].active)
                break;
        }
    }
    else
    {
        i = n;
    }

    // i is now the active sprite
#ifdef SERVER
    LogTraceG("CreateBullet 3");
#endif
    // activate sprite
    bullet[i].active = true;
#ifndef SERVER
    bullet[i].hashit = false;
#endif
    bullet[i].style = sstyle;
    bullet[i].num = i;
    bullet[i].owner = sowner;
    bullet[i].timeout = guns[weaponindex].timeout;
#ifndef SERVER // TODO: Check if this should be used also in server
    bullet[i].timeoutprev = guns[weaponindex].timeout;
#endif
    bullet[i].hitmultiply = hitm;
#ifndef SERVER // TODO: Check if this should be used also in server
    bullet[i].hitmultiplyprev = hitm;
#endif
    bullet[i].whizzed = false;

    if (
#ifndef SERVER
        (sowner == camerafollowsprite) ||
#endif
        (sstyle == bullet_style_flamearrow) || (sstyle == bullet_style_flame))
        bullet[i].whizzed = true;

    if (sprite[sowner].player->controlmethod == human)
    {
        bullet[i].ownerpingtick = sprite[sowner].player->pingticksb;
#ifdef SERVER
        bullet[i].ownerpingtick += pingticksadd;
#endif
    }
    else
        bullet[i].ownerpingtick = 0;

    bullet[i].ownerweapon = snum;
    bullet[i].hitbody = 0;
    bullet[i].hitspot.x = 0;
    bullet[i].hitspot.y = 0;
    bullet[i].tracking = 0;

#ifndef SERVER // TODO: Check if this should be used also in server
    if (sprite[sowner].aimdistcoef < defaultaimdist)
        bullet[i].tracking = 255;

    bullet[i].imagestyle = sprite[sowner].weapon.bulletimagestyle;
#else
    bullet[i].initial = spos;
#endif
    bullet[i].startuptime = maintickcounter;
    bullet[i].ricochetcount = 0;
#ifndef SERVER // TODO: Check if this should be used also in server
    bullet[i].degradecount = 0;
    bullet[i].pingadd = 0;
    bullet[i].pingaddstart = 0;
#endif

    if (seed == std::numeric_limits<std::uint16_t>::max())
    {
        if (sprite[sowner].bulletcount == std::numeric_limits<std::uint16_t>::max())
            sprite[sowner].bulletcount = 0;
        else
            sprite[sowner].bulletcount += 1;
        seed = sprite[sowner].bulletcount;
    }
    bullet[i].seed = seed;

#ifdef SERVER
    LogTraceG("CreateBullet 4");
#endif

    mass = 1.0;

    if (!mustcreate)
        if (sstyle == bullet_style_flame)
        {
            spos.x = spos.x + svelocity.x;
            spos.y = spos.y + svelocity.y;
        }

    bullet[i].initial = spos;

#ifdef SERVER
    LogTraceG("CreateBullet 5");
#endif
    // activate sprite part
    bulletparts.createpart(spos, svelocity, mass, i);

#ifndef SERVER

    // SEND BULLLET THROUGH NETWORK
    if (net)
    {
        if ((sowner == mysprite) && (clientstopmovingcounter > 0))
        {
            if ((sprite[sowner].weapon.fireinterval > fireinterval_net) || mustcreate ||
                (bullet[i].style == bullet_style_fragnade) ||
                (bullet[i].style == bullet_style_clusternade) ||
                (bullet[i].style == bullet_style_cluster))
            {
                clientsendbullet(i);
                // Damage multiplier hack was here, they recalled ClientSendBullet
            }
            else if ((sprite[sowner].weapon.fireinterval <= fireinterval_net) &&
                     (sprite[sowner].burstcount == 0) &&
                     (maintickcounter > lastforceclientspritesnapshotmovtick + fireinterval_net))
            {
                forceclientspritesnapshotmov = true;
                lastforceclientspritesnapshotmovtick = maintickcounter;
            }
        }
    }

    if (sowner == mysprite)
    {
        if (bullet[i].style == bullet_style_fragnade)
        {
            if (wepstats[18].shots == 0)
                wepstats[18].textureid = GFX::INTERFACE_NADE;
            wepstats[18].shots = wepstats[18].shots + 1;
            if (wepstatsnum == 0)
                wepstatsnum = 1;
            if (wepstats[18].name == "")
                wepstats[18].name = "Grenade";
        }
        else if ((bullet[i].style == bullet_style_cluster) ||
                 (bullet[i].style == bullet_style_clusternade))
        {
            if (wepstats[19].shots == 0)
                wepstats[19].textureid = GFX::INTERFACE_CLUSTER_NADE;
            wepstats[19].shots = wepstats[19].shots + 1;
            if (wepstatsnum == 0)
                wepstatsnum = 1;
            if (wepstats[19].name == "")
                wepstats[19].name = "Clusters";
        }
        else if (bullet[i].style == bullet_style_m2)
        {
            if (wepstats[20].shots == 0)
                wepstats[20].textureid = GFX::INTERFACE_GUNS_M2;
            wepstats[20].shots = wepstats[20].shots + 1;
            if (wepstatsnum == 0)
                wepstatsnum = 1;
            if (wepstats[20].name == "")
                wepstats[20].name = "Stationary gun";
        }
        else if (bullet[i].style == bullet_style_punch)
        {
            if (wepstats[17].shots == 0)
                wepstats[17].textureid = GFX::INTERFACE_GUNS_FIST;
            wepstats[17].shots = wepstats[17].shots + 1;
            if (wepstatsnum == 0)
                wepstatsnum = 1;
            if (wepstats[17].name == "")
                wepstats[17].name = "Hands";
        }
        else
        {
            j = sprite[mysprite].weapon.num;
            if (j == guns[noweapon].num)
                j = 17;

            if (wepstats[j].shots == 0)
            {
                switch (j)
                {
                case 0:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_SOCOM;
                    break;
                case 1:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_DEAGLES;
                    break;
                case 2:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_MP5;
                    break;
                case 3:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_AK74;
                    break;
                case 4:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_STEYR;
                    break;
                case 5:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_SPAS;
                    break;
                case 6:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_RUGER;
                    break;
                case 7:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_M79;
                    break;
                case 8:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_BARRETT;
                    break;
                case 9:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_MINIMI;
                    break;
                case 10:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_MINIGUN;
                    break;
                case 11:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_KNIFE;
                    break;
                case 12:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_CHAINSAW;
                    break;
                case 13:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_LAW;
                    break;
                case 14:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_FLAMER;
                    break;
                case 15:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_BOW;
                    break;
                case 16:
                    wepstats[j].textureid = GFX::INTERFACE_GUNS_BOW;
                    break;
                }
            }
            wepstats[j].shots = wepstats[j].shots + 1;
            if (wepstatsnum == 0)
                wepstatsnum = 1;
            if (wepstats[j].name == "")
                wepstats[j].name = sprite[mysprite].weapon.name;
        }
    }
#else
    if (net)
    {
        LogTraceG("CreateBullet 6");

        if (sprite[sowner].weapon.fireinterval > fireinterval_net)
            serverbulletsnapshot(i, 0, false);
        else
            for (j = 1; j <= max_sprites; j++)
                if (sprite[j].active && (sprite[j].player->controlmethod == human) &&
                    (j != bullet[i].owner) && (sprite[j].player->port == 0) &&
                    sprite[j].isspectator())
                    serverbulletsnapshot(i, j, false);
    }
#endif

    // Bullet[i].Update;
#ifdef SERVER
    LogTraceG("CreateBullet 7");
#endif

    result = i;
    return result;
}
#ifdef SERVER
std::int32_t servercreatebullet(tvector2 spos, tvector2 svelocity, std::uint8_t snum,
                                std::int32_t sowner, std::uint8_t n, float hitm, bool net)
{
    std::int32_t i;

    std::int32_t result;
    result = -1;

    if ((sowner <= 0) || (sowner >= max_sprites))
        return result;

    i = createbullet(spos, svelocity, snum, sowner, n, hitm, net, true, 0);

    bullet[i].ownerpingtick = 0;
    bullet[i].dontcheat = true;

    if (net)
        serverbulletsnapshot(i, 0, true);

    result = i;
    return result;
}
#endif

template <Config::Module M>
bool bulletcansend(float x, float y, std::int32_t i, float vx)
{
    float sx, sy;

    bool result;
    LogTraceG("BulletCanSend");
    result = false;

    if ((i > max_players) || (i < 0))
        return result;

    if (i == 0)
    {
        result = true;
        return result;
    }

    sx = spriteparts.pos[i].x - ((float)((spriteparts.pos[i].x - sprite[i].control.mouseaimx)) / 2);
    sy = spriteparts.pos[i].y - ((float)((spriteparts.pos[i].y - sprite[i].control.mouseaimy)) / 2);
#ifdef SERVER
    if ((x > (sx - max_game_width)) && (x < (sx + max_game_width)) && (y > (sy - 480)) &&
        (y < (sy + 480)))
        result = true;
    if ((x > (sx - 1640)) && (x < (sx + 1640)) && (y > (sy - 680)) && (y < (sy + 680)))
        if (((sx < x) && (vx < 0)) || ((sx > x) && (vx > 0)))
            result = true;
#else
    if ((x > (sx - gamewidth)) && (x < (sx + gamewidth)) && (y > (sy - gameheight)) &&
        (y < (sy + gameheight)))
        result = true;

    if ((x > (sx - 1640)) && (x < (sx + 1640)) && (y > (sy - 680)) && (y < (sy + 680)))
        if (((sx < x) && (vx < 0)) || ((sx > x) && (vx > 0)))
            result = true;
#endif
    return result;
}

template bool bulletcansend<Config::GetModule()>(float x, float y, std::int32_t i, float vx);

#ifndef SERVER
bool canhitspray(std::int32_t victim, std::int32_t attacker)
{
    bool result;
    result = false;

    if (victim == mysprite)
    {
        if ((victim == attacker) or (CVar::sv_friendlyfire) or sprite[victim].issolo() or
            sprite[victim].isnotinsameteam(sprite[attacker]))
        {
            result = true;
        }
    }
    return result;
}

void hitspray()
{
    std::int16_t bink;

    bink = sprite[mysprite].weapon.bink;
    if (bink > 0)
        hitspraycounter = calculatebink(hitspraycounter, bink);
}

void calculaterecoil(float px, float py, float &cx, float &cy, float da)
{
    float dx, dy, radius, alpha, displacementx;
    // M: TVector2;

    displacementx = 0;
    // If cursor is on right side displace recoil circle to left,
    // otherwise to right
    if (cx > px) // Test to find out best Result for displacement_x
    {
        px = px - displacementx;
    }
    else
    {
        px = px + displacementx;
    }

    // Calculate delta x/y and radius
    dx = cx - px;
    dy = cy - py;
    radius = sqrt(dx * dx + dy * dy);

    // Depending on is cursor is on left/right side do different things
    if (dx > 0)
    {
        alpha = atan((float)(dy) / dx);
        alpha = alpha + da;

        if (alpha > pi / 2)
        {
            alpha = pi / 2;
        }
    }
    else
    {
        alpha = atan((float)(dy) / dx) + pi;
        alpha = alpha - da;
        if (alpha < pi / 2)
        {
            alpha = pi / 2;
        }
    }

    // Calculate new x/y for cursor
    // cx := (px + Cos(alpha) * radius);

    // If Soldat uses a coordinate system where y < 0 at top instead of bottom,
    // this needs subtraction instead of addition
    // cy := (py - Sin(alpha) * radius);

    // M.X := (dx / Sprite[CameraFollowSprite].AimDistCoef) * 3;
    // M.Y := (dy / Sprite[CameraFollowSprite].AimDistCoef) * 3;

    cx = px + ((dx + cos(alpha) * radius) / sprite[camerafollowsprite].aimdistcoef) *
                  ((float)(sprite[camerafollowsprite].aimdistcoef) / 2);
    cy = py + ((dy + sin(alpha) * radius) / sprite[camerafollowsprite].aimdistcoef) *
                  ((float)(sprite[camerafollowsprite].aimdistcoef) / 2);

    // Lastly make sure that cursor is still at the same side as it was first
    if (dx > 0)
    {
        if (cx <= px)
        {
            cx = (px + 0.0001); // We add a floatly small value like 0.001 to
                                // distinguish that the cursos is on right side
                                // (if it was right in the middle it'd be possible to
                                // interpret as it being on left or right side)
        }
    }
    else
    {
        if (cx >= px)
        {
            cx = (px - 0.0001);
        }
    }
}
#endif

// TBULLET
template <Config::Module M>
void Bullet<M>::update()
{
    tvector2 oldv, a;
    float dist;
    tvector2 oldp, oldop;
    tvector2 hitp, hitp2, hitp3;

#ifdef SERVER
    LogTraceG("TBullet.Update");
#endif

    timeoutprev = timeout;
    hitmultiplyprev = hitmultiply;
    velocityprev = bulletparts.velocity[num];

    oldv = bulletparts.velocity[num];
    oldp = bulletparts.pos[num];
    oldop = bulletparts.oldpos[num];
    dist = -1;
    hitp2.x = 0;
    hitp3.x = 0;

    checkoutofbounds();

    // check collision with map
    if (style != bullet_style_fragnade)
        hitp = checkmapcollision(bulletparts.pos[num].x, bulletparts.pos[num].y);
    else
    {
        hitp = checkmapcollision(bulletparts.pos[num].x, bulletparts.pos[num].y - 2);
        hitp = checkmapcollision(bulletparts.pos[num].x, bulletparts.pos[num].y);
    }

    if (!active)
    {
        a = vec2subtract(hitp, oldop);
        dist = vec2length(a);

        bulletparts.velocity[num] = oldv;
        ricochetcount -= 1;
        bulletparts.pos[num] = oldp;
        bulletparts.oldpos[num] = oldop;
    }

    // check if hit collider
    hitp2 = checkcollidercollision(dist);

    if (!active)
    {
        if (hitp2.x == 0)
            a = vec2subtract(hitp, oldop);
        else
            a = vec2subtract(hitp2, oldop);

        dist = vec2length(a);
        bulletparts.velocity[num] = oldv;
        bulletparts.pos[num] = oldp;
        bulletparts.oldpos[num] = oldop;
    }

    // check if hit sprites
    hitp3 = checkspritecollision(dist);

    if (!active)
    {
        if (hitp3.x == 0)
        {
            if (hitp2.x == 0)
                a = vec2subtract(hitp, oldop);
            else
                a = vec2subtract(hitp2, oldop);
        }
        else
            a = vec2subtract(hitp3, oldop);

        dist = vec2length(a);
    }

    // check if hit things
    hitp = checkthingcollision(dist);

    // count Time Out
    timeout -= 1;
    if (timeout == 0)
    {
        switch (style)
        {
        case bullet_style_plain:
        case bullet_style_shotgun:
        case bullet_style_flame:
        case bullet_style_punch:
        case bullet_style_arrow:
        case bullet_style_clusternade:
        case bullet_style_knife:
        case bullet_style_thrownknife:
            kill();
            break;
        case bullet_style_fragnade:
        case bullet_style_m79:
        case bullet_style_flamearrow:
        case bullet_style_law: {
            hit(hit_type_fragnade);
            kill();
        }
        break;
        case bullet_style_cluster: {
            hit(hit_type_cluster);
            kill();
        }
        break;
        case bullet_style_m2: {
            hit(hit_type_flak);
            kill();
        }
        break;
        } // case
    }     // TimeOut = 0

    // lose power on distance
    if (timeout % 6 == 0)
    {
        if ((ownerweapon != guns[barrett].num) && (ownerweapon != guns[m79].num) &&
            (ownerweapon != guns[knife].num) && (ownerweapon != guns[law].num))
        {
            a = vec2subtract(initial, bulletparts.pos[num]);
            dist = vec2length(a);

            if (degradecount == 0)
            {
                if (dist > 500)
                {
                    hitmultiply = hitmultiply * 0.5;
                    degradecount += 1;
                }
            }
            else if (degradecount == 1)
            {
                if (dist > 900)
                {
                    hitmultiply = hitmultiply * 0.5;
                    degradecount += 1;
                }
            }
        }
    }

#ifndef SERVER
    // Bullet Tracking
    if (owner == mysprite)
        if (tracking == 255)
            tracking = owner;

    if (tracking == mysprite)
    {
        if (sprite[owner].position != pos_stand)
        {
            camerax = bulletparts.pos[num].x + 5 * bulletparts.velocity[num].x;
            cameray = bulletparts.pos[num].y + 5 * bulletparts.velocity[num].y;
        }
        else
        {
            tracking = 0;
        }
    }

    // his sound
    if ((timeout == bullet_timeout - 25) && (style != bullet_style_shotgun))
        playsound(sfx_bulletby, bulletparts.pos[num]);

    // whiizz above head
    if (!whizzed)
        if (style != bullet_style_punch)
            if (camerafollowsprite > 0)
                if ((bulletparts.pos[num].x > spriteparts.pos[camerafollowsprite].x - 200) &&
                    (bulletparts.pos[num].x < spriteparts.pos[camerafollowsprite].x + 200) &&
                    (bulletparts.pos[num].y > spriteparts.pos[camerafollowsprite].y - 350) &&
                    (bulletparts.pos[num].y < spriteparts.pos[camerafollowsprite].y + 100))
                {
                    playsound(sfx_bulletby2 + Random(4), bulletparts.pos[num]);
                    whizzed = true;
                }

    // fire for flaming arrow
    if (style == bullet_style_flamearrow)
    {
        // smoke
        if (Random(2) == 0)
            createspark(bulletparts.pos[num], vector2(0, -0.5), 37, num, 40);

        if (Random(2) == 0)
            createspark(bulletparts.pos[num], vector2(0, -0.5), 36, num, 40);
    }

    // law missile smoke
    if (style == bullet_style_law)
    {
        // smoke
        createspark(bulletparts.pos[num], vector2(0, -1.5), 59, num, 50);
        if (Random(2) == 0)
            createspark(bulletparts.pos[num], bulletparts.velocity[num], 2, num, 5);
    }
#endif

    // flame
    if (style == bullet_style_flame)
        bulletparts.forces[num].y = bulletparts.forces[num].y - 0.15;

#ifndef SERVER
    // bleed
    if (hitbody > 0)
        if (Random(5) == 0)
        {
            createspark(
                bulletparts.pos[num],
                vector2(bulletparts.velocity[num].x * 0.5, bulletparts.velocity[num].y * 0.5), 4,
                owner, 90);
        }
#endif
}

#ifndef SERVER
template <Config::Module M>
void Bullet<M>::render(double timeelapsed)
{
    tvector2 bulletpos, bulletvel;
    tvector2 _p, _p2, _scala, a, b;
    float roto, alfa;
    float grenvel = 0.0;
    float ox, oy;
    float sinusvar;

    tgfxspritearray &t = textures;
    bulletpos = bulletparts.pos[num];

    if (CVar::sv_realisticmode)
        if ((owner > 0) && (owner < max_sprites + 1))
            if (sprite[owner].active)
                if (sprite[owner].visible == 0)
                    if (map.raycast(bulletpos, sprite[mysprite].skeleton.pos[9], grenvel, gamewidth,
                                    true))
                        return;

    bulletvel = bulletparts.velocity[num];
    sinusvar = sin(timeoutfloat + 5.1 * timeelapsed);

    switch (style)
    {
    case bullet_style_plain: // bullet
        if (timeoutfloat < bullet_timeout - 2)
        {
            if ((imagestyle != guns[eagle].bulletimagestyle) &&
                (imagestyle != guns[mp5].bulletimagestyle) &&
                (imagestyle != guns[ak74].bulletimagestyle) &&
                (imagestyle != guns[steyraug].bulletimagestyle) &&
                (imagestyle != guns[ruger77].bulletimagestyle) &&
                (imagestyle != guns[barrett].bulletimagestyle) &&
                (imagestyle != guns[m249].bulletimagestyle) &&
                (imagestyle != guns[minigun].bulletimagestyle) &&
                (imagestyle != guns[colt].bulletimagestyle))
                imagestyle = guns[colt].bulletimagestyle;

            _p.x = bulletpos.x + bulletvel.x;
            _p.y = bulletpos.y + bulletvel.y;
            _p2.x = _p.x - bulletvel.x;
            _p2.y = _p.y - bulletvel.y;
            _scala.x = (float)(vec2length(bulletvel)) / bullettrail;
            _scala.y = 1;
            roto = -angle2points(_p, _p2);

            alfa = ((hitmultiply * _scala.x * _scala.x) / 4.63) * 255;

            if (alfa > 230)
                alfa = 230;
            if (alfa < 50)
                alfa = 50;

            if (pingadd < 1)
                gfxdrawsprite(t[imagestyle], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                              rgba(0xffffff, round(alfa)));

            if (pingadd > 0)
            {
                a.x = bulletpos.x - initial.x;
                a.y = bulletpos.y - initial.y;
                b.x = vec2length(a) * min((float)(1) / bulletlength,
                                          ((float)((pingadd + 2)) / pingaddstart) / bullettrail);
                b.y = 1;

                if (active)
                    alfa = (float)(alfa) / 6;
                else
                    alfa = (float)(alfa) / 4;
                gfxdrawsprite(t[imagestyle], _p.x, _p.y, b.x, b.y, 0, 0, roto,
                              rgba(0xffffff, round(alfa)));
            }

            if (hitbody > 0)
            {
                if (trails == 1)
                    if (timeoutfloat < bullet_timeout - 7)
                    {
                        _p.x = bulletpos.x;
                        _p.y = bulletpos.y;
                        _scala.x = fabs((float)(vec2length(bulletvel)) / 4);
                        _scala.y = 1;
                        gfxdrawsprite(t[imagestyle], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                                      rgba(0xffdddd, bulletalpha / 2));
                    }
            }
            else
            {
                if (trails == 1)
                {
                    if (timeoutfloat < bullet_timeout - 7)
                    {
                        _p.x = bulletpos.x;
                        _p.y = bulletpos.y;
                        _scala.x = fabs(vec2length(bulletvel) / 3.5);
                        _scala.y = 1;
                        gfxdrawsprite(t[imagestyle], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                                      rgba(0xffffff, bulletalpha / 2));
                    }
                }
            }
        }
        break;

    case bullet_style_fragnade: {
        grenvel = vec2length(bulletvel);
        if (trails == 1)
            if (timeoutfloat < grenade_timeout - 3)
            {
                if (bulletvel.y > 0)
                    ox = -1;
                else
                    ox = 1;
                if (bulletvel.x > 0)
                    oy = 1;
                else
                    oy = -1;

                _p.x = bulletpos.x + ox;
                _p.y = bulletpos.y - 3 + oy;
                _p2.x = _p.x - bulletvel.x;
                _p2.y = _p.y - bulletvel.y;
                roto = -angle2points(_p, _p2);
                _scala.x = (float)(grenvel) / 3;
                _scala.y = 1;
                gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                              rgba(0x64ff64, round(bulletalpha * 0.75)));
            }

        _p.x = bulletpos.x - 1;
        _p.y = bulletpos.y - 4;
        _p2.x = bulletparts.oldpos[num].x - 1;
        _p2.y = bulletparts.oldpos[num].y - 4;
        gfxdrawsprite(t[GFX::WEAPONS_FRAG_GRENADE], _p.x, _p.y);
    }
    break;

    case bullet_style_shotgun:
        if (timeoutfloat < bullet_timeout - 2)
        {
            _p.x = bulletpos.x + bulletvel.x;
            _p.y = bulletpos.y + bulletvel.y;
            _p2.x = _p.x - bulletvel.x;
            _p2.y = _p.y - bulletvel.y;
            roto = -angle2points(_p, _p2);
            gfxdrawsprite(t[GFX::WEAPONS_SPAS_BULLET], _p.x, _p.y, 0, 0, roto, rgba(0xffffff, 150));

            if (trails == 1)
                if (timeoutfloat < bullet_timeout - 3)
                {
                    _p.x = bulletpos.x;
                    _p.y = bulletpos.y;
                    _scala.x = fabs((float)(vec2length(bulletvel)) / 9);
                    _scala.y = 1;
                    gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                                  roto, rgba(0xffffff, bulletalpha / 5));
                }
        }
        break;

    case bullet_style_m79:
        if (timeoutfloat < bullet_timeout - 2)
        {
            _p.x = bulletpos.x;
            _p.y = bulletpos.y + 1;
            _p2.x = _p.x - bulletvel.x;
            _p2.y = _p.y - bulletvel.y;
            _scala.x = 1;
            _scala.y = 1;
            roto = -angle2points(_p, _p2);
            gfxdrawsprite(t[GFX::WEAPONS_M79_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                          degtorad(timeoutfloat * 6), rgba(0xffffff, 252));

            if (trails == 1)
                if (timeoutfloat < bullet_timeout - 4)
                {
                    if (bulletvel.y > 0)
                        ox = -1;
                    else
                        ox = 1;
                    if (bulletvel.x > 0)
                        oy = 1;
                    else
                        oy = -1;

                    _p.x = bulletpos.x + ox;
                    _p.y = bulletpos.y + oy;
                    _scala.x = fabs((float)(vec2length(bulletvel)) / 4);
                    _scala.y = 1.3;
                    gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                                  roto, rgba(0xffff55, bulletalpha));
                }
        }
        break;

    case bullet_style_flame: {
        _p.x = bulletpos.x - 8;
        _p.y = bulletpos.y - 17;
        if ((timeoutfloat > 0) && (timeoutfloat <= flamer_timeout))
            gfxdrawsprite(
                t[GFX::SPARKS_FLAMES_EXPLODE16 - (std::int32_t)trunc((float)(timeoutfloat) / 2)],
                _p.x, _p.y);
    }
    break;

    case bullet_style_arrow:
        if (timeoutfloat < bullet_timeout - 2)
        {
            _p.x = bulletpos.x + bulletvel.x;
            _p.y = bulletpos.y + bulletvel.y;
            _p2.x = _p.x - bulletvel.x;
            _p2.y = _p.y - bulletvel.y;
            roto = -angle2points(_p, _p2);
            gfxdrawsprite(t[GFX::WEAPONS_ARROW], _p.x, _p.y, 0, 0, roto);

            if (trails == 1)
                if (timeoutfloat > arrow_resist)
                {
                    _p.x = bulletpos.x;
                    _p.y = bulletpos.y;
                    _scala.x = fabs((float)(vec2length(bulletvel)) / 3);
                    _scala.y = 1;
                    gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                                  roto, rgba(0xffffff, bulletalpha / 7));
                }
        }
        break;

    case bullet_style_flamearrow:
        if (timeoutfloat < bullet_timeout - 2)
        {
            _p.x = bulletpos.x + bulletvel.x;
            _p.y = bulletpos.y + bulletvel.y;
            _p2.x = _p.x - bulletvel.x;
            _p2.y = _p.y - bulletvel.y;
            roto = -angle2points(_p, _p2);
            gfxdrawsprite(t[GFX::WEAPONS_ARROW], _p.x, _p.y, 0, 0, roto);
        }
        break;

    case bullet_style_clusternade: {
        _p.x = bulletpos.x;
        _p.y = bulletpos.y - 3;
        gfxdrawsprite(t[GFX::WEAPONS_CLUSTER_GRENADE], _p.x, _p.y, 0, 0,
                      degtorad(timeoutfloat * 5) * iif(bulletvel.x < 0, -1, 1));
    }
    break;

    case bullet_style_cluster: {
        _p.x = bulletpos.x;
        _p.y = bulletpos.y - 2;
        gfxdrawsprite(t[GFX::WEAPONS_CLUSTER], _p.x, _p.y);
    }
    break;

    case bullet_style_law:
        if (timeoutfloat < bullet_timeout - 2)
        {
            _p.x = bulletpos.x + bulletvel.x;
            _p.y = bulletpos.y + bulletvel.y;
            _p2.x = _p.x - bulletvel.x;
            _p2.y = _p.y - bulletvel.y;
            roto = -angle2points(_p, _p2);
            gfxdrawsprite(t[GFX::WEAPONS_MISSILE], _p.x, _p.y, 0, 0, roto);

            if (trails == 1)
            {
                if (timeoutfloat < bullet_timeout - 7)
                {
                    _p.x = bulletpos.x;
                    _p.y = bulletpos.y;
                    _scala.x = fabs((float)(vec2length(bulletvel)) / 3);
                    _scala.y = 1;
                    gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                                  roto, rgba(0xffffff, bulletalpha / 5));
                }
            }
        }
        break;

    case bullet_style_thrownknife: {
        _p.x = bulletpos.x + bulletvel.x;
        _p.y = bulletpos.y + bulletvel.y;
        roto = timeoutfloat / pi;

        if (bulletvel.x >= 0)
            gfxdrawsprite(t[GFX::WEAPONS_KNIFE], _p.x, _p.y, 4, 1, roto);
        else
            gfxdrawsprite(t[GFX::WEAPONS_KNIFE2], _p.x, _p.y, 4, 1, -roto);
    }
    break;

    case bullet_style_m2:
        if (timeoutfloat < m2bullet_timeout - 2)
        {
            _p.x = bulletpos.x + bulletvel.x;
            _p.y = bulletpos.y + bulletvel.y;
            _p2.x = _p.x - bulletvel.x;
            _p2.y = _p.y - bulletvel.y;
            _scala.x = fabs((float)(vec2length(bulletvel)) / bullettrail);
            _scala.y = 1.2;
            roto = -angle2points(_p, _p2);
            gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                          rgba(0xffbf77, bulletalpha * 2));

            if (trails == 1)
            {
                if (timeoutfloat < m2bullet_timeout - 13)
                {
                    _p.x = bulletpos.x;
                    _p.y = bulletpos.y;
                    _scala.x = fabs((float)(vec2length(bulletvel)) / 3);
                    _scala.y = 1;
                    gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                                  roto, rgba(0xffffff, bulletalpha / 5));

                    _scala.x = fabs(vec2length(bulletvel) / (sinusvar + 2.5));
                    _scala.y = sinusvar;
                    gfxdrawsprite(t[GFX::WEAPONS_SMUDGE], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                                  roto, rgba(0xffffff, bulletalpha / 6));
                }
            }
        }
        break;
    }
}
#endif

template <Config::Module M>
void Bullet<M>::kill()
{
#ifdef SERVER
    LogTraceG("TBullet.Kill");
#endif

    active = false;
    if (num > 0)
        bulletparts.active[num] = false;
    setlength(thingcollisions, 0);
}

template <Config::Module M>
tvector2 Bullet<M>::checkmapcollision(float x, float y)
{
    float largestvelocitycomponent;
    std::int32_t j, b, w, k, w2;
    tvector2 pos, perp, step, temp, temp2;
    float d = 0.0;
    std::int32_t detacc;
    std::int32_t kx, ky;
    std::int32_t tempint = 0;
    bool teamcol;

    tvector2 result;

#ifdef SERVER
    LogTraceG("TBullet.CheckMapCollision");
#endif

    result = pos;

    // make step
    largestvelocitycomponent =
        max(fabs(bulletparts.velocity[num].x), fabs(bulletparts.velocity[num].y));

    detacc = trunc(largestvelocitycomponent / 2.5);

    if (detacc == 0)
        detacc = 1;
    vec2scale(step, bulletparts.velocity[num], (float)(1) / detacc);

    // make steps for accurate collision detection
    for (b = 0; b <= detacc - 1; b++)
    {
        pos.x = x + b * step.x;
        pos.y = y + b * step.y;

        // iterate through maps sector polygons
        kx = round((float)(pos.x) / map.sectorsdivision);
        ky = round((float)(pos.y) / map.sectorsdivision);
        if ((kx < -map.sectorsnum) || (kx > map.sectorsnum) || (ky < -map.sectorsnum) ||
            (ky > map.sectorsnum))
        {
            kill();
            return result;
        }

        if (high(map.sectors[kx][ky].polys) > 0)
            for (j = 1; j <= high(map.sectors[kx][ky].polys); j++)
            {
                w = map.sectors[kx][ky].polys[j];
                teamcol = teamcollides(w, sprite[owner].player->team, true);
                if (teamcol)
                    if ((map.polytype[w] != poly_type_only_player) &&
                        (map.polytype[w] != poly_type_doesnt) &&
                        (map.polytype[w] != poly_type_only_flaggers) &&
                        (map.polytype[w] != poly_type_not_flaggers) &&
                        (map.polytype[w] != poly_type_background) &&
                        (map.polytype[w] != poly_type_background_transition))
                        if (map.pointinpolyedges(pos.x, pos.y, w))
                        {
                            switch (style)
                            {
                            case bullet_style_plain:
                            case bullet_style_shotgun:
                            case bullet_style_punch:
                            case bullet_style_knife:
                            case bullet_style_m2: {
                                bulletparts.oldpos[num] = bulletparts.pos[num];
                                bulletparts.pos[num] = vec2subtract(pos, bulletparts.velocity[num]);
                                temp = bulletparts.pos[num];
                                temp2 = bulletparts.velocity[num];

                                perp = vec2subtract(bulletparts.pos[num], hitspot);
                                d = vec2length(perp);
                                // ricochet!
                                if (d > 50.0)
                                {
                                    ricochetcount += 1;
                                    perp = map.closestperpendicular(w, bulletparts.pos[num], d,
                                                                    tempint);
                                    d = vec2length(bulletparts.velocity[num]);
                                    vec2normalize(perp, perp);
                                    vec2scale(perp, perp, -d);

                                    bulletparts.velocity[num].x =
                                        bulletparts.velocity[num].x * ((float)(25) / 35) +
                                        perp.x * ((float)(10) / 35);
                                    bulletparts.velocity[num].y =
                                        bulletparts.velocity[num].y * ((float)(25) / 35) +
                                        perp.y * ((float)(10) / 35);
                                    bulletparts.pos[num] = pos;
                                    hitspot = bulletparts.pos[num];

                                    vec2normalize(perp, bulletparts.velocity[num]);
                                    vec2scale(perp, perp, (float)(d) / 6);
                                    bulletparts.oldpos[num] = bulletparts.pos[num];
                                    pos.x = bulletparts.pos[num].x + perp.x;
                                    pos.y = bulletparts.pos[num].y + perp.y;
                                    kx = round((float)(pos.x) / map.sectorsdivision);
                                    ky = round((float)(pos.y) / map.sectorsdivision);
                                    if ((kx > -map.sectorsnum) && (kx < map.sectorsnum) &&
                                        (ky > -map.sectorsnum) && (ky < map.sectorsnum))
                                    {
                                        for (k = 1; k <= high(map.sectors[kx][ky].polys); k++)
                                        {
                                            w2 = map.sectors[kx][ky].polys[k];
                                            if ((map.polytype[w2] != poly_type_only_player) &&
                                                (map.polytype[w2] != poly_type_doesnt) &&
                                                (map.polytype[w2] != poly_type_only_flaggers) &&
                                                (map.polytype[w2] != poly_type_not_flaggers) &&
                                                (map.polytype[w2] != poly_type_background) &&
                                                (map.polytype[w2] !=
                                                 poly_type_background_transition) and
                                                teamcollides(w2, sprite[owner].player->team, true))
                                                if (map.pointinpolyedges(pos.x, pos.y, w2))
                                                {
                                                    kill();
                                                    break;
                                                }
                                        }
                                    }
                                }
                                else
                                {
                                    kill();
                                }

                                if (active)
                                {
                                    bulletparts.pos[num] = temp;
                                    perp = bulletparts.velocity[num];
                                    bulletparts.velocity[num] = temp2;
                                    hit(hit_type_ricochet);
                                    bulletparts.pos[num] = hitspot;
                                    bulletparts.velocity[num] = perp;
                                }
                                else
                                {
                                    bulletparts.pos[num] = temp;
                                    perp = bulletparts.velocity[num];
                                    bulletparts.velocity[num] = temp2;
                                    hit(hit_type_wall);
                                    bulletparts.pos[num] = hitspot;
                                    bulletparts.velocity[num] = perp;
                                }
                            }
                            break;
                            case bullet_style_arrow: {
                                bulletparts.pos[num] = vec2subtract(pos, bulletparts.velocity[num]);
                                bulletparts.forces[num].y =
                                    bulletparts.forces[num].y - bulletparts.gravity;
                                if (timeout > arrow_resist)
                                    timeout = arrow_resist;
                                if (timeout < 20)
                                    bulletparts.forces[num].y =
                                        bulletparts.forces[num].y + bulletparts.gravity;
                            }
                            break;
                            case bullet_style_fragnade:
                            case bullet_style_flame: {
#ifndef SERVER
                                // bounce sound
                                if (style == bullet_style_fragnade)
                                    if (vec2length(bulletparts.velocity[num]) > 1.5)
                                        playsound(sfx_grenade_bounce, spriteparts.pos[num]);
#endif

                                perp =
                                    map.closestperpendicular(w, bulletparts.pos[num], d, tempint);

                                vec2normalize(perp, perp);
                                vec2scale(perp, perp, d);

                                bulletparts.pos[num] = pos;
                                bulletparts.velocity[num] =
                                    vec2subtract(bulletparts.velocity[num], perp);

                                vec2scale(bulletparts.velocity[num], bulletparts.velocity[num],
                                          grenade_surfacecoef);

                                if (style == bullet_style_flame)
                                    if (timeout > 16)
                                        timeout = 16;
                            }
                            break;
                            case bullet_style_m79:
                            case bullet_style_flamearrow:
                            case bullet_style_law: {
                                bulletparts.oldpos[num] = bulletparts.pos[num];
                                bulletparts.pos[num] = vec2subtract(pos, bulletparts.velocity[num]);
                                temp = bulletparts.pos[num];
                                temp2 = bulletparts.velocity[num];

                                perp = vec2subtract(bulletparts.pos[num], hitspot);
                                d = vec2length(perp);
                                // ricochet!
                                if (d > 50.0)
                                {
                                    ricochetcount += 1;
                                    perp = map.closestperpendicular(w, bulletparts.pos[num], d,
                                                                    tempint);
                                    d = vec2length(bulletparts.velocity[num]);
                                    vec2normalize(perp, perp);
                                    vec2scale(perp, perp, -d);

                                    bulletparts.velocity[num].x =
                                        bulletparts.velocity[num].x * ((float)(25) / 35) +
                                        perp.x * ((float)(10) / 35);
                                    bulletparts.velocity[num].y =
                                        bulletparts.velocity[num].y * ((float)(25) / 35) +
                                        perp.y * ((float)(10) / 35);
                                    bulletparts.pos[num] = pos;
                                    hitspot = bulletparts.pos[num];

                                    vec2normalize(perp, bulletparts.velocity[num]);
                                    vec2scale(perp, perp, (float)(d) / 6);

                                    pos.x = bulletparts.pos[num].x + perp.x;
                                    pos.y = bulletparts.pos[num].y + perp.y;
                                    kx = round((float)(pos.x) / map.sectorsdivision);
                                    ky = round((float)(pos.y) / map.sectorsdivision);
                                    if ((kx > -map.sectorsnum) && (kx < map.sectorsnum) &&
                                        (ky > -map.sectorsnum) && (ky < map.sectorsnum))
                                    {
                                        for (k = 1; k <= high(map.sectors[kx][ky].polys); k++)
                                        {
                                            w2 = map.sectors[kx][ky].polys[k];
                                            if ((map.polytype[w2] != poly_type_only_player) &&
                                                (map.polytype[w2] != poly_type_doesnt) &&
                                                (map.polytype[w2] != poly_type_only_flaggers) &&
                                                (map.polytype[w2] != poly_type_not_flaggers) &&
                                                (map.polytype[w2] != poly_type_background) &&
                                                (map.polytype[w2] !=
                                                 poly_type_background_transition) and
                                                teamcollides(w2, sprite[owner].player->team, true))
                                                if (map.pointinpolyedges(pos.x, pos.y, w2))
                                                {
                                                    kill();
                                                    break;
                                                }
                                        }
                                    }
                                }
                                else
                                {
                                    kill();
                                }

                                if (active)
                                {
                                    bulletparts.pos[num] = temp;
                                    perp = bulletparts.velocity[num];
                                    bulletparts.velocity[num] = temp2;
                                    hit(hit_type_ricochet);
                                    bulletparts.pos[num] = hitspot;
                                    bulletparts.velocity[num] = perp;
                                }
                                else
                                {
                                    bulletparts.pos[num] = temp;
                                    perp = bulletparts.velocity[num];
                                    bulletparts.velocity[num] = temp2;
                                    hit(hit_type_explode);
                                    bulletparts.pos[num] = hitspot;
                                    bulletparts.velocity[num] = perp;
                                }
                            }
                            break;
                            case bullet_style_clusternade: {
                                hit(hit_type_clusternade);
                                kill();
                            }
                            break;
                            case bullet_style_cluster: {
                                hit(hit_type_cluster);
                                kill();
                            }
                            break;
                            case bullet_style_thrownknife: {
                                bulletparts.pos[num] = vec2subtract(pos, bulletparts.velocity[num]);

                                // create knife thing
#ifdef SERVER
                                creatething(bulletparts.pos[num], owner, object_combat_knife, 255);
#endif

                                hit(hit_type_wall);
                                kill();
                            }
                            break;
                            } // case

                            result = pos;
                            return result;
                        } // PointinPolyEdges
            }             // for j
    }                     // for b
    return result;
}

template <Config::Module M>
tvector2 Bullet<M>::checkspritecollision(float lasthitdist)
{
    const std::array<std::int32_t, 7> bodypartspriority = {{12, 11, 10, 6, 5, 4, 3}};
    tspriteindexes spritesbydistance;
    std::int32_t spritecount, spritecounter;
    std::int32_t j;
    tvector2 pos, norm, colpos, a, col;
    tvector2 bulletvelocity;
    particlesystem *candidateskeleton;
    std::int32_t bodypartpriorityindex;
    std::int32_t bodypartid;
    tvector2 bodypartoffset;
    tvector2 buttstockpositionoffset, startpoint, endpoint;
    std::uint8_t where;
    std::uint8_t weaponindex;
    float dist, mindist;
    std::int32_t r;
    tvector2 bulletpush;
    std::int32_t pushtick;
    float hitboxmodifier;
    float speed;
    bool wasdead;
    std::uint8_t nocollision;
#ifndef SERVER
    std::uint8_t clothesshreadstyle;
    std::int32_t i;
    std::int32_t srv = 0;
#endif

    tvector2 result;
#ifdef SERVER
    LogTraceG("TBullet.CheckSpriteCollision {} {} {} {} {}", style, owner, ownerweapon, timeout,
              tracking);
#endif

    if ((style == bullet_style_arrow) && (timeout <= arrow_resist))
        return result;

    pos.x = 0;
    result = pos;

    bulletvelocity = bulletparts.velocity[num];

#ifdef SERVER
    LogTraceG("TBullet.CheckSpriteCollision 2");
#endif
    // Iterate through sprites
    if (style != bullet_style_clusternade)
    {
        spritecount = filterspritesbydistance(spritesbydistance);
        spritecounter = 0;

        while (spritecounter < spritecount)
        {
            spritecounter = spritecounter + 1;
            j = spritesbydistance[spritecounter];

            col = getspritecollisionpoint(j);

            where = 0;
            if (style != bullet_style_fragnade)
                r = part_radius;
            else
                r = part_radius + 1;

#ifdef SERVER
            LogTraceG("TBullet.CheckSpriteCollision 3 -- {}", j);
#endif

            candidateskeleton = &sprite[j].skeleton;

            // Pre-calculate some points if it's a melee weapon
            if ((style == bullet_style_punch) || (style == bullet_style_knife))
            {
                pos = bulletparts.pos[num];

                buttstockpositionoffset = sprite[owner].gethandsaimdirection();
                vec2scale(buttstockpositionoffset, buttstockpositionoffset, 4);

                startpoint = vec2add(sprite[owner].skeleton.pos[15], buttstockpositionoffset);
                endpoint = vec2add(pos, bulletvelocity);
            }
            else
            {
                startpoint = bulletparts.pos[num];
                endpoint = vec2add(startpoint, bulletvelocity);
            }

            // Check for collision with the body parts
            mindist = std::numeric_limits<float>::max();

            for (bodypartpriorityindex = 0; bodypartpriorityindex <= high(bodypartspriority);
                 bodypartpriorityindex++)
            {
                bodypartid = bodypartspriority[bodypartpriorityindex];

                bodypartoffset =
                    vec2subtract(candidateskeleton->pos[bodypartid], spriteparts.pos[j]);
                colpos = vec2add(col, bodypartoffset);

                // FIXME(skoskav): Offset player sprite 2px to the left because sprites are glitchy
                // like that (don't need to do it for melee weapons though because their offsets
                // cancel each other out)
                if ((style != bullet_style_punch) && (style != bullet_style_knife))
                    colpos.x = colpos.x - 2;

                if (linecirclecollision(startpoint, endpoint, colpos, r, pos))
                {
                    dist = sqrdist(startpoint, pos);

                    if (dist < mindist)
                    {
                        where = bodypartid;
                        mindist = dist;
                    }
                }
            }

#ifdef SERVER
            LogTraceG("TBullet.CheckSpriteCollision 4 ");
#endif

            if (((style != bullet_style_punch) && (style != bullet_style_knife)) || (j != owner))
            {
                if (where > 0)
                {
                    // order collision
                    if (lasthitdist > -1)
                    {
                        a = vec2subtract(pos, bulletparts.oldpos[num]);
                        dist = vec2length(a);

                        if (dist > lasthitdist)
                        {
                            break;
                        }
                    }

                    sprite[j].brain.pissedoff = owner;

                    norm = vec2subtract(pos, sprite[j].skeleton.pos[where]);
                    vec2scale(norm, norm, 1.3);
                    norm.y = -norm.y;

#ifdef SERVER
                    LogTraceG("TBullet.CheckSpriteCollision 5 {}", where);
#endif

                    result = pos;

                    nocollision = guns[weaponnumtoindex(ownerweapon)].nocollision;

                    if (((nocollision & weapon_nocollision_enemy) != 0) and
                        (sprite[j].isnotinsameteam(sprite[owner])))
                        continue;

                    if (((nocollision & weapon_nocollision_team) != 0) and
                        (sprite[j].isinsameteam(sprite[owner])) && (j != owner))
                        continue;

                    if (((nocollision & weapon_nocollision_self) != 0) && (owner == j))
                        continue;

                    if (sprite[j].ceasefirecounter < 0)
                    {
                        weaponindex = getweaponindex();

                        // Collision respond
                        if (!sprite[j].deadmeat)
                        {
                            if ((style != bullet_style_fragnade) && (style != bullet_style_flame) &&
                                (style != bullet_style_arrow))
                            {
                                vec2scale(bulletpush, bulletvelocity, guns[weaponindex].push);
                                pushtick = sprite[j].player->pingticks / 2 + ownerpingtick + 1;
                                if (pushtick > max_pushtick)
                                    pushtick = max_pushtick;
                                sprite[j].nextpush[pushtick] =
                                    vec2add(sprite[j].nextpush[pushtick], bulletpush);
                            }
                        }

                        switch (style)
                        {
                        case bullet_style_plain:
                        case bullet_style_shotgun:
                        case bullet_style_punch:
                        case bullet_style_knife:
                        case bullet_style_m2: {
                            bulletparts.pos[num] = pos;

#ifndef SERVER
                            // Blood spark
                            if ((CVar::sv_friendlyfire) or sprite[owner].issolo() or
                                sprite[owner].isnotinsameteam(sprite[j]) or (j == owner))
                            {
                                hit(hit_type_blood);
                            }

                            // Shake screen
                            if ((owner == mysprite) && (ownerweapon == guns[chainsaw].num))
                            {
                                camerax = camerax - 3 + Random(7);
                                cameray = cameray - 3 + Random(7);
                            }

                            // Puff
                            vec2normalize(a, bulletvelocity);
                            vec2scale(a, a, 3);
                            a = vec2add(bulletparts.pos[num], a);
                            if (CVar::r_maxsparks > (max_sparks - 10))
                                createspark(a, a, 50, j, 31);

                            // Shread clothes
                            if (CVar::r_maxsparks > (max_sparks - 10))
                            {
                                if (where <= 4)
                                    clothesshreadstyle = 49; // Piece of clothing with pants
                                else if (where <= 11)
                                    clothesshreadstyle = 48; // Piece of clothing
                                else
                                    clothesshreadstyle = 0;

                                if (clothesshreadstyle > 0)
                                {
                                    for (i = 1; i <= 2; i++)
                                    {
                                        if (Random(8) == 0)
                                        {
                                            a.x = sin(Random(100));
                                            a.y = cos(Random(100));
                                            createspark(pos, a, clothesshreadstyle, j, 120);
                                        }
                                    }
                                }
                            }

                            // play hit sound
                            if (sprite[j].vest < 1)
                            {
                                if (!sprite[j].deadmeat)
                                    playsound(sfx_hit_arg + Random(3), bulletparts.pos[num]);
                                else
                                    playsound(sfx_dead_hit, bulletparts.pos[num]);
                            }
                            else
                                playsound(sfx_vesthit, bulletparts.pos[num]);
#endif

                            // Head, torso or leg hitbox modifier
                            if (where <= 4)
                                hitboxmodifier = guns[weaponnumtoindex(ownerweapon)].modifierlegs;
                            else if (where <= 11)
                                hitboxmodifier = guns[weaponnumtoindex(ownerweapon)].modifierchest;
                            else
                                hitboxmodifier = guns[weaponnumtoindex(ownerweapon)].modifierhead;

                            speed = vec2length(bulletvelocity);

                            wasdead = sprite[j].deadmeat;

                            sprite[j].healthhit(
#ifndef SERVER
                                srv *
#endif
                                    speed * hitmultiply * hitboxmodifier,
                                owner, where, num, norm);

#ifndef SERVER
                            // hit Spray
                            if (canhitspray(j, owner))
                                hitspray();
#endif

#ifdef SERVER
                            LogTraceG("TBullet.CheckSpriteCollision a ");
#endif

                            // drop weapon when punched
                            if (style == bullet_style_punch)
                                if (sprite[j].issolo() or
                                    (sprite[j].isnotsolo() and
                                     sprite[j].isnotinsameteam(sprite[owner])))
                                    if ((sprite[j].weapon.num != guns[bow].num) &&
                                        (sprite[j].weapon.num != guns[bow2].num))
                                        sprite[j].bodyapplyanimation(throwweapon, 11);

                            hitbody = j;

                            // Pierce check and break to next sprite
                            if (wasdead)
                            {
                                vec2scale(bulletparts.velocity[num], bulletvelocity, 0.9);
                                bulletvelocity = bulletparts.velocity[num];
                                hit(hit_type_bodyhit);
                                continue;
                            }

                            if (sprite[j].deadmeat or (speed > 23))
                            {
                                vec2scale(bulletparts.velocity[num], bulletvelocity, 0.75);
                                bulletvelocity = bulletparts.velocity[num];
                                hit(hit_type_bodyhit);
                                continue;
                            }

                            if ((speed > 5) && ((float)(speed) / guns[weaponindex].speed >= 0.9))
                            {
                                vec2scale(bulletparts.velocity[num], bulletvelocity, 0.66);
                                bulletvelocity = bulletparts.velocity[num];
                                hit(hit_type_bodyhit);
                                continue;
                            }

                            // Destroy bullet
                            kill();
                        }
                        break;

                        case bullet_style_fragnade: {
                            if (!sprite[j].deadmeat)
                            {
                                hit(hit_type_fragnade, j, where);
                                kill();
                            }
                        }
                        break;

                        case bullet_style_arrow:
                            if (timeout > arrow_resist)
                            {
                                bulletparts.pos[num] = vec2subtract(pos, bulletparts.velocity[num]);
                                bulletparts.forces[num].y =
                                    bulletparts.forces[num].y - bulletparts.gravity;
                                if (((CVar::sv_friendlyfire)&sprite[owner].isnotsolo() and
                                     sprite[owner].isinsameteam(sprite[j])
#ifndef SERVER
                                     && (num != mysprite)
#endif
                                         ) ||
                                    (sprite[j].bonusstyle == bonus_flamegod))
                                    ;
                                else
                                    hit(hit_type_blood);
#ifndef SERVER
                                // play hit sound
                                if (sprite[j].vest < 1)
                                {
                                    if (!sprite[j].deadmeat)
                                        playsound(sfx_hit_arg + Random(3), bulletparts.pos[num]);
                                    else
                                        playsound(sfx_dead_hit, bulletparts.pos[num]);
                                }
                                else
                                    playsound(sfx_vesthit, bulletparts.pos[num]);
#endif

#ifdef SERVER
                                LogTraceG("TBullet.CheckSpriteCollision b ");
#endif

                                // Head, torso or leg hitbox modifier
                                if (where <= 4)
                                    hitboxmodifier =
                                        guns[weaponnumtoindex(ownerweapon)].modifierlegs;
                                else if (where <= 11)
                                    hitboxmodifier =
                                        guns[weaponnumtoindex(ownerweapon)].modifierchest;
                                else
                                    hitboxmodifier =
                                        guns[weaponnumtoindex(ownerweapon)].modifierhead;

                                speed = vec2length(bulletparts.velocity[num]);

                                sprite[j].healthhit(
#ifndef SERVER
                                    srv *
#endif
                                        speed * hitmultiply * hitboxmodifier,
                                    owner, where, num, norm);

                                if (!sprite[j].deadmeat)
                                    sprite[j].skeleton.pos[where] = a;

                                kill();
                            }
                            break;

                        case bullet_style_m79:
                        case bullet_style_flamearrow:
                        case bullet_style_law:
                            if (!sprite[j].deadmeat)
                            {
                                hit(hit_type_explode, j, where);
                                bulletparts.pos[num] = pos;
                                kill();

                                sprite[j].healthhit(
#ifndef SERVER
                                    srv *
#endif
                                        vec2length(bulletparts.velocity[num]) * hitmultiply,
                                    owner, where, num, norm);

#ifdef SERVER
                                LogTraceG("TBullet.CheckSpriteCollision c ");
#endif

                                if (!sprite[j].deadmeat)
                                    sprite[j].skeleton.pos[where] = a;
                            }
                            break;

                        case bullet_style_flame:
                            if (owner != j)
                            {
                                bulletparts.pos[num] = sprite[j].skeleton.pos[where];
                                if (!sprite[j].deadmeat)
                                    bulletparts.velocity[num] = spriteparts.velocity[j];
                                else
                                {
                                    bulletparts.velocity[num].x = 0;
                                    bulletparts.velocity[num].y = 0;
                                }

#ifdef SERVER
                                if ((timeout < 3) && (ricochetcount < 2))
#else
                                if ((timeout < 2) && (ricochetcount < 1))
#endif
                                {
                                    if (hitmultiply >= (float)(guns[flamer].hitmultiply) / 3)
                                    {
                                        timeout = flamer_timeout - 1;
                                        ricochetcount += 1;
                                        a.x = -spriteparts.velocity[j].x;
                                        a.y = -spriteparts.velocity[j].y;
                                        createbullet(
                                            sprite[j].skeleton.pos[where], a, guns[flamer].num,
                                            owner, 255, (float)(2 * hitmultiply) / 3,
                                            //                        False, {$IFDEF
                                            //                        SERVER}False{$ELSE}True{$ENDIF});
                                            false, true);
                                    }

                                    if (sprite[j].health > -1)
                                        sprite[j].healthhit(
#ifndef SERVER
                                            srv *
#endif
                                                hitmultiply,
                                            owner, where, num, norm);
                                }
                            }
                            break;

                        case bullet_style_cluster: {
#ifdef SERVER
                            LogTraceG("TBullet.CheckSpriteCollision f ");
#endif

                            hit(hit_type_cluster, j, where);

                            if (!sprite[j].deadmeat)
                                sprite[j].skeleton.pos[where] = a;

                            kill();
                        }
                        break;

                        case bullet_style_thrownknife: {
#ifndef SERVER
                            // Blood spark
                            if ((CVar::sv_friendlyfire) or sprite[owner].issolo() or
                                sprite[owner].isnotinsameteam(sprite[j]) or (j == owner))
                            {
                                hit(hit_type_blood);
                            }

                            // Puff
                            vec2normalize(a, bulletparts.velocity[num]);
                            vec2scale(a, a, 3);
                            a = vec2add(bulletparts.pos[num], a);
                            if (CVar::r_maxsparks > (max_sparks - 10))
                                createspark(a, a, 50, j, 31);

                            // Shread clothes
                            if (CVar::r_maxsparks > (max_sparks - 10))
                            {
                                if (where <= 4)
                                    clothesshreadstyle = 49; // Piece of clothing with pants
                                else if (where <= 11)
                                    clothesshreadstyle = 48; // Piece of clothing
                                else
                                    clothesshreadstyle = 0;

                                if (clothesshreadstyle > 0)
                                {
                                    for (i = 1; i <= 2; i++)
                                    {
                                        if (Random(8) == 0)
                                        {
                                            a.x = sin(Random(100));
                                            a.y = cos(Random(100));
                                            createspark(pos, a, clothesshreadstyle, j, 120);
                                        }
                                    }
                                }
                            }

                            // play hit sound
                            if (std::find(spritecollisions.begin(), spritecollisions.end(),
                                          sprite[j].num) != spritecollisions.end())
                            {
                                NotImplemented(NITag::OTHER, "Check spritecollisions");
#if 0
                                include(spritecollisions, sprite[j].num);
#endif

                                if (sprite[j].vest < 1)
                                {
                                    if (!sprite[j].deadmeat)
                                        playsound(sfx_hit_arg + Random(3), bulletparts.pos[num]);
                                    else
                                        playsound(sfx_dead_hit, bulletparts.pos[num]);
                                }
                                else
                                    playsound(sfx_vesthit, bulletparts.pos[num]);
                            }
#endif

#ifdef SERVER
                            LogTraceG("TBullet.CheckSpriteCollision g ");
#endif

                            wasdead = sprite[j].deadmeat;

                            sprite[j].healthhit(
#ifndef SERVER
                                srv *
#endif
                                    vec2length(bulletparts.velocity[num]) * hitmultiply * 0.01,
                                owner, where, num, norm);

                            if (!sprite[j].deadmeat)
                                sprite[j].skeleton.pos[where] = a;

                            if ((!wasdead) or (CVar::sv_realisticmode))
                            {
#ifdef SERVER
                                // create knife thing
                                creatething(bulletparts.pos[num], owner, object_combat_knife, 255);
#endif
                                kill();
                            }
                        }
                        break;
                        } // case

                        // Bullet is destroyed, so exit
                        return result;
                    }
                }
            }

#ifdef SERVER
            LogTraceG("TBullet.CheckSpriteCollision 7 ");
#endif
        } // while SpriteCounter < SpriteCount
    }
    return result;
}

template <Config::Module M>
tvector2 Bullet<M>::checkthingcollision(float lasthitdist)
{
    std::int32_t i, j;
    tvector2 startpoint, endpoint, pos, colpos, a;
    std::uint8_t where;
    float dist;
    tvector2 thingvel;
    tvector2 veldiff;
    tvector2 bulletpush;
    std::uint8_t newindex;
    bool skipcollision;

    tvector2 result;
#ifdef SERVER
    LogTraceG("TBullet.CheckThingCollision");
#endif

    pos.x = 0;
    result = pos;

    // iterate through Things
    if (style != bullet_style_fragnade)
        for (j = 1; j <= max_things; j++)
            if (thing[j].active && (timeout < bullet_timeout - 1) && thing[j].collidewithbullets and
                (owner != thing[j].holdingsprite) &&
                (((CVar::sv_gamemode == gamestyle_inf) && (thing[j].style == object_bravo_flag)) ||
                 (CVar::sv_gamemode != gamestyle_inf)) &&
                (thing[j].style != object_stationary_gun))
            {
                startpoint = bulletparts.pos[num];
                endpoint = vec2add(startpoint, bulletparts.velocity[num]);

                where = 0;

                for (i = 1; i <= 2; i++)
                {
                    colpos = thing[j].skeleton.pos[i];

                    if (linecirclecollision(startpoint, endpoint, colpos, flag_part_radius, pos))
                    {
                        where = i;
                        break;
                    }
                }

                if ((where == 1) || (where == 2))
                {
                    // order collision
                    if (lasthitdist > -1)
                    {
                        a = vec2subtract(pos, bulletparts.oldpos[num]);
                        dist = vec2length(a);
                        if (dist > lasthitdist)
                            break;
                    }

                    // Thing push cooldown from this bullet
                    skipcollision = false;

                    for (i = 0; i <= high(thingcollisions); i++)
                    {
                        if (thingcollisions[i].thingnum == thing[j].num)
                        {
                            if (maintickcounter < thingcollisions[i].cooldownend)
                            {
                                skipcollision = true;
                                break;
                            }
                        }
                    }

                    if (skipcollision)
                        break;

                    // TODO: has high performance impact? -> do alloc like std vector
                    newindex = length(thingcollisions);
                    setlength(thingcollisions, newindex + 1);
                    thingcollisions[newindex] =
                        thingcollision(thing[j].num, maintickcounter + thing_collision_cooldown);

                    // collision respond
                    thingvel =
                        vec2subtract(thing[j].skeleton.pos[where], thing[j].skeleton.oldpos[where]);
                    veldiff = vec2subtract(bulletparts.velocity[num], thingvel);

                    vec2scale(bulletpush, veldiff,
                              guns[getweaponindex()].push * thing_push_multiplier);

                    thing[j].skeleton.pos[where] =
                        vec2add(thing[j].skeleton.pos[where], bulletpush);

                    result = pos;
                    thing[j].statictype = false;

                    switch (style)
                    {
                    case bullet_style_plain:
                    case bullet_style_fragnade:
                    case bullet_style_shotgun: {
                        hit(hit_type_thing);
                    }
                    break;
                    }

                    break;
                }
            } // for j
    return result;
}

template <Config::Module M>
tvector2 Bullet<M>::checkcollidercollision(float lasthitdist)
{
    std::int32_t j;
#ifndef SERVER
    std::int32_t i;
#endif
    tvector2 startpoint, endpoint, pos, colpos, a;
    float dist;

#ifdef SERVER
    LogTraceG("TBullet.CheckColliderCollision");
#endif

    tvector2 result;

    pos.x = 0;
    result = pos;

    // iterate through colliders
    for (j = 1; j <= 128; j++)
        if (map.collider[j].active)
        {
            startpoint = bulletparts.pos[num];
            endpoint = vec2add(startpoint, bulletparts.velocity[num]);
            // make steps for accurate collision detection

            colpos.x = map.collider[j].x;
            colpos.y = map.collider[j].y;

            if (linecirclecollision(startpoint, endpoint, colpos, map.collider[j].radius / 1.7,
                                    pos))
            {
                // order collision
                if (lasthitdist > -1)
                {
                    a = vec2subtract(pos, bulletparts.oldpos[num]);
                    dist = vec2length(a);
                    if (dist > lasthitdist)
                    {
                        break;
                    }
                }

                switch (style)
                {
                case bullet_style_plain:
                case bullet_style_shotgun:
                case bullet_style_punch:
                case bullet_style_knife:
                case bullet_style_thrownknife:
                case bullet_style_m2: {
                    bulletparts.pos[num] = vec2subtract(pos, bulletparts.velocity[num]);

#ifndef SERVER
                    // dirt
                    if (CVar::r_maxsparks > (max_sparks - 10))
                        for (i = 1; i <= 2; i++)
                            if (Random(4) == 0)
                            {
                                a.x = sin(Random(100));
                                a.y = cos(Random(100));
                                createspark(pos, a, 44 + Random(4), j, 120);
                            }
#endif

                            // create knife thing
#ifdef SERVER
                    if (style == bullet_style_thrownknife)
                        creatething(bulletparts.pos[num], owner, object_combat_knife, 255);
#endif

#ifndef SERVER
                    playsound(sfx_colliderhit, bulletparts.pos[num]);
#endif

                    hit(hit_type_wall);
                    kill();
                }
                break;
                case bullet_style_fragnade: // frag grenade exploded
                    if (timeout < grenade_timeout - 2)
                    {
                        hit(hit_type_fragnade);
                        kill();
                    }
                    break;
                case bullet_style_flame: {
                    kill();
                }
                break;
                case bullet_style_arrow:
                    if (timeout > arrow_resist)
                    {
                        bulletparts.forces[num].y = bulletparts.forces[num].y - bulletparts.gravity;
                        hit(hit_type_wall);

                        kill();
                    }
                    break;
                case bullet_style_m79:
                case bullet_style_flamearrow:
                case bullet_style_law: {
                    hit(hit_type_explode); // plays m79 explosion sound
                    kill();
                }
                break;
                case bullet_style_clusternade: {
                    hit(hit_type_clusternade);
                    kill();
                }
                break;
                case bullet_style_cluster: {
                    hit(hit_type_cluster);
                    kill();
                }
                break;
                } // case

                result = pos;
                return result;
            }
        } // for j
    return result;
}

template <Config::Module M>
void Bullet<M>::hit(std::int32_t t, std::int32_t spritehit, std::int32_t where)
{
    tvector2 a, b;
    std::int32_t i;

#ifdef SERVER
    LogTraceG("TBullet.Hit");
#endif

    switch (t)
    {
    case hit_type_wall: // wall hit
    {
#ifndef SERVER
        b = bulletparts.velocity[num];
        a = vec2add(bulletparts.pos[num], bulletparts.velocity[num]);
        vec2scale(b, b, -0.06);
        b.y = b.y - 1.0;

        b.x = b.x * (0.6 + (float)(Random(8)) / 10);
        b.y = b.y * (0.8 + (float)(Random(4)) / 10);
        createspark(a, b, 3, owner, 60);

        b.x = b.x * (0.8 + (float)(Random(4)) / 10);
        b.y = b.y * (0.6 + (float)(Random(8)) / 10);
        createspark(a, b, 3, owner, 65);

        vec2scale(b, b, 0.4 + (float)(Random(4)) / 10);
        createspark(a, b, 1, owner, 60);

        b.x = b.x * (0.5 + (float)(Random(4)) / 10);
        b.y = b.y * (0.7 + (float)(Random(8)) / 10);
        createspark(a, b, 3, owner, 50);

        b.x = 0;
        b.y = 0;
        if (CVar::r_maxsparks > (max_sparks - 5))
            createspark(a, b, 56, owner, 22);

        if (timeout < bullet_timeout - 5)
        {
            playsound(sfx_ric + Random(4), bulletparts.pos[num]);
        }
#endif
    }
    break;

    case hit_type_blood: // body hit -blood
    {
#ifndef SERVER
        b = bulletparts.velocity[num];
        a = bulletparts.pos[num];
        vec2scale(b, b, 0.025);
        // Vec3Normalize(b, b);
        b.x = b.x * 1.2;
        b.y = b.y * 0.85;
        createspark(a, b, 4, owner, 70);

        b.x = b.x * 0.745;
        b.y = b.y * 1.1;
        createspark(a, b, 4, owner, 75);
        b.x = b.x * 0.9;
        b.y = b.y * 0.85;
        if (Random(2) == 0)
            createspark(a, b, 4, owner, 75);

        b.x = b.x * 1.2;
        b.y = b.y * 0.85;
        createspark(a, b, 5, owner, 80);

        b.x = b.x * 1;
        b.y = b.y * 1;
        createspark(a, b, 5, owner, 85);

        b.x = b.x * 0.5;
        b.y = b.y * 1.05;
        if (Random(2) == 0)
            createspark(a, b, 5, owner, 75);

        for (i = 1; i <= 7; i++)
            if (Random(6) == 0)
            {
                b.x = sin(Random(100)) * 1.6;
                b.y = cos(Random(100)) * 1.6;
                createspark(a, b, 4, owner, 55);
            }
#endif
    }
    break;

    case hit_type_explode: // 40mm grenade explode
    {
#ifndef SERVER
        a = vector2(0.0, 0.0);

        if (CVar::r_maxsparks > (max_sparks - 10))
            createspark(bulletparts.pos[num], a, 60, owner, 255);

        if (CVar::r_maxsparks > (max_sparks - 10))
            createspark(bulletparts.pos[num], a, 54, owner, smoke_anims * 4 + 10);

        createspark(bulletparts.pos[num], a, 12, owner, explosion_anims * 3);
        playsound(sfx_m79_explosion, bulletparts.pos[num]);
#endif

        explosionhit(hit_type_explode, spritehit, where);
    }
    break;

    case hit_type_fragnade: {
#ifndef SERVER
        a = vector2(0.0, 0.0);

        if (CVar::r_maxsparks > (max_sparks - 10))
            createspark(bulletparts.pos[num], a, 60, owner, 190);

        if (CVar::r_maxsparks > (max_sparks - 10))
            createspark(bulletparts.pos[num], a, 54, owner, smoke_anims * 4 + 10);

        createspark(bulletparts.pos[num], a, 17, owner, explosion_anims * 3);
        playsound(sfx_grenade_explosion, bulletparts.pos[num]);
#endif

        explosionhit(hit_type_fragnade, spritehit, where);
    }
    break;

    case hit_type_thing: {
#ifndef SERVER
        b = bulletparts.velocity[num];
        a = vec2add(bulletparts.pos[num], bulletparts.velocity[num]);
        vec2scale(b, b, -0.02);
        vec2scale(b, b, 0.4 + (float)(Random(4)) / 10);
        createspark(a, b, 1, owner, 70);

        playsound(sfx_bodyfall, bulletparts.pos[num]);
#endif
    }
    break;

    case hit_type_clusternade: {
#ifndef SERVER
        b.x = 0;
        b.y = 0;
        createspark(bulletparts.pos[num], b, 29, owner, 55);

        playsound(sfx_clustergrenade, bulletparts.pos[num]);
#endif

        a = vec2subtract(bulletparts.pos[num], bulletparts.velocity[num]);

        for (i = 1; i <= 5; i++)
        {
            b = bulletparts.velocity[num];
            vec2scale(b, b, -0.75);
            b.x = -b.x - 2.5 + ((float)(Random(50)) / 10.0f);
            b.y = b.y - 2.5 + ((float)(Random(25)) / 10.0f);
            createbullet(a, b, guns[cluster].num, owner, 255,
                         (float)(guns[fraggrenade].hitmultiply) / 2.0f, true, false);
        }
    }
    break;

    case hit_type_cluster: {
#ifndef SERVER
        a = vector2(0.0, 0.0);
        createspark(bulletparts.pos[num], a, 28, owner, explosion_anims * 3);
        playsound(sfx_cluster_explosion, bulletparts.pos[num]);
#endif

        explosionhit(hit_type_cluster, spritehit, where);
    }
    break;

    case hit_type_flak: {
#ifndef SERVER
        a = vector2(0.0, 0.0);
        createspark(bulletparts.pos[num], a, 29, owner, 55);
        playsound(sfx_m2explode, bulletparts.pos[num]);
#endif

        explosionhit(hit_type_flak, spritehit, where);
    }
    break;

#ifndef SERVER
    case hit_type_bodyhit: {
        if (CVar::r_maxsparks < (max_sparks - 10))
            return;

        b = bulletparts.velocity[num];
        a = bulletparts.pos[num];
        vec2scale(b, b, 0.075);

        b.x = b.x * 1.2;
        b.y = b.y * 0.85;
        createspark(a, b, 4, owner, 60);
        b.x = b.x * 0.745;
        b.y = b.y * 1.1;
        createspark(a, b, 4, owner, 65);
        b.x = b.x * 1.5;
        b.y = b.y * 0.4;
        createspark(a, b, 5, owner, 70);
        b.x = b.x * 1;
        b.y = b.y * 1;
        createspark(a, b, 5, owner, 75);
        b.x = b.x * 0.4;
        b.y = b.y * 1.15;

        for (i = 1; i <= 4; i++)
            if (Random(6) == 0)
            {
                b.x = sin(Random(100)) * 1.2;
                b.y = cos(Random(100)) * 1.2;
                createspark(a, b, 4, owner, 50);
            }
    }
    break;

    case hit_type_ricochet: // ricochet hit
    {
        a = vec2add(bulletparts.pos[num], bulletparts.velocity[num]);

        b.x = (-2.0 + (float)(Random(40)) / 10.0);
        b.y = (-2.0 + (float)(Random(40)) / 10.0);
        createspark(a, b, 26, owner, 35);
        b.x = (-2.0 + (float)(Random(40)) / 10.0);
        b.y = (-2.0 + (float)(Random(40)) / 10.0);
        createspark(a, b, 26, owner, 35);
        b.x = (-3.0 + (float)(Random(60)) / 10.0);
        b.y = (-3.0 + (float)(Random(60)) / 10.0);
        createspark(a, b, 26, owner, 35);
        b.x = (-3.0 + (float)(Random(60)) / 10.0);
        b.y = (-3.0 + (float)(Random(60)) / 10.0);
        createspark(a, b, 26, owner, 35);
        b.x = (-3.0 + (float)(Random(60)) / 10.0);
        b.y = (-3.0 + (float)(Random(60)) / 10.0);
        createspark(a, b, 26, owner, 35);
        b.x = (-3.0 + (float)(Random(60)) / 10.0);
        b.y = (-3.0 + (float)(Random(60)) / 10.0);
        createspark(a, b, 27, owner, 35);

        playsound(sfx_ric5 + Random(3), bulletparts.pos[num]);
    }
    break;
#endif
    } // case
}

template <Config::Module M>
void Bullet<M>::explosionhit(std::int32_t typ, std::int32_t spritehit, std::int32_t where)
{
    const float after_explosion_radius2 = after_explosion_radius * after_explosion_radius;
    const std::array<std::int32_t, 7> bodyparts = {{12, 11, 10, 6, 5, 4, 3}};
#ifndef SERVER
    const std::int32_t srv = 0;
#endif
    tvector2 a, col;
    std::int32_t i, j, w, igun, pushtick;
    bool parthit;
    float s, explosionradius, explosionradius2, hitboxmodifier;
    std::uint8_t nocollision;
    float s2 = 0.0;
#ifndef SERVER
    tvector2 b;
    std::int32_t n, rnd;
#endif

    switch (typ)
    {
    case hit_type_fragnade: {
        igun = fraggrenade;
        explosionradius = fraggrenade_explosion_radius;
    }
    break;
    case hit_type_explode: {
        igun = m79;
        explosionradius = m79grenade_explosion_radius;
    }
    break;
    case hit_type_cluster:
    case hit_type_flak: {
        igun = fraggrenade;
        explosionradius = clustergrenade_explosion_radius;
    }
    break;
    default:
        return;
    }

    explosionradius2 = explosionradius * explosionradius;

    // check explosion collision with sprites
    for (i = 1; i <= max_sprites; i++)
    {
        if (!sprite[i].active or sprite[i].isspectator())
            continue;

        nocollision = guns[weaponnumtoindex(ownerweapon)].nocollision;

        if (((nocollision & weapon_nocollision_exp_enemy) != 0) and
            (sprite[i].isnotinsameteam(sprite[owner])))
            continue;

        if (((nocollision & weapon_nocollision_exp_team) != 0) and
            (sprite[i].isinsameteam(sprite[owner])) && (i != owner))
            continue;

        if (((nocollision & weapon_nocollision_exp_self) != 0) && (owner == i))
            continue;

        if (!sprite[i].deadmeat)
        {
            col = getspritecollisionpoint(i);
            hitboxmodifier = 1.0;

            // if hitpoint is not given find closest one
            w = where;
            if ((i != spritehit) || (where == 0))
            {
                s = std::numeric_limits<float>::max();
                for (j = low(bodyparts); j <= high(bodyparts); j++)
                {
                    a.x = col.x + (sprite[i].skeleton.pos[bodyparts[j]].x - spriteparts.pos[i].x);
                    a.y = col.y + (sprite[i].skeleton.pos[bodyparts[j]].y - spriteparts.pos[i].y);
                    a = vec2subtract(bulletparts.pos[num], a);
                    s2 = vec2length2(a);

                    if (s2 < s)
                    {
                        s = s2;           // squared distance
                        w = bodyparts[j]; // hitpoint index
                    }
                }
            }

            if (w <= 4)
                hitboxmodifier = guns[igun].modifierlegs;
            else if (w <= 11)
                hitboxmodifier = guns[igun].modifierchest;
            else
                hitboxmodifier = guns[igun].modifierhead;

            col.x = col.x + (sprite[i].skeleton.pos[w].x - spriteparts.pos[i].x);
            col.y = col.y + (sprite[i].skeleton.pos[w].y - spriteparts.pos[i].y);

            a = vec2subtract(bulletparts.pos[num], col);
            s = vec2length2(a);

            if (s < explosionradius2)
            {
                s = sqrt(s);

#ifndef SERVER
                createspark(spriteparts.pos[i], vector2(0, -0.01), 5, owner, 80);
                playsound(sfx_explosion_erg, spriteparts.pos[i]);
#endif

                // collision respond
                a.x = (a.x * ((float)(1) / (s + 1)) * explosion_impact_multiply);
                a.y = (a.y * ((float)(1) / (s + 1)) * explosion_impact_multiply);

                if (typ == hit_type_fragnade || typ == hit_type_explode)
                    a.y *= 2.0f;
                else
                    hitboxmodifier *= 0.5f; // cluster/flak is halved

                pushtick = sprite[i].player->pingticks / 2 + ownerpingtick + 1;
                pushtick = min(pushtick, max_pushtick);
                sprite[i].nextpush[pushtick].x -= a.x;
                sprite[i].nextpush[pushtick].y -= a.y;

                if (sprite[i].ceasefirecounter < 0)
                {
                    s = ((float)(1) / (s + 1)) * guns[igun].hitmultiply * hitboxmodifier;
                    sprite[i].healthhit(
#ifndef SERVER
                        srv *
#endif
                            s,
                        owner, 1, num, a);
                }

#ifndef SERVER
                if (canhitspray(i, owner))
                    hitspray();
#endif
            } // s < explosion radius
        }     // not DeadMeat

        if (sprite[i].deadmeat)
        {
            parthit = false;

            for (j = 1; j <= 16; j++)
            {
                a = vec2subtract(bulletparts.pos[num], sprite[i].skeleton.pos[j]);
                s = vec2length2(a);

                if (s < explosionradius2)
                {
                    s = sqrt(s);
                    vec2scale(a, a, ((float)(1) / (s + 1)) * explosion_deadimpact_multiply);
                    sprite[i].skeleton.oldpos[j].x += a.x;
                    sprite[i].skeleton.oldpos[j].y += a.y;

                    parthit = true;
                    s2 = s;
                }
            }

            if (parthit)
            {
                hitboxmodifier = 1.0;

                if (typ == hit_type_explode)
                    s2 = max(s2, 20.0000001f);
                else if (typ == hit_type_cluster || typ == hit_type_flak)
                    hitboxmodifier = 0.5;

                s2 = ((float)(1) / (s2 + 1)) * guns[igun].hitmultiply * hitboxmodifier;
                sprite[i].healthhit(
#ifndef SERVER
                    srv *
#endif
                        s2,
                    owner, 1, num, a);
            }
        }
    } // for Sprite[i]

    // check explosion collision with things
    for (i = 1; i <= max_things; i++)
    {
        if (!thing[i].active or !thing[i].collidewithbullets)
            continue;

        for (j = 1; j <= 4; j++)
        {
            a = vec2subtract(bulletparts.pos[num], thing[i].skeleton.pos[j]);
            s = vec2length2(a);

            if (s < explosionradius2)
            {
                s = sqrt(s);
                vec2scale(a, a, 0.5 * ((float)(1) / (s + 1)) * explosion_impact_multiply);
                thing[i].skeleton.oldpos[j].x += a.x;
                thing[i].skeleton.oldpos[j].y += a.y;
                thing[i].statictype = false;
            }
        }
    } // for Thing[i]

    if (!(typ == hit_type_fragnade || typ == hit_type_explode))
        return;

    // check explosion collision with bullets
    active = false;
    for (i = 1; i <= max_bullets; i++)
    {
        if ((i != num) && bullet[i].active &&
            ((bullet[i].style == bullet_style_fragnade) || (bullet[i].style == bullet_style_m79) ||
             (bullet[i].style == bullet_style_law)))
        {
            a = vec2subtract(bulletparts.pos[num], bulletparts.pos[i]);
            s = vec2length2(a);

            if (s < after_explosion_radius2)
            {
                switch (bullet[i].style)
                {
                case bullet_style_fragnade:
                    bullet[i].hit(hit_type_fragnade);
                    break;
                case bullet_style_m79:
                    bullet[i].hit(hit_type_explode);
                    break;
                case bullet_style_law:
                    bullet[i].hit(hit_type_explode);
                    break;
                }

                bullet[i].kill();
            }
        }
    }

#ifndef SERVER
    // Grenade Effect
    if (CVar::snd_effects_explosions)
    {
        if (mysprite > 0)
        {
            if (sprite[mysprite].health > -50)
            {
                if (distance(bulletparts.pos[num], spriteparts.pos[mysprite]) < grenadeeffect_dist)
                {
                    grenadeeffecttimer = 320;
                    playsound(sfx_hum);
                }
            }
        }
    }

    a = vec2subtract(bulletparts.pos[num], bulletparts.velocity[num]);

    // dirt fly
    if (CVar::r_maxsparks > (max_sparks - 10))
    {
        n = iif(typ == hit_type_fragnade, 6, 7);
        s = iif(typ == hit_type_fragnade, -0.2, -0.15);

        for (i = 1; i <= n; i++)
        {
            vec2scale(b, bulletparts.velocity[num], s);
            b.x = -b.x - 3.5 + ((float)(Random(70)) / 10.0);
            b.y = b.y - 3.5 + ((float)(Random(65)) / 10.0);
            if (Random(4) == 0)
                createspark(a, b, 40, owner, 180 + Random(50));
            if (Random(4) == 0)
                createspark(a, b, 41, owner, 180 + Random(50));
            if (Random(4) == 0)
                createspark(a, b, 42, owner, 180 + Random(50));
            if (Random(4) == 0)
                createspark(a, b, 43, owner, 180 + Random(50));
        }
    }

    // smaller dirt fly
    if (CVar::r_maxsparks > (max_sparks - 10))
    {
        n = iif(typ == hit_type_fragnade, 7, 5);
        s = iif(typ == hit_type_fragnade, -0.2, -0.15);
        rnd = iif(typ == hit_type_fragnade, 4, 3);

        for (i = 1; i <= n; i++)
        {
            vec2scale(b, bulletparts.velocity[num], s);
            b.x = -b.x - 3.5 + ((float)(Random(70)) / 10.0);
            b.y = b.y - 3.5 + ((float)(Random(65)) / 10.0);
            if (Random(rnd) == 0)
                createspark(a, b, 44, owner, 120);
            if (Random(rnd) == 0)
                createspark(a, b, 45, owner, 120);
            if (Random(rnd) == 0)
                createspark(a, b, 46, owner, 120);
            if (Random(rnd) == 0)
                createspark(a, b, 47, owner, 120);
        }
    }

    // iskry fly
    if (CVar::r_maxsparks > (max_sparks - 10))
    {
        n = iif(typ == hit_type_fragnade, 3, 4);
        rnd = iif(typ == hit_type_fragnade, 23, 22);

        for (i = 1; i <= n; i++)
        {
            vec2scale(b, bulletparts.velocity[num], -0.3);
            b.x = -b.x - 3.5 + ((float)(Random(70)) / 10.0);
            b.y = b.y - 3.5 + ((float)(Random(65)) / 10.0);
            if (Random(rnd) == 0)
                createspark(a, b, 2, owner, 120);
            if (Random(rnd) == 0)
                createspark(a, b, 2, owner, 120);
            if (Random(rnd) == 0)
                createspark(a, b, 2, owner, 120);
        }
    }

    // plomyki
    if (CVar::r_maxsparks > (max_sparks - 10))
    {
        n = iif(typ == hit_type_fragnade, 3, 4);
        j = iif(typ == hit_type_fragnade, 25, 20);
        rnd = iif(typ == hit_type_fragnade, 50, 40);
        s = iif(typ == hit_type_fragnade, -0.05, -0.1);

        for (i = 1; i <= n; i++)
        {
            a.x = a.x - j + Random(rnd);
            a.y = a.y - j + Random(rnd);
            vec2scale(b, bulletparts.velocity[num], s);
            b.x = -b.x - 3.5 + ((float)(Random(70)) / 10);
            b.y = b.y - 3.5 + ((float)(Random(65)) / 10);
            createspark(a, b, 64, owner, 35);
        }
    }
#endif
}

template <Config::Module M>
void Bullet<M>::checkoutofbounds()
{
    std::int32_t bound;
    tvector2 *bulletpartspos;

#ifdef SERVER
    LogTraceG("TBullet.CheckOutOfBounds");
#endif

    bound = map.sectorsnum * map.sectorsdivision - 10;
    bulletpartspos = &bulletparts.pos[num];

    if ((fabs(bulletpartspos->x) > bound) || (fabs(bulletpartspos->y) > bound))
        kill();
}

template <Config::Module M>
std::int32_t Bullet<M>::filterspritesbydistance(tspriteindexes &spriteindexes)
{
    std::int32_t i, j, spritecount;
    float roughdistance;
    tspritedistances distances;

    std::int32_t result;
    spritecount = 0;

    for (i = 1; i <= max_sprites; i++)
    {
        if (targetablesprite(i))
        {
            // Get a representation of the distance between this bullet and the sprite
            roughdistance = getcomparablespritedistance(i);

            // Add the sprite index at the appropriate place using a variant of Insertion sort
            spritecount = spritecount + 1;
            j = spritecount;

            while ((j > 1) && (roughdistance < distances[j - 1]))
                j = j - 1;

            std::memmove(&distances[j + 1], &distances[j], (spritecount - j) * sizeof(float));
            distances[j] = roughdistance;

            std::memmove(&spriteindexes[j + 1], &spriteindexes[j],
                         (spritecount - j) * sizeof(std::int32_t));
            spriteindexes[j] = i;
        }
    }

    result = spritecount;
    return result;
}

template <Config::Module M>
bool Bullet<M>::targetablesprite(std::int32_t i)
{
    std::int32_t ownervulnerabletime;

    bool result;
    if (style == bullet_style_fragnade)
        ownervulnerabletime = grenade_timeout - 50;
    else if (style == bullet_style_m2)
        ownervulnerabletime = m2bullet_timeout - 20;
    else if (style == bullet_style_flame)
        ownervulnerabletime = flamer_timeout;
    else
        ownervulnerabletime = bullet_timeout - 20;

    // Check whether a sprite can be hit by this bullet
    result = sprite[i].active && ((owner != i) || (timeout < ownervulnerabletime)) &&
             (hitbody != i) && sprite[i].isnotspectator();
    return result;
}

template <Config::Module M>
float Bullet<M>::getcomparablespritedistance(std::int32_t i)
{
    tvector2 spritecol, distance;

    float result;
    spritecol = getspritecollisionpoint(i);
    distance = vec2subtract(bulletparts.pos[num], spritecol);

    // Faster Euclidean distance calc used for comparisons.
    // Sqrt can be skipped because if "Sqrt[a] < Sqrt[b]" then "a < b"
    result = distance.x * distance.x + distance.y * distance.y;
    return result;
}

template <Config::Module M>
tvector2 Bullet<M>::getspritecollisionpoint(std::int32_t i)
{
    // Why is this an exception to the usual rule??
    tvector2 result;
#ifndef SERVER
    if ((style == bullet_style_flame) && (timeout > flamer_timeout - 2))
    {
        result = spriteparts.pos[i];
        return result;
    }
#endif

    // Ping compensation to get a more accurate collision point
#ifndef SERVER
    if (sprite[i].player->controlmethod != bot)
    {
        if (ownerpingtick < max_oldpos)
            result = oldspritepos[i][ownerpingtick];
        else
            result = oldspritepos[i][max_oldpos - 1];
    }
    else
#endif
        result = spriteparts.pos[i];
    return result;
}

template <Config::Module M>
std::uint8_t Bullet<M>::getweaponindex()
{
    std::uint8_t weaponindex;

    std::uint8_t result;
    for (weaponindex = 1; weaponindex <= high(guns); weaponindex++)
    {
        if (ownerweapon == guns[weaponindex].num)
        {
            result = weaponindex;
            return result;
        }
    }
    result = 0; // Not possible
    return result;
}

template class Bullet<Config::GetModule()>;
