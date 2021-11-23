// automatically converted
#include "Things.hpp"

#include "common/Logging.hpp"

#ifndef SERVER
#include "../../client/Client.hpp"
#include "../../client/GameRendering.hpp"
#include "../../client/Gfx.hpp"
#include "../../client/Sound.hpp"
#else
#include "../../server/Server.hpp"
#include "../network/NetworkServerGame.hpp"
#include "../network/NetworkServerMessages.hpp"
#include "../network/NetworkServerThing.hpp"
#endif

#include "../Calc.hpp"
#include "../Cvar.hpp"
#include "../Game.hpp"
#include "../gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"

#include <numbers>

//clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
//clang-format on

using std::numbers::pi;

template <Config::Module M>
std::int32_t creatething(tvector2 spos, std::uint8_t owner, std::uint8_t sstyle, std::uint8_t n)

{
    std::int32_t i, k, s;
    tvector2 a;
#ifdef SERVER
    tvector2 b;
    float weaponthrowspeedpos1, weaponthrowspeedpos2;
    tvector2 weaponthrowvelocity;
#endif

    std::int32_t result;
    LogTraceG("CreateThing");

    i = 0;
    // Remove flag if a new one is created
    if (sstyle < object_ussocom)
        for (k = 1; k <= max_things; k++)
            if ((thing[k].active) && (thing[k].style == sstyle))
                thing[k].kill();

    if (n == 255)
    {
        s = 1;
        // FIXME (helloer): Check if this should be synced
#ifndef SERVER
        if (sstyle == object_parachute)
            s = max_things / 2;
#endif

        for (i = s; i <= max_things + 1; i++)
        {
            if (i == max_things + 1)
            {
                result = -1;
                return result;
            }
            if (!thing[i].active)
                break;
        }
    }
    else
        i = n; // i is now the active sprite

    // assert((i != 0), "thing id must not be 0");

    // activate sprite
    thing[i].active = true;
    thing[i].style = sstyle;
    thing[i].num = i;
    thing[i].holdingsprite = 0;
    thing[i].owner = owner;
    thing[i].timeout = 0;
    thing[i].skeleton.destroy();
    thing[i].skeleton.timestep = 1;
    thing[i].statictype = false;
    thing[i].inbase = false;
#ifndef SERVER
    thing[i].tex1 = 0;
    thing[i].tex2 = 0;
#endif

    thing[i].bgstate.backgroundstatus = background_transition;
    thing[i].bgstate.backgroundpoly = background_poly_unknown;

    for (k = 1; k <= 4; k++)
        thing[i].collidecount[k] = 0;

    if (owner != 255)
    {
        if (sprite[owner].direction == 1)
            k = 0;
        else
            k = 1;
    }
    else
        k = 0;

    switch (sstyle)
    { // specific style creation
    case object_alpha_flag:
    case object_bravo_flag:
    case object_pointmatch_flag: // Flag
    {
        thing[i].skeleton.vdamping = 0.991;
        thing[i].skeleton.gravity = 1.0 * grav;
        thing[i].skeleton.clone(flagskeleton);
        // A and B flags face eachother.
        if (sstyle == object_alpha_flag)
        {
            thing[i].skeleton.pos[3].x = 12;
            thing[i].skeleton.pos[4].x = 12;
            thing[i].skeleton.oldpos[3].x = 12;
            thing[i].skeleton.oldpos[4].x = 12;
        }
        thing[i].radius = 19;
        if (sstyle != object_pointmatch_flag)
            thing[i].inbase = true;

#ifndef SERVER
        if (CVar::sv_gamemode == gamestyle_inf)
            thing[i].texture = GFX::OBJECTS_INFFLAG;
        else
            thing[i].texture = GFX::OBJECTS_FLAG;
#endif

        thing[i].timeout = flag_timeout;
        thing[i].interest = flag_interest_time;

        thing[i].collidewithbullets = true;

        if ((CVar::sv_gamemode == gamestyle_inf) && (sstyle == object_alpha_flag))
            thing[i].collidewithbullets = false;
    }
    break; // Flag

    case object_ussocom: // Socom
    {
        thing[i].skeleton.vdamping = 0.994;
        thing[i].skeleton.gravity = 1.05 * grav;
        thing[i].skeleton.clone(rifleskeleton10);
#ifndef SERVER
        thing[i].tex1 = GFX::WEAPONS_N_SOCOM + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_desert_eagle: // Deagle
    {
        thing[i].skeleton.vdamping = 0.996;
        thing[i].skeleton.gravity = 1.09 * grav;
        thing[i].skeleton.clone(rifleskeleton11);
#ifndef SERVER
        thing[i].tex1 = GFX::WEAPONS_N_DEAGLES + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_hk_mp5: // Mp5
    {
        thing[i].skeleton.vdamping = 0.995;
        thing[i].skeleton.gravity = 1.11 * grav;
        thing[i].skeleton.clone(rifleskeleton22);
#ifndef SERVER
        thing[i].tex1 = guns[mp5].texturenum + k;
        thing[i].tex2 = guns[mp5].cliptexturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_ak74: // Ak74
    {
        thing[i].skeleton.vdamping = 0.994;
        thing[i].skeleton.gravity = 1.16 * grav;
        thing[i].skeleton.clone(rifleskeleton37);
#ifndef SERVER
        thing[i].tex1 = guns[ak74].texturenum + k;
        thing[i].tex2 = guns[ak74].cliptexturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_steyr_aug: // SteyrAug
    {
        thing[i].skeleton.vdamping = 0.994;
        thing[i].skeleton.gravity = 1.16 * grav;
        thing[i].skeleton.clone(rifleskeleton37);
#ifndef SERVER
        thing[i].tex1 = guns[steyraug].texturenum + k;
        thing[i].tex2 = guns[steyraug].cliptexturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_spas12: // Spas
    {
        thing[i].skeleton.vdamping = 0.993;
        thing[i].skeleton.gravity = 1.15 * grav;
        thing[i].skeleton.clone(rifleskeleton36);
#ifndef SERVER
        thing[i].tex1 = guns[spas12].texturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_ruger77: // Ruger
    {
        thing[i].skeleton.vdamping = 0.993;
        thing[i].skeleton.gravity = 1.13 * grav;
        thing[i].skeleton.clone(rifleskeleton36);
#ifndef SERVER
        thing[i].tex1 = guns[ruger77].texturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_m79: // M79
    {
        thing[i].skeleton.vdamping = 0.994;
        thing[i].skeleton.gravity = 1.15 * grav;
        thing[i].skeleton.clone(rifleskeleton28);
        // FIXME (helloer): Check why Tex1 is different
#ifndef SERVER
        thing[i].tex1 = guns[m79].texturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_barret_m82a1: // Barrett
    {
        thing[i].skeleton.vdamping = 0.993;
        thing[i].skeleton.gravity = 1.18 * grav;
        thing[i].skeleton.clone(rifleskeleton43);
#ifndef SERVER
        thing[i].tex1 = guns[barrett].texturenum + k;
        thing[i].tex2 = guns[barrett].cliptexturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_minimi: // M249
    {
        thing[i].skeleton.vdamping = 0.993;
        thing[i].skeleton.gravity = 1.2 * grav;
        thing[i].skeleton.clone(rifleskeleton39);
#ifndef SERVER
        thing[i].tex1 = guns[m249].texturenum + k;
        thing[i].tex2 = guns[m249].cliptexturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_minigun: // Minigun
    {
        thing[i].skeleton.vdamping = 0.991;
        thing[i].skeleton.gravity = 1.4 * grav;
        thing[i].skeleton.clone(rifleskeleton55);
#ifndef SERVER
        thing[i].tex1 = guns[minigun].texturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_rambo_bow: // Bow
    {
        thing[i].skeleton.vdamping = 0.996;
        thing[i].skeleton.gravity = 0.65 * grav;
        thing[i].skeleton.clone(rifleskeleton50);
#ifndef SERVER
        thing[i].tex1 = GFX::WEAPONS_N_BOW + k;
#endif
        thing[i].radius = bow_radius;
        thing[i].timeout = flag_timeout;
        thing[i].interest = bow_interest_time;
        thing[i].collidewithbullets = true;
    }
    break;
    case object_medical_kit: // medikit
    {
        thing[i].skeleton = boxskeleton;
        thing[i].skeleton.vdamping = 0.989;
        thing[i].skeleton.gravity = 1.05 * grav;
        thing[i].radius = kit_radius;
        thing[i].timeout = CVar::sv_respawntime * gunresisttime;
        thing[i].interest = default_interest_time;
        thing[i].collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing[i].texture = GFX::OBJECTS_MEDIKIT;
#endif
    }
    break;
    case object_grenade_kit: // grenadekit
    {
        thing[i].skeleton = boxskeleton;
        thing[i].skeleton.vdamping = 0.989;
        thing[i].skeleton.gravity = 1.07 * grav;
        thing[i].radius = kit_radius;
        thing[i].timeout = flag_timeout;
        thing[i].interest = default_interest_time;
        thing[i].collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing[i].texture = GFX::OBJECTS_GRENADEKIT;
#endif
    }
    break;
    case object_flamer_kit: // flamerkit
    {
        thing[i].skeleton = boxskeleton;
        thing[i].skeleton.vdamping = 0.989;
        thing[i].skeleton.gravity = 1.17 * grav;
        thing[i].radius = kit_radius;
        thing[i].timeout = flag_timeout;
        thing[i].interest = default_interest_time;
        thing[i].collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing[i].texture = GFX::OBJECTS_FLAMERKIT;
#endif
    }
    break;
    case object_predator_kit: // predatorkit
    {
        thing[i].skeleton = boxskeleton;
        thing[i].skeleton.vdamping = 0.989;
        thing[i].skeleton.gravity = 1.17 * grav;
        thing[i].radius = kit_radius;
        thing[i].timeout = flag_timeout;
        thing[i].interest = default_interest_time;
        thing[i].collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing[i].texture = GFX::OBJECTS_PREDATORKIT;
#endif
    }
    break;
    case object_vest_kit: // vestkit
    {
        thing[i].skeleton = boxskeleton;
        thing[i].skeleton.vdamping = 0.989;
        thing[i].skeleton.gravity = 1.17 * grav;
        thing[i].radius = kit_radius;
        thing[i].timeout = flag_timeout;
        thing[i].interest = default_interest_time;
        thing[i].collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing[i].texture = GFX::OBJECTS_VESTKIT;
#endif
    }
    break;
    case object_berserk_kit: // berserkerkit
    {
        thing[i].skeleton = boxskeleton;
        thing[i].skeleton.vdamping = 0.989;
        thing[i].skeleton.gravity = 1.17 * grav;
        thing[i].radius = kit_radius;
        thing[i].timeout = flag_timeout;
        thing[i].interest = default_interest_time;
        thing[i].collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing[i].texture = GFX::OBJECTS_BERSERKERKIT;
#endif
    }
    break;
    case object_cluster_kit: // clusterkit
    {
        thing[i].skeleton = boxskeleton;
        thing[i].skeleton.vdamping = 0.989;
        thing[i].skeleton.gravity = 1.07 * grav;
        thing[i].radius = kit_radius;
        thing[i].timeout = flag_timeout;
        thing[i].interest = default_interest_time;
        thing[i].collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing[i].texture = GFX::OBJECTS_CLUSTERKIT;
#endif
    }
    break;
    case object_parachute: // para
    {
        thing[i].skeleton.vdamping = 0.993;
        thing[i].skeleton.gravity = 1.15 * grav;
        thing[i].skeleton.clone(paraskeleton);
#ifndef SERVER
        thing[i].tex1 = GFX::GOSTEK_PARA_ROPE;
        thing[i].tex2 = GFX::GOSTEK_PARA;
#endif
        thing[i].timeout = 3600;
    }
    break;
    case object_combat_knife: // Knife
    {
        thing[i].skeleton.vdamping = 0.994;
        thing[i].skeleton.gravity = 1.15 * grav;
        thing[i].skeleton.clone(rifleskeleton18);

        a = thing[i].skeleton.pos[2];
        thing[i].skeleton.pos[2] = thing[i].skeleton.pos[1];
        thing[i].skeleton.oldpos[2] = thing[i].skeleton.pos[1];

        thing[i].skeleton.pos[1] = a;
        thing[i].skeleton.oldpos[1] = a;

        thing[i].skeleton.pos[1].x = thing[i].skeleton.pos[1].x + (float)(Random(100)) / 100;
        thing[i].skeleton.pos[2].x = thing[i].skeleton.pos[2].x - (float)(Random(100)) / 100;

#ifndef SERVER
        thing[i].tex1 = guns[knife].texturenum + k;
        thing[i].tex2 = 0;
#endif
        thing[i].radius = gun_radius * 1.5;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_chainsaw: // Chainsaw
    {
        thing[i].skeleton.vdamping = 0.994;
        thing[i].skeleton.gravity = 1.15 * grav;
        thing[i].skeleton.clone(rifleskeleton28);
#ifndef SERVER
        thing[i].tex1 = guns[chainsaw].texturenum + k;
        thing[i].tex2 = 0;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_law: // LAW
    {
        thing[i].skeleton.vdamping = 0.994;
        thing[i].skeleton.gravity = 1.15 * grav;
        thing[i].skeleton.clone(rifleskeleton28);
#ifndef SERVER
        thing[i].tex1 = guns[law].texturenum + k;
#endif
        thing[i].radius = gun_radius;
        thing[i].timeout = gunresisttime;
        thing[i].interest = 0;
        thing[i].collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_stationary_gun: // stationary gun
    {
        thing[i].skeleton.vdamping = 0.99;
        thing[i].skeleton.gravity = 0.2 * grav;
        thing[i].skeleton.clone(statskeleton);
#ifndef SERVER
        thing[i].tex1 = 0;
        thing[i].tex2 = 0;
#endif
        thing[i].timeout = 60;
        thing[i].radius = stat_radius;
        thing[i].statictype = false;
        thing[i].interest = 0;
        thing[i].collidewithbullets = false;
    }
    break;
    } // case

    thing[i].owner = owner;
    thing[i].moveskeleton(spos.x, spos.y, false);

#ifdef SERVER
    // Throw weapon
    if (((sstyle > object_pointmatch_flag) && (sstyle < object_medical_kit)) ||
        (sstyle == object_law) || (sstyle == object_chainsaw))
        if ((owner > 0) && (owner < max_sprites + 1))
        {
            // Add player velocity
            thing[i].skeleton.pos[1] =
                vec2add(thing[i].skeleton.pos[1], spriteparts.velocity[owner]);
            thing[i].skeleton.pos[2] =
                vec2add(thing[i].skeleton.pos[2], spriteparts.velocity[owner]);

            // Add throw velocity
            b = sprite[owner].getcursoraimdirection();

            if (!sprite[owner].deadmeat)
            {
                weaponthrowspeedpos1 = 0.01;
                weaponthrowspeedpos2 = 3;
            }
            else
            {
                weaponthrowspeedpos1 = 0.02;
                weaponthrowspeedpos2 = 0.64;
            }

            vec2scale(weaponthrowvelocity, b, weaponthrowspeedpos1);
            thing[i].skeleton.pos[1] = vec2add(thing[i].skeleton.pos[1], weaponthrowvelocity);
            vec2scale(weaponthrowvelocity, b, weaponthrowspeedpos2);
            thing[i].skeleton.pos[2] = vec2add(thing[i].skeleton.pos[2], weaponthrowvelocity);
        }

    // send net info
    if (sstyle != object_parachute)
        serverthingmustsnapshot(i);
#endif
    result = i;
    return result;
}

template <Config::Module M>
tthingcollision thingcollision(std::uint8_t thingnum, std::uint32_t cooldownend)
{
    tthingcollision result;
    result.thingnum = thingnum;
    result.cooldownend = cooldownend;
    return result;
}

template <Config::Module M>
bool spawnboxes(tvector2 &start, std::uint8_t team, std::uint8_t num)
{
    std::int32_t i;
    std::int32_t spawnscount;
    PascalArray<std::int32_t, 1, 255> spawns;
    std::int32_t previousspawn;

    bool result;
    LogTraceG("SpawnBoxes");
    result = true;

    start.x = 0;
    start.y = 0;

    for (i = 1; i <= 255; i++)
        spawns[i] = -1;

    spawnscount = 0;
    previousspawn = 0;

    for (i = 1; i <= 255; i++)
        if (map.spawnpoints[i].active)
            if (map.spawnpoints[i].team == team)
                if (thing[num].lastspawn != i)
                {
                    spawnscount += 1;
                    spawns[spawnscount] = i;
                }
                else
                    previousspawn = i;

    if (spawnscount == 0)
    {
        if (previousspawn != 0)
        {
            spawnscount += 1;
            spawns[spawnscount] = previousspawn;
        }
        else
        {
            result = false;
            for (i = 1; i <= 255; i++)
                if (map.spawnpoints[i].active)
                {
                    spawnscount += 1;
                    spawns[spawnscount] = i;
                }
        }
    }

    if (spawnscount > 0)
    {
        i = Random(spawnscount) + 1;
        start.x = map.spawnpoints[spawns[i]].x - 4 + Random(8);
        start.y = map.spawnpoints[spawns[i]].y - 4 + Random(4);
        thing[num].lastspawn = spawns[i];
    }
    return result;
}

template <Config::Module M>
bool randomizestart(tvector2 &start, std::uint8_t team)
{
    std::int32_t i;
    std::int32_t spawnscount;
    PascalArray<std::int32_t, 1, 255> spawns;

    bool result;
    LogTraceG("RandomizeStart {} x {}", start.x, start.y);

    result = true;

    start.x = 0;
    start.y = 0;

    for (i = 1; i <= max_spawnpoints; i++)
        spawns[i] = -1;

    spawnscount = 0;

    for (i = 1; i <= max_spawnpoints; i++)
        if (map.spawnpoints[i].active)
            if (map.spawnpoints[i].team == team)
            {
                spawnscount += 1;
                spawns[spawnscount] = i;
            }

    if (spawnscount == 0)
    {
        result = false;
        for (i = 1; i <= max_spawnpoints; i++)
            if (map.spawnpoints[i].active)
            {
                spawnscount += 1;
                spawns[spawnscount] = i;
            }
    }

    if (spawnscount > 0)
    {
        i = Random(spawnscount) + 1;
        LogDebugG("{} {}", map.spawnpoints[spawns[i]].x, Random(8));
        start.x = map.spawnpoints[spawns[i]].x - 4 + Random(8);
        start.y = map.spawnpoints[spawns[i]].y - 4 + Random(4);
        LogDebugG("{} {}", map.spawnpoints[spawns[i]].y, -4 + Random(4));
    }
    return result;
}

template <Config::Module M>
void Thing<M>::update()
{
    std::int32_t i;
#ifdef SERVER
    std::int32_t j;
#endif
    bool collided, collided2;
    tvector2 a, b;
    bool wasstatic;

#ifdef SERVER
    LogTraceG("TThing.Update");
#endif

    wasstatic = statictype;

    if (!statictype)
    {
        collided = false;
        collided2 = false;

        // Reset the background poly test before collision checks
        bgstate.backgroundtestprepare();

        for (i = 1; i <= 4; i++)
        {
            if (skeleton.active[i] && ((holdingsprite == 0) || ((holdingsprite > 0) && (i == 2))))
            {
                if (style < object_ussocom)
                {
                    if (i == 1)
                    {
                        if (checkmapcollision(i, skeleton.pos[i].x - 10, skeleton.pos[i].y - 8) ||
                            checkmapcollision(i, skeleton.pos[i].x + 10, skeleton.pos[i].y - 8) ||
                            checkmapcollision(i, skeleton.pos[i].x - 10, skeleton.pos[i].y) ||
                            checkmapcollision(i, skeleton.pos[i].x + 10, skeleton.pos[i].y))
                        {
                            if (collided)
                                collided2 = true;
                            collided = true;

                            if (collided)
                                skeleton.forces[2].y =
                                    skeleton.forces[2].y + flag_stand_forceup * grav;
                        }
                    }
                    else
                    {
                        if (checkmapcollision(i, skeleton.pos[i].x, skeleton.pos[i].y))
                        {
                            if (collided)
                                collided2 = true;
                            collided = true;
                        }
                    }
                }
                else if (style >= object_ussocom)
                {
                    if (checkmapcollision(i, skeleton.pos[i].x, skeleton.pos[i].y))
                    {
                        if (collided)
                            collided2 = true;
                        collided = true;
                    }
                }
            } // Skeleton.Active[i]
        }

        // If no background poly contact then reset any background poly status
        bgstate.backgroundtestreset();

        skeleton.doverlettimestep();

        if ((style == object_stationary_gun) && (timeout < 0))
        {
            skeleton.pos[2] = skeleton.oldpos[2];
            skeleton.pos[3] = skeleton.oldpos[3];
        }

        // Make the thing static if not moving much
        a = vec2subtract(skeleton.pos[1], skeleton.oldpos[1]);
        b = vec2subtract(skeleton.pos[2], skeleton.oldpos[2]);
        if (style != object_stationary_gun)
            if (collided && collided2)
                if ((float)((vec2length(a) + vec2length(b))) / 2 < minmovedelta)
                    statictype = true;

        // Sprite is Holding this Flag
        if (style < object_ussocom)
            if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
            {
                skeleton.pos[1] = sprite[holdingsprite].skeleton.pos[8];
                skeleton.forces[2].y = skeleton.forces[2].y + flag_holding_forceup * grav;
                interest = default_interest_time;

                interest = flag_interest_time;

                sprite[holdingsprite].holdedthing = num;
                timeout = flag_timeout;

                if (bgstate.backgroundstatus != background_transition)
                {
                    bgstate.backgroundstatus = sprite[holdingsprite].bgstate.backgroundstatus;
                    bgstate.backgroundpoly = sprite[holdingsprite].bgstate.backgroundpoly;
                }
            } // HoldingSprite > 0
    }

#ifdef SERVER
    LogTraceG("TThing.Update 1");
#endif

    // check if flag is in base
    switch (style)
    {
    case object_alpha_flag:
    case object_bravo_flag: {
        a.x = map.spawnpoints[map.flagspawn[style]].x;
        a.y = map.spawnpoints[map.flagspawn[style]].y;

        if (distance(skeleton.pos[1].x, skeleton.pos[1].y, a.x, a.y) < base_radius)
        {
            inbase = true;
            timeout = flag_timeout;
            interest = flag_interest_time;

#ifdef SERVER
            if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
                if (sprite[holdingsprite].player->team == style)
                    respawn();
#endif
        }
        else
        {
            inbase = false;
        }

        teamflag[style] = num;
    }
    break;
    case object_pointmatch_flag: {
#ifndef SERVER
        teamflag[1] = num;
#endif
    }
    break;
    }

#ifdef SERVER
    LogTraceG("TThing.Update 2");
#endif

// check if flag is touchdown
#ifdef SERVER
    if ((style == object_alpha_flag) || (style == object_bravo_flag))
        if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
            if (sprite[holdingsprite].player->team != style)
            { // check if other flag is inbase
                for (i = 1; i <= max_things; i++)
                    if ((thing[i].active) && (thing[i].inbase) && (i != num) &&
                        (thing[i].holdingsprite == 0)) // check if flags are close
                        if (distance(skeleton.pos[1], thing[i].skeleton.pos[1]) < touchdown_radius)
                        {
                            if (sprite[holdingsprite].player->team == team_alpha)
                            {
#ifndef SERVER
                                if (sv_gamemode.intvalue == gamestyle_inf)
                                    playsound(sfx_infiltmus);
                                else
                                    playsound(sfx_ctf);
#endif

                                sprite[holdingsprite].player->flags =
                                    sprite[holdingsprite].player->flags + 1;
                                teamscore[1] += 1;

                                b.x = 0;
                                b.y = 0;
                                if (CVar::sv_gamemode == gamestyle_inf)
                                {
                                    teamscore[1] += CVar::sv_inf_redaward - 1;
                                    // penalty
                                    if (playersteamnum[1] > playersteamnum[2])
                                        teamscore[1] -= 5 * (playersteamnum[1] - playersteamnum[2]);
                                    if (teamscore[1] < 0)
                                        teamscore[1] = 0;

#ifndef SERVER
                                    // flame it
                                    for (j = 1; j <= 10; j++)
                                    {
                                        a.x = thing[i].skeleton.pos[2].x - 10 + Random(20);
                                        a.y = thing[i].skeleton.pos[2].y - 10 + Random(20);
                                        createspark(a, b, 36, 0, 35);
                                        if (Random(2) == 0)
                                            createspark(a, b, 37, 0, 75);
                                    }
#endif
                                }

#ifndef SERVER
                                // cap spark
                                createspark(thing[i].skeleton.pos[2], b, 61, holdingsprite, 18);
#endif

                                sortplayers();
#ifdef SERVER
                                GetMainConsole().console(sprite[holdingsprite].player->name + ' ' +
                                                             "scores for Alpha Team",
                                                         alpha_message_color);
                                sprite[holdingsprite].player->scorespersecond += 1;
#else
                                bigmessage(_("Alpha Team Scores!"), capturectfmessagewait,
                                           alpha_message_color);
                                GetMainConsole().console(
                                    wideformat(_("{} scores for Alpha Team"),
                                               set::of(sprite[holdingsprite].player->name, eos)),
                                    alpha_message_color);
#endif

                                serverflaginfo(capturered, holdingsprite);
                            }
                            if (sprite[holdingsprite].player->team == team_bravo)
                            {
#ifndef SERVER
                                playsound(sfx_ctf);
#endif
                                sprite[holdingsprite].player->flags =
                                    sprite[holdingsprite].player->flags + 1;
                                teamscore[2] += 1;

#ifndef SERVER
                                // cap spark
                                b.x = 0;
                                b.y = 0;
                                createspark(thing[i].skeleton.pos[2], b, 61, holdingsprite, 15);
#endif

                                sortplayers();
#ifdef SERVER
                                GetMainConsole().console(sprite[holdingsprite].player->name + ' ' +
                                                             "scores for Bravo Team",
                                                         bravo_message_color);
                                sprite[holdingsprite].player->scorespersecond += 1;
#else
                                bigmessage(_("Bravo Team Scores!"), capturectfmessagewait,
                                           bravo_message_color);
                                GetMainConsole().console(
                                    wideformat(_("{} scores for Bravo Team"),
                                               set::of(sprite[holdingsprite].player->name, eos)),
                                    bravo_message_color);
#endif
                                serverflaginfo(captureblue, holdingsprite);
                            }
#ifdef SCRIPT
                            scrptdispatcher.onflagscore(holdingsprite, style);
#endif

                            if (CVar::bots_chat)
                            {
                                if (sprite[holdingsprite].player->controlmethod == bot)
                                    if (Random(sprite[holdingsprite].brain.chatfreq / 3) == 0)
                                        serversendstringmessage(
                                            (sprite[holdingsprite].brain.chatwinning), all_players,
                                            holdingsprite, msgtype_pub);
                            }

                            respawn();

                            if (CVar::sv_survivalmode)
                            {
                                survivalendround = true;

                                // Everyone should die in floatistic after cap
                                // because if nobody is dead the round will not end
                                for (j = 1; j <= max_sprites; j++)
                                    if (sprite[j].active && !sprite[j].deadmeat)
                                    {
                                        sprite[j].healthhit(
#ifdef SERVER
                                            4000
#else
                                            150
#endif
                                            ,
                                            j, 1, -1, sprite[j].skeleton.pos[12]);
                                        sprite[j].player->deaths -= 1;
                                    }
                            }
                        }
            }
#endif
#ifdef SERVER
    LogTraceG("TThing.Update 3");
#endif

    if (style == object_stationary_gun)
    {
#ifndef SERVER
        checkstationaryguncollision(false);
#else
        checkstationaryguncollision();
#endif
    }

    // check if sprite grabs thing
#ifdef SERVER
    if (style != object_stationary_gun)
        checkspritecollision();
#endif

    if (style == object_rambo_bow)
    {
        for (i = 1; i <= max_sprites; i++)
            if (sprite[i].active)
                if ((sprite[i].weapon.num == guns[bow].num) ||
                    (sprite[i].weapon.num == guns[bow2].num))
                {
#ifndef SERVER
                    gamethingtarget = 0;
#endif
                    kill();
                }
    }

#ifndef SERVER
    // flag on wind sound - and para
    if ((style < object_ussocom) || (style == object_parachute))
        if (Random(75) == 0)
            if (vec2length(vec2subtract(skeleton.pos[2], skeleton.oldpos[2])) > 1.0)
                playsound(sfx_flag + Random(2), skeleton.pos[2]);
#endif

    // Parachute
    if (style == object_parachute)
        if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
        {
            skeleton.pos[4] = sprite[holdingsprite].skeleton.pos[12];
            skeleton.forces[1].y = -spriteparts.velocity[holdingsprite].y;
            sprite[holdingsprite].holdedthing = num;

            if (skeleton.pos[3].x < skeleton.pos[4].x)
            {
                a = skeleton.pos[4];
                skeleton.pos[4] = skeleton.pos[3];
                skeleton.oldpos[4] = skeleton.pos[3];
                skeleton.pos[3] = a;
                skeleton.oldpos[3] = a;
                spriteparts.forces[holdingsprite].y = grav;
            }
        }
        else
        {
#ifndef SERVER
            if (timeout > 180)
                timeout = 180;
#endif
        }

#ifdef SERVER
    LogTraceG("TThing.Update 4");
#endif

    // count Time Out
    timeout = timeout - 1;
    if (timeout < -1000)
        timeout = -1000;
    if (timeout == 0)
    {
        switch (style)
        {
        case object_alpha_flag:
        case object_bravo_flag:
        case object_pointmatch_flag:
        case object_rambo_bow: {
#ifdef SERVER
            if (holdingsprite > 0)
                timeout = flag_timeout;
            else
                respawn();
#endif
        }
        break;
        case object_ussocom:
        case object_desert_eagle:
        case object_hk_mp5:
        case object_ak74:
        case object_steyr_aug:
        case object_spas12:
        case object_ruger77:
        case object_m79:
        case object_barret_m82a1:
        case object_minimi:
        case object_minigun:
        case object_combat_knife:
        case object_chainsaw:
        case object_law:
            kill();
            break;
        case object_flamer_kit:
        case object_predator_kit:
        case object_vest_kit:
        case object_berserk_kit:
        case object_cluster_kit:
        case object_parachute:
            kill();
            break;
        } // case
    }     // TimeOut = 0

    checkoutofbounds();

    if ((!wasstatic) && statictype)
        std::memcpy(&skeleton.oldpos[1], &skeleton.pos[1], 4 * sizeof(tvector2));
}

#ifndef SERVER
template <Config::Module M>
void Thing<M>::render(double timeelapsed)
{
    tvector2 _p, a;
    tvector2 _scala, _ra;
    float roto;
    struct tvector2 *pos1, *pos2, *pos3, *pos4;

    tgfxspritearray &t = textures;

    if (CVar::sv_realisticmode)
        if ((owner > 0) && (owner < max_sprites + 1))
            if (sprite[owner].active)
                if (sprite[owner].visible == 0)
                    return;

    pos1 = &skeleton.pos[1];
    pos2 = &skeleton.pos[2];
    pos3 = &skeleton.pos[3];
    pos4 = &skeleton.pos[4];

    // Iluminate the target thing
    if (gamethingtarget == num)
    {
        _p.x = pos1->x + (float)((pos2->x - pos1->x)) / 2 - 12.5;
        _p.y = pos1->y + (float)((pos2->y - pos1->y)) / 2 - 12.5;
        gfxdrawsprite(t[GFX::OBJECTS_ILUM], _p.x, _p.y,
                      rgba(0xffffff, round(fabs(5 + 20 * sin(5.1 * timeelapsed)))));
    }

    switch (style)
    {
    case object_alpha_flag:
    case object_bravo_flag:
    case object_pointmatch_flag: // Flag
    {
        if ((CVar::sv_realisticmode) && (holdingsprite > 0))
            if (sprite[holdingsprite].visible == 0)
                return;

        // fade out (sort of)
        if (timeout < 300)
            if ((timeout % 6) < 3)
                return;

        _p.x = pos1->x;
        _p.y = pos1->y;
        _ra.x = 0;
        _ra.y = 0;
        roto = angle2points(*pos1, *pos2);

        gfxdrawsprite(t[GFX::OBJECTS_FLAG_HANDLE], _p.x, _p.y, _ra.x, _ra.y, -roto);

        if (inbase)
        {
            _p.x = pos1->x + (float)((pos2->x - pos1->x)) / 2 - 12.5;
            _p.y = pos1->y + (float)((pos2->y - pos1->y)) / 2 - 12.5;
            gfxdrawsprite(t[GFX::OBJECTS_ILUM], _p.x, _p.y,
                          rgba(0xffffff, round(fabs(5 + 20 * sin(5.1 * timeelapsed)))));
        }
    }
    break; // 1,2,3

    case object_ussocom:
    case object_desert_eagle:
    case object_hk_mp5:
    case object_ak74:
    case object_steyr_aug:
    case object_spas12:
    case object_ruger77:
    case object_m79:
    case object_barret_m82a1:
    case object_minimi:
    case object_minigun:
    case object_rambo_bow:
    case object_combat_knife:
    case object_chainsaw:
    case object_law: {
        // fade out (sort of)
        if (timeout < 300)
            if ((timeout % 6) < 3)
                return;

        _p.x = pos1->x;
        _p.y = pos1->y - 3;
        _ra.x = 0;
        _ra.y = 2;
        roto = angle2points(*pos1, *pos2);

        if (tex1 == 0)
            return;

        if (CVar::log_level == 1)
        {
            if (!statictype)
                gfxdrawsprite(t[tex1], _p.x, _p.y, _ra.x, _ra.y, -roto);
            else
                gfxdrawsprite(t[tex1], _p.x, _p.y, _ra.x, _ra.y, -roto, rgba(0xffffff, 115));
        }
        else
        {
            gfxdrawsprite(t[tex1], _p.x, _p.y, _ra.x, _ra.y, -roto);
        }

        if (tex2 > 0)
            gfxdrawsprite(t[tex2], _p.x, _p.y, _ra.x, _ra.y, -roto);
    }
    break; // 4,5,6,7,8,9,10,11,12,13,14

    case object_parachute: // para
    {
        // rope 1
        _p.x = pos4->x;
        _p.y = pos4->y - 0.55;
        _ra.x = 0;
        _ra.y = 0.55;
        roto = angle2points(*pos4, *pos2);
        gfxdrawsprite(t[tex1], _p.x, _p.y, _ra.x, _ra.y, -roto);

        // rope 2
        _p.x = pos4->x;
        _p.y = pos4->y - 0.55;
        _ra.x = 0;
        _ra.y = 0.55;
        roto = angle2points(*pos4, *pos3) - (5.0f * pi / 180.f);
        gfxdrawsprite(t[tex1], _p.x, _p.y, _ra.x, _ra.y, -roto);

        // rope 3
        _p.x = pos4->x;
        _p.y = pos4->y - 0.55;
        _ra.x = 0;
        _ra.y = 0.55;
        roto = angle2points(*pos4, *pos1);
        gfxdrawsprite(t[tex1], _p.x, _p.y, _ra.x, _ra.y, -roto);

        a = vec2subtract(*pos2, *pos3);
        _scala.y = vec2length(a) / 45.83;
        _scala.x = _scala.y;
        if (_scala.y > 2.0f)
            return;

        // chute 2
        _p.x = pos3->x;
        _p.y = pos3->y;
        _ra.x = 0;
        _ra.y = 0;
        roto = angle2points(*pos3, *pos1);

        gfxdrawsprite(t[tex2 + 1], _p.x, _p.y, _scala.x, _scala.y, _ra.x, _ra.y, -roto,
                      rgba(color));

        // chute 1
        _p.x = pos1->x;
        _p.y = pos1->y;
        _ra.x = 0;
        _ra.y = 0;
        roto = angle2points(*pos1, *pos2);

        gfxdrawsprite(t[tex2], _p.x, _p.y, _scala.x, _scala.y, _ra.x, _ra.y, -roto, rgba(color));
    }
    break; // 23

    case object_stationary_gun: // stat gun
    {
        _p.x = pos3->x;
        _p.y = pos3->y - 20;
        _ra.x = 0;
        _ra.y = 0;
        roto = angle2points(*pos3, *pos2);
        gfxdrawsprite(t[GFX::WEAPONS_M2_STAT], _p.x, _p.y, _ra.x, _ra.y, -roto);

        _p.x = pos1->x;
        _p.y = pos1->y - 13;
        _ra.x = 5;
        _ra.y = 4;
        roto = angle2points(*pos4, *pos1);
        if (pos4->x >= pos1->x)
            gfxdrawsprite(t[GFX::WEAPONS_M2_2], _p.x, _p.y, _ra.x, _ra.y, roto,
                          rgba(255, 255 - 10 * interest, 255 - 13 * interest));
        else
            gfxdrawsprite(t[GFX::WEAPONS_M2], _p.x, _p.y, _ra.x, _ra.y, roto,
                          rgba(255, 255 - 10 * interest, 255 - 13 * interest));
    }
    break; // 27
    }      // case
}

template <Config::Module M>
void Thing<M>::polygonsrender()
{
    tvector2 pos1, pos2, pos3, pos4;
    tvector2 a;
    tgfxcolor colorbase, colortop, colorlow;
    pgfxrect tc;
    std::vector<tgfxvertex> v{4};

    if (texture == 0)
        return;

    switch (style)
    {
    case object_alpha_flag:
    case object_bravo_flag:
    case object_pointmatch_flag: // Flags
    {
        // fade out (sort of)
        if (timeout < 300)
            if ((timeout % 6) < 3)
                return;

        if ((CVar::sv_realisticmode) && (holdingsprite > 0))
            if (sprite[holdingsprite].visible == 0)
                return;
    }
    break;
    }

    pos1 = skeleton.pos[1];
    pos2 = skeleton.pos[2];
    pos3 = skeleton.pos[3];
    pos4 = skeleton.pos[4];

    switch (style)
    {
    case object_alpha_flag: // Red Flag
    {
        if (CVar::sv_gamemode == gamestyle_inf)
        {
            colorbase = rgba(0xeeeeee);
            colorlow = rgba(0xddeeee);
            colortop = rgba(0xffeeee);
        }
        else
        {
            colorbase = rgba(0xad1515);
            colorlow = rgba(0x951515);
            colortop = rgba(0xb51515);
        }
    }
    break;
    case object_bravo_flag: // Blue Flag
    {
        if (CVar::sv_gamemode == gamestyle_inf)
        {
            colorbase = rgba(0x333333);
            colorlow = rgba(0x333322);
            colortop = rgba(0x333344);
        }
        else
        {
            colorbase = rgba(0x510ad);
            colorlow = rgba(0x51095);
            colortop = rgba(0x510b5);
        }
    }
    break;
    case object_pointmatch_flag: // Yellow Flag
    {
        colorbase = rgba(0xadad15);
        colorlow = rgba(0x959515);
        colortop = rgba(0xb5b515);
    }
    break;
    case object_medical_kit:
    case object_grenade_kit:
    case object_flamer_kit:
    case object_predator_kit:
    case object_vest_kit:
    case object_berserk_kit:
    case object_cluster_kit: // Kits
    {
        colorbase = rgba(0xffffff);
        colorlow = rgba(0xffffff);
        colortop = rgba(0xffffff);
    }
    break;
    }

    switch (style)
    {
    case object_alpha_flag:
    case object_bravo_flag:
    case object_pointmatch_flag: // Flags
    {
        // Move up the position at the handle to halfway between it and the flag tip
        a = vec2subtract(pos2, pos1);
        vec2scale(a, a, 0.5);
        pos1 = vec2add(pos1, a);
    }
    break;
    }

    tc = &textures[texture]->texcoords;

    v[0] = gfxvertex(pos2.x, pos2.y, tc->left, tc->top, colorbase);
    v[1] = gfxvertex(pos1.x, pos1.y, tc->left, tc->bottom, colortop);
    v[2] = gfxvertex(pos4.x, pos4.y, tc->right, tc->bottom, colorbase);
    v[3] = gfxvertex(pos3.x, pos3.y, tc->right, tc->top, colorlow);

    gfxdrawquad(textures[texture]->texture, v);
}
#endif

template <Config::Module M>
bool Thing<M>::checkmapcollision(std::int32_t i, float x, float y)
{
    std::int32_t j, w;
    std::int32_t b = 0;
    tvector2 pos, perp, posdiff, posdiffperp;
    float d = 0.0;
    float posdifflen;
    std::int32_t rx, ry;
    bool teamcol;

    bool result;

#ifdef SERVER
    LogTraceG("TThing.CheckMapCollision");
#endif

    result = false;
    pos.x = x;
    pos.y = y - 0.5;

    // iterate through map polygons
    rx = round((float)(pos.x) / map.sectorsdivision);
    ry = round((float)(pos.y) / map.sectorsdivision);
    if ((rx > -map.sectorsnum) && (rx < map.sectorsnum) && (ry > -map.sectorsnum) &&
        (ry < map.sectorsnum))
    {
        bgstate.backgroundtestbigpolycenter(pos);

        for (j = 1; j <= high(map.sectors[rx][ry].polys); j++)
        {
            w = map.sectors[rx][ry].polys[j];

            teamcol = true;

            if ((owner > 0) && (owner < max_sprites + 1))
            {
                teamcol = teamcollides(w, sprite[owner].player->team, false);
            }

            if ((style < object_ussocom) && (map.polytype[w] > poly_type_lava) &&
                (map.polytype[w] < poly_type_bouncy))
                teamcol = false;

            if (teamcol && (map.polytype[w] != poly_type_only_bullets) &&
                (map.polytype[w] != poly_type_only_player) &&
                (map.polytype[w] != poly_type_doesnt) &&
                (map.polytype[w] != poly_type_only_flaggers) &&
                (map.polytype[w] != poly_type_not_flaggers))
            {
                if (map.pointinpolyedges(pos.x, pos.y, w))
                {
                    if (bgstate.backgroundtest(w))
                        continue;

                    perp = map.closestperpendicular(w, pos, d, b);

                    vec2normalize(perp, perp);
                    vec2scale(perp, perp, d);

                    switch (style)
                    {
                    case object_alpha_flag:
                    case object_bravo_flag:
                    case object_pointmatch_flag: {
                        if (i == 1)
                        {
                            skeleton.pos[i] = skeleton.oldpos[i];
                        }
                        else
                        {
                            // FIXME(skoskav): For more accurate bounce, it should be the sum of the
                            // object's momentum and the intrusion on the polygon's perpendicular

                            // Bounce back the Pos with Perp and move the OldPos behind the new Pos,
                            // so it now travels in the direction of Perp
                            posdiff = vec2subtract(skeleton.pos[i], skeleton.oldpos[i]);
                            posdifflen = vec2length(posdiff);
                            vec2normalize(posdiffperp, perp);
                            vec2scale(posdiffperp, posdiffperp, posdifflen);

                            skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);
                            skeleton.oldpos[i] = vec2add(skeleton.pos[i], posdiffperp);

                            if ((i == 2) && (holdingsprite == 0))
                                skeleton.forces[i].y = skeleton.forces[i].y - 1;
                        }
                    }
                    break;

                    case object_ussocom:
                    case object_desert_eagle:
                    case object_hk_mp5:
                    case object_ak74:
                    case object_steyr_aug:
                    case object_spas12:
                    case object_ruger77:
                    case object_m79:
                    case object_barret_m82a1:
                    case object_minimi:
                    case object_minigun:
                    case object_combat_knife:
                    case object_chainsaw:
                    case object_law: {
                        skeleton.pos[i] = skeleton.oldpos[i];
                        skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);

#ifndef SERVER
                        if ((collidecount[i] == 0) ||
                            ((vec2length(vec2subtract(skeleton.pos[i], skeleton.oldpos[i])) >
                              1.5) &&
                             (collidecount[i] < 30)))
                            playsound(sfx_weaponhit, skeleton.pos[i]);
#endif
                    }
                    break;

                    case object_rambo_bow:
                    case object_medical_kit:
                    case object_grenade_kit:
                    case object_flamer_kit:
                    case object_predator_kit:
                    case object_vest_kit:
                    case object_berserk_kit:
                    case object_cluster_kit: {
                        skeleton.pos[i] = skeleton.oldpos[i];
                        skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);

#ifndef SERVER
                        if ((collidecount[i] == 0) ||
                            ((vec2length(vec2subtract(skeleton.pos[i], skeleton.oldpos[i])) >
                              1.5) &&
                             (collidecount[i] < 3)))
                            playsound(sfx_kit_fall + Random(2), skeleton.pos[i]);
#endif
                    }
                    break;

                    case object_parachute: {
                        skeleton.pos[i] = skeleton.oldpos[i];
                        skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);

#ifndef SERVER
                        if ((collidecount[i] == 0) ||
                            ((vec2length(vec2subtract(skeleton.pos[i], skeleton.oldpos[i])) >
                              1.5) &&
                             (collidecount[i] < 3)))
                            playsound(sfx_flag + Random(2), skeleton.pos[i]);
#endif
                    }
                    break;

                    case object_stationary_gun: {
                        skeleton.pos[i] = skeleton.oldpos[i];
                        skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);
                    }
                    break;
                    }

                    // avoid overflow error
                    collidecount[i] = (std::uint8_t)(collidecount[i] + 1);

                    result = true;
                }
            }
        }
    }
    return result;
}

template <Config::Module M>
void Thing<M>::kill()
{
#ifdef SERVER
    LogTraceG("TThing.Kill");
#endif

    if (num <= 0) // skip uninited Things
        return;
    thing[num].skeleton.destroy();
    active = false;
#ifndef SERVER
    texture = 0;
#endif
}

template <Config::Module M>
void Thing<M>::checkoutofbounds()
{
    std::int32_t i;
    std::int32_t bound;
    struct tvector2 *skeletonpos;

#ifdef SERVER
    LogTraceG("TThing.CheckOutOfBounds");
#endif

    bound = map.sectorsnum * map.sectorsdivision - 10;

    for (i = 1; i <= 4; i++)
    {
        skeletonpos = &skeleton.pos[i];

        if ((fabs(skeletonpos->x) > bound) || (fabs(skeletonpos->y) > bound))
        {
            switch (style)
            {
            case object_alpha_flag:
            case object_bravo_flag:
            case object_pointmatch_flag:
            case object_rambo_bow:
            case object_medical_kit:
            case object_grenade_kit:
            case object_flamer_kit:
            case object_predator_kit:
            case object_vest_kit:
            case object_berserk_kit:
            case object_cluster_kit: {
                respawn();
#ifndef SERVER
                kill();
#endif
            }
            break;

            case object_ussocom:
            case object_desert_eagle:
            case object_hk_mp5:
            case object_ak74:
            case object_steyr_aug:
            case object_spas12:
            case object_ruger77:
            case object_m79:
            case object_barret_m82a1:
            case object_minimi:
            case object_minigun:
            case object_combat_knife:
            case object_chainsaw:
            case object_law:
            case object_stationary_gun: {
                kill();
            }
            break;

            case object_parachute: {
#ifndef SERVER
                if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
                    sprite[holdingsprite].holdedthing = 0;
                holdingsprite = 0;
                kill();
#endif
            }
            break;
            }
        }
    }
}

template <Config::Module M>
void Thing<M>::respawn()
{
    tvector2 a;
    std::int32_t i;

#ifdef SERVER
    LogTraceG("TThing.Respawn");
#endif

    if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
    {
        sprite[holdingsprite].holdedthing = 0;
        if (sprite[holdingsprite].player->team == team_alpha)
            sprite[holdingsprite].brain.pathnum = 1;
        if (sprite[holdingsprite].player->team == team_bravo)
            sprite[holdingsprite].brain.pathnum = 2;
    }

    kill();
    randomizestart(a, 0);

    switch (style)
    {
    case object_alpha_flag:
        randomizestart(a, 5);
        break;
    case object_bravo_flag:
        randomizestart(a, 6);
        break;
    case object_pointmatch_flag:
        randomizestart(a, 14);
        break;
    case object_rambo_bow:
        randomizestart(a, 15);
        break;
    case object_medical_kit:
        spawnboxes(a, 8, num);
        break;
    case object_grenade_kit:
        spawnboxes(a, 7, num);
        break;
    case object_flamer_kit:
        randomizestart(a, 11);
        break;
    case object_predator_kit:
        randomizestart(a, 13);
        break;
    case object_vest_kit:
        randomizestart(a, 10);
        break;
    case object_berserk_kit:
        randomizestart(a, 12);
        break;
    case object_cluster_kit:
        randomizestart(a, 9);
        break;
    }

    creatething(a, 255, style, num);
    thing[num].timeout = flag_timeout;
    thing[num].interest = default_interest_time;
    thing[num].statictype = false;

    for (i = 1; i <= 4; i++)
        thing[num].collidecount[i] = 0;

    if (style == object_rambo_bow)
        thing[num].interest = bow_interest_time;
    if (style < object_ussocom)
        thing[num].interest = flag_interest_time;

        // send net info
#ifdef SERVER
    serverthingmustsnapshot(num);
#endif
}

template <Config::Module M>
void Thing<M>::moveskeleton(float x1, float y1, bool fromzero)
{
    std::int32_t i;

#ifdef SERVER
    LogTraceG("TThing.MoveSkeleton");
#endif

    if (!fromzero)
        for (i = 1; i <= num_particles; i++)
            if (skeleton.active[i])
            {
                skeleton.pos[i].x = skeleton.pos[i].x + x1;
                skeleton.pos[i].y = skeleton.pos[i].y + y1;
                skeleton.oldpos[i] = skeleton.pos[i];
            }

    if (fromzero)
        for (i = 1; i <= num_particles; i++)
            if (skeleton.active[i])
            {
                skeleton.pos[i].x = x1;
                skeleton.pos[i].y = y1;
                skeleton.oldpos[i] = skeleton.pos[i];
            }
}

#ifdef SERVER
template <Config::Module M>
std::int32_t Thing<M>::checkspritecollision()
{
    std::int32_t j, closestplayer;
    tvector2 pos, norm, colpos, a;
    float k, closestdist, dist;
    std::uint8_t weaponindex;
    std::uint32_t capcolor;
#ifdef SERVER
    std::string smallcaptextstr;
#else
    const std::wstring &bigcaptext;
    = '';
    const std::wstring &smallcaptext;
    = '';
#endif

#ifdef SERVER
    std::int32_t result;
    LogTraceG("TThing.CheckSpriteCollision");
#endif

    result = -1;

    a = vec2subtract(skeleton.pos[1], skeleton.pos[2]);
    k = (float)(vec2length(a)) / 2;
    vec2normalize(a, a);
    vec2scale(a, a, -k);
    pos = vec2add(skeleton.pos[1], a);

    // iterate through sprites
    closestdist = 9999999;
    closestplayer = -1;
    for (j = 1; j <= max_sprites; j++)
        if (sprite[j].active && !sprite[j].deadmeat && sprite[j].isnotspectator())
        {
            colpos = spriteparts.pos[j];
            norm = vec2subtract(pos, colpos);
            if (vec2length(norm) >= radius)
            {
                pos = skeleton.pos[1];
                colpos = spriteparts.pos[j];
                norm = vec2subtract(pos, colpos);

                if (vec2length(norm) >= radius)
                {
                    pos = skeleton.pos[2];
                    colpos = spriteparts.pos[j];
                    norm = vec2subtract(pos, colpos);
                }
            }

            dist = vec2length(norm);
            if (dist < radius)
                if (dist < closestdist)
#ifdef SERVER
                    if (!((style == object_medical_kit) && (sprite[j].health == starthealth)))
                        if (!((style == object_grenade_kit) &&
                              (sprite[j].tertiaryweapon.ammocount == CVar::sv_maxgrenades) &&
                              (sprite[j].tertiaryweapon.num == guns[fraggrenade].num)))
#endif
                            if (!((style < object_ussocom) && (sprite[j].ceasefirecounter > 0)))
                            {
                                closestdist = dist;
                                closestplayer = j;
                            }
        } // for j

    j = closestplayer;

    if (j > 0) // collision
    {
        if (((((style > object_pointmatch_flag) && (style < object_rambo_bow) &&
               (sprite[j].bodyanimation.id != change.id)) ||
              ((style > object_parachute) && (sprite[j].bodyanimation.id != change.id))) &&
             (sprite[j].weapon.num == guns[noweapon].num) &&
             (sprite[j].brain.favweapon != guns[noweapon].num) && (timeout < gunresisttime - 30)) ||
            ((style == 15) && (sprite[j].weapon.num == guns[noweapon].num) &&
             (timeout < (CVar::sv_respawntime * flag_timeout - 100))) ||
            (((style == object_medical_kit) && (sprite[j].health < starthealth)
#ifdef SERVER
              && (sprite[j].haspack == false)
#endif
                  ) ||
             ((style == object_grenade_kit) &&
              (sprite[j].tertiaryweapon.ammocount < CVar::sv_maxgrenades) &&
              ((sprite[j].tertiaryweapon.num != guns[clustergrenade].num) ||
               (sprite[j].tertiaryweapon.ammocount == 0)))) ||
            ((((style == object_flamer_kit) && (sprite[j].weapon.num != guns[bow].num) &&
               (sprite[j].weapon.num != guns[bow2].num)) ||
              (style == object_predator_kit) || (style == object_berserk_kit)) &&
             (sprite[j].bonusstyle == bonus_none) && (sprite[j].ceasefirecounter < 1)) ||
            ((style == object_vest_kit)
#ifdef SERVER
             && (sprite[j].vest < defaultvest)
#endif
                 ) ||
            ((style == object_cluster_kit)
#ifdef SERVER
             && ((sprite[j].tertiaryweapon.num == guns[fraggrenade].num) ||
                 (sprite[j].tertiaryweapon.ammocount == 0))
#endif
                 ))
        {
#ifndef SERVER
            if (((style > object_pointmatch_flag) && (style < object_rambo_bow)) ||
                (style > object_parachute)) // take sound
                playsound(sfx_takegun, spriteparts.pos[sprite[j].num]);
            else if (style == object_rambo_bow) // rambo sound
                playsound(sfx_takebow, spriteparts.pos[sprite[j].num]);
            else if (style == object_medical_kit) // take medikit sound
                playsound(sfx_takemedikit, spriteparts.pos[sprite[j].num]);
            else if (style == object_grenade_kit) // take grenade kit sound
                playsound(sfx_pickupgun, spriteparts.pos[sprite[j].num]);
            else if (style == object_flamer_kit) // take flamer kit sound
                playsound(sfx_godflame, spriteparts.pos[sprite[j].num]);
            else if (style == object_predator_kit) // take predator kit sound
                playsound(sfx_predator, spriteparts.pos[sprite[j].num]);
            else if (style == object_vest_kit) // take vest kit sound
                playsound(sfx_vesttake, spriteparts.pos[sprite[j].num]);
            else if (style == object_berserk_kit) // take berserker kit sound
                playsound(sfx_berserker, spriteparts.pos[sprite[j].num]);
            else if (style == object_cluster_kit) // take cluster kit sound
                playsound(sfx_pickupgun, spriteparts.pos[sprite[j].num]);
#endif

#ifdef SERVER
            // Send thing take info through NET
            serverthingtaken(num, j);
#endif
            if (style != object_rambo_bow)
                kill();
        }

#ifdef SERVER
        knifecan[j] = true;
#endif

        switch (style)
        {
        case object_alpha_flag:
        case object_bravo_flag:
        case object_pointmatch_flag: {
            if ((CVar::sv_gamemode == gamestyle_inf) && (style == object_alpha_flag))
                return result;

            // Dont allow flag cap when round has ended
            if (survivalendround && ((style == object_alpha_flag) || (style == object_bravo_flag) ||
                                     (style == object_pointmatch_flag)))
                return result;

            statictype = false;
            timeout = flag_timeout;
            interest = flag_interest_time;

            if ((sprite[j].player->team != style) || !inbase)
            {
                if ((holdingsprite == 0) && (sprite[j].flaggrabcooldown < 1))
                {
#ifndef SERVER
                    // capture sound
                    playsound(sfx_capture, skeleton.pos[1]);
#endif
                    holdingsprite = j;
#ifdef SERVER
                    serverthingtaken(num, j);
#endif

                    capcolor = capture_message_color;
                    switch (CVar::sv_gamemode)
                    {
                    case gamestyle_htf:
                    case gamestyle_ctf:
                        switch (sprite[j].player->team)
                        {
                        case team_alpha:
                            capcolor = alpha_message_color;
                            break;
                        case team_bravo:
                            capcolor = bravo_message_color;
                            break;
                        }
                        break;
                    }

#ifdef SERVER
                    smallcaptextstr = "";
#else
                    smallcaptext = "";
#endif

                    switch (CVar::sv_gamemode)
                    {
                    case gamestyle_pointmatch:
                    case gamestyle_htf: {
#ifdef SERVER
                        smallcaptextstr = sprite[j].player->name + " got the Yellow Flag";
                        if (CVar::sv_gamemode == gamestyle_htf)
                        {
                            sprite[j].player->grabbedinbase = thing[2].inbase;
                            sprite[j].player->grabspersecond += 1;
                        }
#else
                        bigcaptext =
                            iif(j == mysprite, _("You got the Flag!"), _("Yellow Flag captured!"));
                        smallcaptext = _("{} got the Yellow Flag");
#endif
#ifdef SCRIPT
                        scrptdispatcher.onflaggrab(j, style, sprite[j].player->grabbedinbase);
#endif
                    }
                    break;
                    case gamestyle_ctf:
                        if (sprite[j].player->team == style)
                        {
                            switch (sprite[j].player->team)
                            {
                            case team_alpha: {
#ifdef SERVER
                                smallcaptextstr = sprite[j].player->name + " returned the Red Flag";
#else
                                bigcaptext = _("Red flag returned!");
                                smallcaptext = _("%s returned the Red Flag");
#endif
                            }
                            break;
                            case team_bravo: {
#ifdef SERVER
                                smallcaptextstr =
                                    sprite[j].player->name + " returned the Blue Flag";
#else
                                bigcaptext = _("Blue Flag returned!");
                                smallcaptext = _("%s returned the Blue Flag");
#endif
                            }
                            break;
                            }
                            respawn();
#ifdef SCRIPT
                            scrptdispatcher.onflagreturn(j, style);
#endif
                        }
                        else
                        {
                            switch (sprite[j].player->team)
                            {
                            case team_alpha: {
#ifdef SERVER
                                smallcaptextstr =
                                    sprite[j].player->name + " captured the Blue Flag";
                                sprite[j].player->grabbedinbase = thing[2].inbase;
                                sprite[j].player->grabspersecond += 1;
#else
                                bigcaptext = iif(j == mysprite, _("You got the Blue Flag!"),
                                                 _("Blue Flag captured!"));
                                smallcaptext = _("%s captured the Blue Flag");
#endif
                            }
                            break;
                            case team_bravo: {
#ifdef SERVER
                                smallcaptextstr = sprite[j].player->name + " captured the Red Flag";
                                sprite[j].player->grabbedinbase = thing[1].inbase;
                                sprite[j].player->grabspersecond += 1;
#else
                                bigcaptext = iif(j == mysprite, _("You got the Red Flag!"),
                                                 _("Red Flag captured!"));
                                smallcaptext = _("%s captured the Red Flag");
#endif
                            }
                            break;
                            }
#ifdef SCRIPT
                            scrptdispatcher.onflaggrab(j, style, sprite[j].player->grabbedinbase);
#endif
                        }
                        break;
                    case gamestyle_inf:
                        if (sprite[j].player->team == style)
                        {
                            if (sprite[j].player->team == team_bravo)
                            {
#ifdef SERVER
                                smallcaptextstr =
                                    sprite[j].player->name + " returned the Objective";
                                sprite[j].player->grabbedinbase = thing[2].inbase;
                                sprite[j].player->grabspersecond += 1;
#else
                                bigcaptext = iif(j == mysprite, _("You returned the Objective!"),
                                                 _("Objective returned!"));
                                smallcaptext = _("%s returned the Objective");
#endif
                            }
#ifdef SCRIPT
                            scrptdispatcher.onflagreturn(j, style);
#endif
                            respawn();
                        }
                        else
                        {
                            if (sprite[j].player->team == team_alpha)
                            {
#ifdef SERVER
                                smallcaptextstr =
                                    sprite[j].player->name + " captured the Objective";
                                sprite[j].player->grabbedinbase = thing[2].inbase;
                                sprite[j].player->grabspersecond += 1;
#else
                                bigcaptext = iif(j == mysprite, _("You got the Objective!"),
                                                 _("Objective captured!"));
                                smallcaptext = _("%s captured the Objective");
#endif
                            }
#ifdef SCRIPT
                            scrptdispatcher.onflaggrab(j, style, sprite[j].player->grabbedinbase);
#endif
                        }
                        break;
                    }

#ifdef SERVER
                    if (smallcaptextstr != "")
                        GetMainConsole().console(smallcaptextstr, capcolor);
#else
                    if (smallcaptext != "")
                    {
                        bigmessage(bigcaptext, capturemessagewait, capcolor);
                        GetMainConsole().console(
                            wideformat(smallcaptext, set::of(sprite[j].player->name, eos)),
                            capcolor);
                    }
#endif
                }
            }
        }
        break;
        case object_ussocom:
        case object_desert_eagle:
        case object_hk_mp5:
        case object_ak74:
        case object_steyr_aug:
        case object_spas12:
        case object_ruger77:
        case object_m79:
        case object_barret_m82a1:
        case object_minimi:
        case object_minigun:
            if (sprite[j].weapon.num == guns[noweapon].num)
                if (sprite[j].brain.favweapon != guns[noweapon].num)
                    if (sprite[j].bodyanimation.id != change.id)
                        if (timeout < gunresisttime - 30)
                        {
                            // Objects 1-3 are flags, so we need for WeaponIndex subtract by flags+1
                            weaponindex = weaponnumtoindex(style - (object_num_flags + 1));
#ifdef SCRIPT
                            // event must be before actual weapon apply.
                            // script might've called ForceWeapon, which we should check.
                            // if it did, we don't apply snapshot weapon's as they were already
                            // applied by force weapon.
                            forceweaponcalled = false;
                            scrptdispatcher.onweaponchange(j, guns[weaponindex].num,
                                                           sprite[j].secondaryweapon.num, ammocount,
                                                           sprite[j].secondaryweapon.ammocount);

                            if (!forceweaponcalled)
                            {
                                ;
#endif
                                sprite[j].applyweaponbynum(guns[weaponindex].num, 1);
                                sprite[j].weapon.ammocount = ammocount;
                                sprite[j].weapon.fireintervalprev = sprite[j].weapon.fireinterval;
                                sprite[j].weapon.fireintervalcount = sprite[j].weapon.fireinterval;
#ifndef SERVER
                                if (j == mysprite)
                                    clientspritesnapshot;
#endif
#ifdef SCRIPT
                            }
#endif
                        }
            break;
        case object_rambo_bow:
            if (sprite[j].weapon.num == guns[noweapon].num)
                if (sprite[j].bodyanimation.id != change.id)
                    if (timeout < flag_timeout - 100)
                    {
#ifdef SCRIPT
                        // event must be before actual weapon apply.
                        // script might've called ForceWeapon, which we should check.
                        // if it did, we don't apply snapshot weapon's as they were already applied
                        // by force weapon.
                        forceweaponcalled = false;
                        scrptdispatcher.onweaponchange(j, guns[bow].num, guns[bow2].num, 1, 1);

                        if (!forceweaponcalled)
                        {
#endif
                            sprite[j].applyweaponbynum(guns[bow].num, 1);
                            sprite[j].applyweaponbynum(guns[bow2].num, 2);
                            // BUG: shouldn't this be Guns[BOW].Ammo? Somebody might've set more
                            // than one
                            sprite[j].weapon.ammocount = 1;
                            sprite[j].weapon.fireintervalprev = sprite[j].weapon.fireinterval;
                            sprite[j].weapon.fireintervalcount = sprite[j].weapon.fireinterval;
                            sprite[j].wearhelmet = 1;
#ifndef SERVER
                            if (j == mysprite)
                                clientspritesnapshot;
#endif
#ifdef SCRIPT
                        }
#endif

#ifndef SERVER
                        if (j == mysprite)
                        {
                            bigmessage(_("You got the Bow!"), capturemessagewait,
                                       capture_message_color);
                            if (!limbolock)
                                gamemenushow(limbomenu, false);
                        }
                        else
                            bigmessage(wideformat(_("%s got the Bow!"),
                                                  set::of(sprite[j].player->name, eos)),
                                       capturemessagewait, capture_message_color);
#endif
                    }
            break;
        case object_medical_kit: {
            if (sprite[j].health < starthealth)
            {
                // pickup health pack
#ifdef SERVER
                if (sprite[j].haspack == false)
#endif
                {
                    team = sprite[j].player->team;
#ifdef SERVER
                    if (CVar::sv_healthcooldown > 0)
                        sprite[j].haspack = true;
#endif
                    sprite[j].health = starthealth;
                    respawn();

#ifdef SCRIPT
                    scrptdispatcher.onkitpickup(j, num);
#endif
                }
            }
        }
        break;
        case object_grenade_kit: {
            if ((sprite[j].tertiaryweapon.ammocount < CVar::sv_maxgrenades) &&
                ((sprite[j].tertiaryweapon.num != guns[clustergrenade].num) ||
                 (sprite[j].tertiaryweapon.ammocount == 0)))
            {
                team = sprite[j].player->team;
                sprite[j].tertiaryweapon = guns[fraggrenade];
                sprite[j].tertiaryweapon.ammocount = CVar::sv_maxgrenades;
                respawn();

#ifdef SCRIPT
                scrptdispatcher.onkitpickup(j, num);
#endif
            }
        }
        break;
        case object_flamer_kit:
            if ((sprite[j].bonusstyle == bonus_none) && (sprite[j].ceasefirecounter < 1))
            {
                if ((sprite[j].weapon.num != guns[bow].num) &&
                    (sprite[j].weapon.num != guns[bow2].num))
                {
#ifdef SCRIPT
                    // event must be before actual weapon apply.
                    // script might've called ForceWeapon, which we should check.
                    // if it did, we don't apply snapshot weapon's as they were already applied
                    // by force weapon.
                    forceweaponcalled = false;
                    scrptdispatcher.onweaponchange(j, guns[flamer].num,
                                                   sprite[j].secondaryweapon.num, ammocount,
                                                   sprite[j].secondaryweapon.ammocount);
                    scrptdispatcher.onkitpickup(j, num);

                    if (!forceweaponcalled)
                    {
                        ;
#endif
                        sprite[j].applyweaponbynum(sprite[j].weapon.num, 2
#ifndef SERVER
                                                   ,
                                                   -1, true
#endif
                        );
                        sprite[j].applyweaponbynum(guns[flamer].num, 1);
                        sprite[j].bonustime = flamerbonustime;
                        sprite[j].bonusstyle = bonus_flamegod;

#ifndef SERVER
                        if (j == mysprite)
                        {
                            bigmessage(_("Flame God Mode!"), capturemessagewait,
                                       bonus_message_color);
                            clientspritesnapshot;
                        }
#endif

                        sprite[j].health = starthealth;
#ifdef SCRIPT
                    }
#endif
                }
            }
            break;
        case object_predator_kit:
            if ((sprite[j].bonusstyle == bonus_none) && (sprite[j].ceasefirecounter < 1))
            {
                sprite[j].alpha = predatoralpha;
                sprite[j].bonustime = predatorbonustime;
                sprite[j].bonusstyle = bonus_predator;

#ifndef SERVER
                if (j == mysprite)
                    bigmessage(_("Predator Mode!"), capturemessagewait, bonus_message_color);
#endif

                sprite[j].health = starthealth;

#ifdef SCRIPT
                scrptdispatcher.onkitpickup(j, num);
#endif
            }
            break;
        case object_vest_kit: {
            sprite[j].vest = defaultvest;

#ifndef SERVER
            if (j == mysprite)
                bigmessage(_("Bulletproof Vest!"), capturemessagewait, capture_message_color);
#endif

#ifdef SCRIPT
            scrptdispatcher.onkitpickup(j, num);
#endif
        }
        break;
        case object_berserk_kit:
            if ((sprite[j].bonusstyle == 0) && (sprite[j].ceasefirecounter < 1))
            {
                sprite[j].bonusstyle = bonus_berserker;
                sprite[j].bonustime = berserkerbonustime;

#ifndef SERVER
                if (j == mysprite)
                    bigmessage(_("Berserker Mode!"), capturemessagewait, bonus_message_color);
#endif

                sprite[j].health = starthealth;

#ifdef SCRIPT
                scrptdispatcher.onkitpickup(j, num);
#endif
            }
            break;
        case object_cluster_kit:
#ifdef SERVER
            if ((sprite[j].tertiaryweapon.num == guns[fraggrenade].num) ||
                (sprite[j].tertiaryweapon.ammocount == 0))
#endif
            {
                sprite[j].tertiaryweapon = guns[clustergrenade];
                sprite[j].tertiaryweapon.ammocount = cluster_grenades;

#ifndef SERVER
                if (j == mysprite)
                    bigmessage(_("Cluster grenades!"), capturemessagewait, capture_message_color);
#endif

#ifdef SCRIPT
                scrptdispatcher.onkitpickup(j, num);
#endif
            }
            break;
        case object_combat_knife:
        case object_chainsaw:
        case object_law:
            if (sprite[j].weapon.num == guns[noweapon].num)
                if (sprite[j].brain.favweapon != guns[noweapon].num)
                    if (sprite[j].bodyanimation.id != change.id)
                        if (timeout < gunresisttime - 30)
                        {
                            // There are in total OBJECT_NUM_NONWEAPON non-weapon objects before the
                            // knife so we need to subtract it+1 for the WeaponIndex (like before)
                            weaponindex = weaponnumtoindex(style - (object_num_nonweapon + 1));
#ifdef SCRIPT
                            // event must be before actual weapon apply.
                            // script might've called ForceWeapon, which we should check.
                            // if it did, we don't apply snapshot weapon's as they were already
                            // applied by force weapon.
                            forceweaponcalled = false;
                            scrptdispatcher.onweaponchange(j, guns[weaponindex].num,
                                                           sprite[j].secondaryweapon.num, ammocount,
                                                           sprite[j].secondaryweapon.ammocount);

                            if (!forceweaponcalled)
                            {
#endif
                                sprite[j].applyweaponbynum(guns[weaponindex].num, 1);
                                sprite[j].weapon.ammocount = ammocount;
                                sprite[j].weapon.fireintervalprev = sprite[j].weapon.fireinterval;
                                sprite[j].weapon.fireintervalcount = sprite[j].weapon.fireinterval;
#ifndef SERVER
                                if (j == mysprite)
                                    clientspritesnapshot;
#endif
#ifdef SCRIPT
                            }
#endif
                        }
            break;
        }

        result = j;
    }
    return result;
}
#endif

#ifdef SERVER
template <Config::Module M>
std::int32_t Thing<M>::checkstationaryguncollision()
#else
template <Config::Module M>
std::int32_t Thing<M>::checkstationaryguncollision(bool clientcheck)
#endif
{
    std::int32_t i, j, k;
    tvector2 pos, norm, colpos;

    std::int32_t result;
#ifdef SERVER
    LogTraceG("TThing.CheckStationaryGunCollision");
#endif

    result = -1;
    if (timeout > 0)
        return result;

    // Stat overheat less
    if (interest > m2gun_overheat + 1)
        interest = 0;

    if (interest > 0)
        if (maintickcounter % 8 == 0)
            interest -= 1;

    pos = skeleton.pos[1];

    for (i = 1; i <= max_sprites; i++)
        if (sprite[i].active && !sprite[i].deadmeat && sprite[i].isnotspectator())
            if (sprite[i].stat == num)
            {
                colpos = spriteparts.pos[i];
                norm = vec2subtract(pos, colpos);
                if (vec2length(norm) < radius) // collision
                {
                    if (sprite[i].player->controlmethod == bot)
                        if (sprite[i].brain.camper > 0)
                            if (sprite[i].holdedthing == 0)
                            {
                                sprite[i].control.right = false;
                                sprite[i].control.left = false;
                                sprite[i].control.up = false;
                                sprite[i].control.down = false;
                                if (sprite[i].legsanimation.id == prone.id)
                                    sprite[i].control.prone = true;
                            }

                    statictype = true;

                    pos.x = sprite[i].control.mouseaimx;
                    pos.y = sprite[i].control.mouseaimy;
                    norm = vec2subtract(pos, sprite[i].skeleton.pos[15]);
                    vec2normalize(norm, norm);
                    vec2scale(norm, norm, 3);
                    norm.x = -norm.x;
                    skeleton.oldpos[4] = skeleton.pos[4];
                    skeleton.pos[4] = vec2add(skeleton.pos[1], norm);

                    interest = sprite[i].usetime;

                    if (sprite[i].control.fire)
                        if (sprite[i].legsanimation.id == stand.id)
                            if (maintickcounter % guns[m2].fireinterval == 0)
                            {
                                if (sprite[i].usetime > m2gun_overheat)
                                {
#ifndef SERVER
                                    playsound(sfx_m2overheat, spriteparts.pos[i]);
#endif
                                    return result;
                                }

                                k = 0;
                                if (sprite[i].usetime > m2gun_overaim)
                                {
                                    k = (sprite[i].usetime / 11);
                                    k = -k + Random(2 * k);
                                }

                                pos.x = skeleton.pos[4].x;
                                pos.y = skeleton.pos[4].y;
                                norm = vec2subtract(pos, skeleton.pos[1]);
                                vec2normalize(norm, norm);
                                vec2scale(norm, norm, guns[m2].speed);
                                norm.x = -norm.x;
                                norm.x = norm.x + k;
                                norm.y = norm.y + k;
                                pos.x = skeleton.pos[4].x + 4;
                                pos.y = skeleton.pos[4].y - 10;

                                createbullet(pos, norm, guns[m2].num, i, 255, guns[m2].hitmultiply,
                                             true, false);

#ifndef SERVER
                                pos = vec2add(pos, norm);
                                vec2scale(norm, norm, 0.1);
                                createspark(pos, norm, 35, i, 15); // smoke

                                colpos = norm;
                                norm.y = -sprite[i].direction * norm.x;
                                norm.x = sprite[i].direction * colpos.y;
                                vec2scale(norm, norm, 0.2);
                                pos.x = skeleton.pos[3].x + 18;
                                pos.y = skeleton.pos[3].y - 20;
                                createspark(pos, norm, 22, i, 255); // hull

                                playsound(sfx_m2fire, spriteparts.pos[i]);
#endif

                                sprite[i].usetime += 1;
                            }
                    return result;
                }
                else
                {
                    sprite[i].stat = 0;
                    statictype = false;
                    return result;
                }
            }

            // iterate through sprites
#ifndef SERVER
    if (clientcheck)
#endif
        if (!statictype)
            for (j = 1; j <= max_sprites; j++)
                if (sprite[j].active && !sprite[j].deadmeat && sprite[j].isnotspectator())
                {
                    colpos = spriteparts.pos[j];
                    norm = vec2subtract(pos, colpos);
                    if (vec2length(norm) < radius) // collision
                    {
                        switch (style)
                        {
                        case object_stationary_gun: {
                            if (sprite[j].player->controlmethod == bot)
                                if (sprite[j].brain.camper > 0)
                                {
                                    sprite[j].control.right = false;
                                    sprite[j].control.left = false;
                                    sprite[j].control.up = false;
                                    sprite[j].control.down = false;
                                    sprite[j].legsapplyanimation(stand, 1);
                                }

                            if (sprite[j].legsanimation.id == stand.id)
                            {
#ifndef SERVER
                                playsound(sfx_m2use, spriteparts.pos[j]);
#endif

                                statictype = true;
                                sprite[j].stat = num;
                                sprite[j].brain.oneplacecount = 0;
#ifdef SERVER
                                serverthingtaken(num, j);
#endif
                            }
                        }
                        break;
                        }

                        result = j;
                        return result;
                    }
                    else if (sprite[j].stat == num)
                        sprite[j].stat = 0;

                    statictype = false;
                } // for j
    return result;
}

template class Thing<>;
template std::int32_t creatething(tvector2 spos, std::uint8_t owner, std::uint8_t sstyle,
                                  std::uint8_t n);
template tthingcollision thingcollision(std::uint8_t thingnum, std::uint32_t cooldownend);
template bool spawnboxes(tvector2 &start, std::uint8_t team, std::uint8_t num);
template bool randomizestart(tvector2 &start, std::uint8_t team);
