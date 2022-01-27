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

#include "../Cvar.hpp"
#include "../Game.hpp"
#include "SpriteSystem.hpp"
#include "common/Calc.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/misc/GlobalSystems.hpp"

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

    auto &guns = GS::GetWeaponSystem().GetGuns();
    auto things = GS::GetThingSystem().GetThings();

    i = 0;
    // Remove flag if a new one is created
    if (sstyle < object_ussocom)
        for (k = 1; k <= max_things; k++)
            if ((things[k].active) && (things[k].style == sstyle))
                things[k].kill();

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
            if (!things[i].active)
                break;
        }
    }
    else
        i = n; // i is now the active sprite

    // assert((i != 0), "thing id must not be 0");

    // activate sprite
    auto &thing = things[i];
    thing.active = true;
    thing.style = sstyle;
    thing.num = i;
    thing.holdingsprite = 0;
    thing.owner = owner;
    thing.timeout = 0;
    thing.skeleton.destroy();
    thing.skeleton.timestep = 1;
    thing.statictype = false;
    thing.inbase = false;
#ifndef SERVER
    thing.tex1 = 0;
    thing.tex2 = 0;
#endif

    thing.bgstate.backgroundstatus = background_transition;
    thing.bgstate.backgroundpoly = background_poly_unknown;

    for (k = 1; k <= 4; k++)
        thing.collidecount[k] = 0;

    if (owner != 255)
    {
        if (SpriteSystem::Get().GetSprite(owner).direction == 1)
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
        thing.skeleton.vdamping = 0.991;
        thing.skeleton.gravity = 1.0 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Flag));
        // A and B flags face eachother.
        if (sstyle == object_alpha_flag)
        {
            thing.skeleton.pos[3].x = 12;
            thing.skeleton.pos[4].x = 12;
            thing.skeleton.oldpos[3].x = 12;
            thing.skeleton.oldpos[4].x = 12;
        }
        thing.radius = 19;
        if (sstyle != object_pointmatch_flag)
            thing.inbase = true;

#ifndef SERVER
        if (CVar::sv_gamemode == gamestyle_inf)
            thing.texture = GFX::OBJECTS_INFFLAG;
        else
            thing.texture = GFX::OBJECTS_FLAG;
#endif

        thing.timeout = flag_timeout;
        thing.interest = flag_interest_time;

        thing.collidewithbullets = true;

        if ((CVar::sv_gamemode == gamestyle_inf) && (sstyle == object_alpha_flag))
            thing.collidewithbullets = false;
    }
    break; // Flag

    case object_ussocom: // Socom
    {
        thing.skeleton.vdamping = 0.994;
        thing.skeleton.gravity = 1.05 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle10));
#ifndef SERVER
        thing.tex1 = GFX::WEAPONS_N_SOCOM + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_desert_eagle: // Deagle
    {
        thing.skeleton.vdamping = 0.996;
        thing.skeleton.gravity = 1.09 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle11));
#ifndef SERVER
        thing.tex1 = GFX::WEAPONS_N_DEAGLES + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_hk_mp5: // Mp5
    {
        thing.skeleton.vdamping = 0.995;
        thing.skeleton.gravity = 1.11 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle22));
#ifndef SERVER
        thing.tex1 = guns[mp5].texturenum + k;
        thing.tex2 = guns[mp5].cliptexturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_ak74: // Ak74
    {
        thing.skeleton.vdamping = 0.994;
        thing.skeleton.gravity = 1.16 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle37));
#ifndef SERVER
        thing.tex1 = guns[ak74].texturenum + k;
        thing.tex2 = guns[ak74].cliptexturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_steyr_aug: // SteyrAug
    {
        thing.skeleton.vdamping = 0.994;
        thing.skeleton.gravity = 1.16 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle37));
#ifndef SERVER
        thing.tex1 = guns[steyraug].texturenum + k;
        thing.tex2 = guns[steyraug].cliptexturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_spas12: // Spas
    {
        thing.skeleton.vdamping = 0.993;
        thing.skeleton.gravity = 1.15 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle36));
#ifndef SERVER
        thing.tex1 = guns[spas12].texturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_ruger77: // Ruger
    {
        thing.skeleton.vdamping = 0.993;
        thing.skeleton.gravity = 1.13 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle36));
#ifndef SERVER
        thing.tex1 = guns[ruger77].texturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_m79: // M79
    {
        thing.skeleton.vdamping = 0.994;
        thing.skeleton.gravity = 1.15 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle28));
        // FIXME (helloer): Check why Tex1 is different
#ifndef SERVER
        thing.tex1 = guns[m79].texturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_barret_m82a1: // Barrett
    {
        thing.skeleton.vdamping = 0.993;
        thing.skeleton.gravity = 1.18 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle43));
#ifndef SERVER
        thing.tex1 = guns[barrett].texturenum + k;
        thing.tex2 = guns[barrett].cliptexturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_minimi: // M249
    {
        thing.skeleton.vdamping = 0.993;
        thing.skeleton.gravity = 1.2 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle39));
#ifndef SERVER
        thing.tex1 = guns[m249].texturenum + k;
        thing.tex2 = guns[m249].cliptexturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_minigun: // Minigun
    {
        thing.skeleton.vdamping = 0.991;
        thing.skeleton.gravity = 1.4 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle55));
#ifndef SERVER
        thing.tex1 = guns[minigun].texturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_rambo_bow: // Bow
    {
        thing.skeleton.vdamping = 0.996;
        thing.skeleton.gravity = 0.65 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle50));
#ifndef SERVER
        thing.tex1 = GFX::WEAPONS_N_BOW + k;
#endif
        thing.radius = bow_radius;
        thing.timeout = flag_timeout;
        thing.interest = bow_interest_time;
        thing.collidewithbullets = true;
    }
    break;
    case object_medical_kit: // medikit
    {
        thing.skeleton = AnimationSystem::Get().GetSkeleton(Box);
        thing.skeleton.vdamping = 0.989;
        thing.skeleton.gravity = 1.05 * grav;
        thing.radius = kit_radius;
        thing.timeout = CVar::sv_respawntime * gunresisttime;
        thing.interest = default_interest_time;
        thing.collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing.texture = GFX::OBJECTS_MEDIKIT;
#endif
    }
    break;
    case object_grenade_kit: // grenadekit
    {
        thing.skeleton = AnimationSystem::Get().GetSkeleton(Box);
        thing.skeleton.vdamping = 0.989;
        thing.skeleton.gravity = 1.07 * grav;
        thing.radius = kit_radius;
        thing.timeout = flag_timeout;
        thing.interest = default_interest_time;
        thing.collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing.texture = GFX::OBJECTS_GRENADEKIT;
#endif
    }
    break;
    case object_flamer_kit: // flamerkit
    {
        thing.skeleton = AnimationSystem::Get().GetSkeleton(Box);
        thing.skeleton.vdamping = 0.989;
        thing.skeleton.gravity = 1.17 * grav;
        thing.radius = kit_radius;
        thing.timeout = flag_timeout;
        thing.interest = default_interest_time;
        thing.collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing.texture = GFX::OBJECTS_FLAMERKIT;
#endif
    }
    break;
    case object_predator_kit: // predatorkit
    {
        thing.skeleton = AnimationSystem::Get().GetSkeleton(Box);
        thing.skeleton.vdamping = 0.989;
        thing.skeleton.gravity = 1.17 * grav;
        thing.radius = kit_radius;
        thing.timeout = flag_timeout;
        thing.interest = default_interest_time;
        thing.collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing.texture = GFX::OBJECTS_PREDATORKIT;
#endif
    }
    break;
    case object_vest_kit: // vestkit
    {
        thing.skeleton = AnimationSystem::Get().GetSkeleton(Box);
        thing.skeleton.vdamping = 0.989;
        thing.skeleton.gravity = 1.17 * grav;
        thing.radius = kit_radius;
        thing.timeout = flag_timeout;
        thing.interest = default_interest_time;
        thing.collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing.texture = GFX::OBJECTS_VESTKIT;
#endif
    }
    break;
    case object_berserk_kit: // berserkerkit
    {
        thing.skeleton = AnimationSystem::Get().GetSkeleton(Box);
        thing.skeleton.vdamping = 0.989;
        thing.skeleton.gravity = 1.17 * grav;
        thing.radius = kit_radius;
        thing.timeout = flag_timeout;
        thing.interest = default_interest_time;
        thing.collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing.texture = GFX::OBJECTS_BERSERKERKIT;
#endif
    }
    break;
    case object_cluster_kit: // clusterkit
    {
        thing.skeleton = AnimationSystem::Get().GetSkeleton(Box);
        ;
        thing.skeleton.vdamping = 0.989;
        thing.skeleton.gravity = 1.07 * grav;
        thing.radius = kit_radius;
        thing.timeout = flag_timeout;
        thing.interest = default_interest_time;
        thing.collidewithbullets = CVar::sv_kits_collide;
#ifndef SERVER
        thing.texture = GFX::OBJECTS_CLUSTERKIT;
#endif
    }
    break;
    case object_parachute: // para
    {
        thing.skeleton.vdamping = 0.993;
        thing.skeleton.gravity = 1.15 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Para));
#ifndef SERVER
        thing.tex1 = GFX::GOSTEK_PARA_ROPE;
        thing.tex2 = GFX::GOSTEK_PARA;
#endif
        thing.timeout = 3600;
    }
    break;
    case object_combat_knife: // Knife
    {
        thing.skeleton.vdamping = 0.994;
        thing.skeleton.gravity = 1.15 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle18));

        a = thing.skeleton.pos[2];
        thing.skeleton.pos[2] = thing.skeleton.pos[1];
        thing.skeleton.oldpos[2] = thing.skeleton.pos[1];

        thing.skeleton.pos[1] = a;
        thing.skeleton.oldpos[1] = a;

        thing.skeleton.pos[1].x = thing.skeleton.pos[1].x + (float)(Random(100)) / 100;
        thing.skeleton.pos[2].x = thing.skeleton.pos[2].x - (float)(Random(100)) / 100;

#ifndef SERVER
        thing.tex1 = guns[knife].texturenum + k;
        thing.tex2 = 0;
#endif
        thing.radius = gun_radius * 1.5;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_chainsaw: // Chainsaw
    {
        thing.skeleton.vdamping = 0.994;
        thing.skeleton.gravity = 1.15 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle28));
#ifndef SERVER
        thing.tex1 = guns[chainsaw].texturenum + k;
        thing.tex2 = 0;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_law: // LAW
    {
        thing.skeleton.vdamping = 0.994;
        thing.skeleton.gravity = 1.15 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Rifle28));
#ifndef SERVER
        thing.tex1 = guns[law].texturenum + k;
#endif
        thing.radius = gun_radius;
        thing.timeout = gunresisttime;
        thing.interest = 0;
        thing.collidewithbullets = CVar::sv_guns_collide;
    }
    break;
    case object_stationary_gun: // stationary gun
    {
        thing.skeleton.vdamping = 0.99;
        thing.skeleton.gravity = 0.2 * grav;
        thing.skeleton.clone(AnimationSystem::Get().GetSkeleton(Stat));
#ifndef SERVER
        thing.tex1 = 0;
        thing.tex2 = 0;
#endif
        thing.timeout = 60;
        thing.radius = stat_radius;
        thing.statictype = false;
        thing.interest = 0;
        thing.collidewithbullets = false;
    }
    break;
    } // case

    thing.owner = owner;
    thing.moveskeleton(spos.x, spos.y, false);

#ifdef SERVER

    // Throw weapon
    if (((sstyle > object_pointmatch_flag) && (sstyle < object_medical_kit)) ||
        (sstyle == object_law) || (sstyle == object_chainsaw))
        if ((owner > 0) && (owner < max_sprites + 1))
        {
            auto &spriteVelocity = SpriteSystem::Get().GetVelocity(owner);
            // Add player velocity
            thing.skeleton.pos[1] = vec2add(thing.skeleton.pos[1], spriteVelocity);
            thing.skeleton.pos[2] = vec2add(thing.skeleton.pos[2], spriteVelocity);

            // Add throw velocity
            b = SpriteSystem::Get().GetSprite(owner).getcursoraimdirection();

            if (!SpriteSystem::Get().GetSprite(owner).deadmeat)
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
            thing.skeleton.pos[1] = vec2add(thing.skeleton.pos[1], weaponthrowvelocity);
            vec2scale(weaponthrowvelocity, b, weaponthrowspeedpos2);
            thing.skeleton.pos[2] = vec2add(thing.skeleton.pos[2], weaponthrowvelocity);
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

    auto &map = GS::GetGame().GetMap();
    auto things = GS::GetThingSystem().GetThings();

    for (i = 1; i <= 255; i++)
    {
        if (map.spawnpoints[i].active)
        {
            if (map.spawnpoints[i].team == team)
            {
                if (things[num].lastspawn != i)
                {
                    spawnscount += 1;
                    spawns[spawnscount] = i;
                }
                else
                    previousspawn = i;
            }
        }
    }

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
        things[num].lastspawn = spawns[i];
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

    auto &map = GS::GetGame().GetMap();

    for (i = 1; i <= max_spawnpoints; i++)
        spawns[i] = -1;

    spawnscount = 0;

    for (i = 1; i <= max_spawnpoints; i++)
    {
        if (map.spawnpoints[i].active)
        {
            if (map.spawnpoints[i].team == team)
            {
                spawnscount += 1;
                spawns[spawnscount] = i;
            }
        }
    }

    if (spawnscount == 0)
    {
        result = false;
        for (i = 1; i <= max_spawnpoints; i++)
        {
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
                skeleton.pos[1] = SpriteSystem::Get().GetSprite(holdingsprite).skeleton.pos[8];
                skeleton.forces[2].y = skeleton.forces[2].y + flag_holding_forceup * grav;
                interest = default_interest_time;

                interest = flag_interest_time;

                SpriteSystem::Get().GetSprite(holdingsprite).holdedthing = num;
                timeout = flag_timeout;

                if (bgstate.backgroundstatus != background_transition)
                {
                    bgstate.backgroundstatus =
                        SpriteSystem::Get().GetSprite(holdingsprite).bgstate.backgroundstatus;
                    bgstate.backgroundpoly =
                        SpriteSystem::Get().GetSprite(holdingsprite).bgstate.backgroundpoly;
                }
            } // HoldingSprite > 0
    }

#ifdef SERVER
    LogTraceG("TThing.Update 1");
#endif
    auto &map = GS::GetGame().GetMap();

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
                if (SpriteSystem::Get().GetSprite(holdingsprite).player->team == style)
                    respawn();
#endif
        }
        else
        {
            inbase = false;
        }

        GS::GetGame().SetTeamFlag(style, num);
    }
    break;
    case object_pointmatch_flag: {
#ifndef SERVER
        GS::GetGame().SetTeamFlag(1, num);
#endif
    }
    break;
    }

#ifdef SERVER
    LogTraceG("TThing.Update 2");
#endif
    auto things = GS::GetThingSystem().GetThings();

// check if flag is touchdown
#ifdef SERVER
    if ((style == object_alpha_flag) || (style == object_bravo_flag))
        if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
            if (SpriteSystem::Get().GetSprite(holdingsprite).player->team != style)
            { // check if other flag is inbase
                for (i = 1; i <= max_things; i++)
                    if ((things[i].active) && (things[i].inbase) && (i != num) &&
                        (things[i].holdingsprite == 0)) // check if flags are close
                        if (distance(skeleton.pos[1], things[i].skeleton.pos[1]) < touchdown_radius)
                        {
                            if (SpriteSystem::Get().GetSprite(holdingsprite).player->team ==
                                team_alpha)
                            {
#ifndef SERVER
                                if (sv_gamemode.intvalue == gamestyle_inf)
                                    playsound(SfxEffect::infiltmus);
                                else
                                    playsound(SfxEffect::ctf);
#endif

                                SpriteSystem::Get().GetSprite(holdingsprite).player->flags =
                                    SpriteSystem::Get().GetSprite(holdingsprite).player->flags + 1;
                                GS::GetGame().SetTeamScore(1, GS::GetGame().GetTeamScore(1) + 1);

                                b.x = 0;
                                b.y = 0;
                                if (CVar::sv_gamemode == gamestyle_inf)
                                {
                                    GS::GetGame().SetTeamScore(1, GS::GetGame().GetTeamScore(1) +
                                                                      CVar::sv_inf_redaward - 1);
                                    // penalty
                                    if (GS::GetGame().GetPlayersTeamNum(1) >
                                        GS::GetGame().GetPlayersTeamNum(2))
                                    {
                                        GS::GetGame().SetTeamScore(
                                            1, GS::GetGame().GetTeamScore(1) -
                                                   (5 * (GS::GetGame().GetPlayersTeamNum(1) -
                                                         GS::GetGame().GetPlayersTeamNum(2))));
                                    }
                                    if (GS::GetGame().GetTeamScore(1) < 0)
                                    {
                                        GS::GetGame().SetTeamScore(
                                            1, GS::GetGame().GetTeamScore(1) + 1);
                                    }

#ifndef SERVER
                                    // flame it
                                    for (j = 1; j <= 10; j++)
                                    {
                                        a.x = thing.skeleton.pos[2].x - 10 + Random(20);
                                        a.y = thing.skeleton.pos[2].y - 10 + Random(20);
                                        createspark(a, b, 36, 0, 35);
                                        if (Random(2) == 0)
                                            createspark(a, b, 37, 0, 75);
                                    }
#endif
                                }

#ifndef SERVER
                                // cap spark
                                createspark(thing.skeleton.pos[2], b, 61, holdingsprite, 18);
#endif

                                GS::GetGame().sortplayers();
#ifdef SERVER
                                GetMainConsole().console(
                                    SpriteSystem::Get().GetSprite(holdingsprite).player->name +
                                        ' ' + "scores for Alpha Team",
                                    alpha_message_color);
                                SpriteSystem::Get()
                                    .GetSprite(holdingsprite)
                                    .player->scorespersecond += 1;
#else
                                bigmessage(_("Alpha Team Scores!"), capturectfmessagewait,
                                           alpha_message_color);
                                GetMainConsole().console(
                                    wideformat(_("{} scores for Alpha Team"),
                                               set::of(SpriteSystem::Get()
                                                           .GetSprite(holdingsprite)
                                                           .player->name,
                                                       eos)),
                                    alpha_message_color);
#endif

                                serverflaginfo(capturered, holdingsprite);
                            }
                            if (SpriteSystem::Get().GetSprite(holdingsprite).player->team ==
                                team_bravo)
                            {
#ifndef SERVER
                                playsound(SfxEffect::ctf);
#endif
                                SpriteSystem::Get().GetSprite(holdingsprite).player->flags =
                                    SpriteSystem::Get().GetSprite(holdingsprite).player->flags + 1;
                                GS::GetGame().SetTeamScore(2, GS::GetGame().GetTeamScore(2) + 1);

#ifndef SERVER
                                // cap spark
                                b.x = 0;
                                b.y = 0;
                                createspark(thing.skeleton.pos[2], b, 61, holdingsprite, 15);
#endif

                                GS::GetGame().sortplayers();
#ifdef SERVER
                                GetMainConsole().console(
                                    SpriteSystem::Get().GetSprite(holdingsprite).player->name +
                                        ' ' + "scores for Bravo Team",
                                    bravo_message_color);
                                SpriteSystem::Get()
                                    .GetSprite(holdingsprite)
                                    .player->scorespersecond += 1;
#else
                                bigmessage(_("Bravo Team Scores!"), capturectfmessagewait,
                                           bravo_message_color);
                                GetMainConsole().console(
                                    wideformat(_("{} scores for Bravo Team"),
                                               set::of(SpriteSystem::Get()
                                                           .GetSprite(holdingsprite)
                                                           .player->name,
                                                       eos)),
                                    bravo_message_color);
#endif
                                serverflaginfo(captureblue, holdingsprite);
                            }
#ifdef SCRIPT
                            scrptdispatcher.onflagscore(holdingsprite, style);
#endif

                            if (CVar::bots_chat)
                            {
                                if (SpriteSystem::Get()
                                        .GetSprite(holdingsprite)
                                        .player->controlmethod == bot)
                                    if (Random(SpriteSystem::Get()
                                                   .GetSprite(holdingsprite)
                                                   .brain.chatfreq /
                                               3) == 0)
                                        serversendstringmessage((SpriteSystem::Get()
                                                                     .GetSprite(holdingsprite)
                                                                     .brain.chatwinning),
                                                                all_players, holdingsprite,
                                                                msgtype_pub);
                            }

                            respawn();

                            if (CVar::sv_survivalmode)
                            {
                                GS::GetGame().SetSurvivalendround(true);

                                // Everyone should die in floatistic after cap
                                // because if nobody is dead the round will not end
                                for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
                                {
#ifdef SERVER
                                    constexpr auto hit = 4000;
#else
                                    constexpr auto hit = 150;
#endif
                                    if (!sprite.deadmeat)
                                    {
                                        sprite.healthhit(hit, sprite.num, 1, -1,
                                                         sprite.skeleton.pos[12]);
                                        sprite.player->deaths -= 1;
                                    }
                                }
                            }
                        }
            }
#endif
    LogTraceG("TThing.Update 3");

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
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {
            if ((sprite.weapon.num == bow_num) || (sprite.weapon.num == bow2_num))
            {
#ifndef SERVER
                gamethingtarget = 0;
#endif
                kill();
            }
        }
    }

#ifndef SERVER
    // flag on wind sound - and para
    if ((style < object_ussocom) || (style == object_parachute))
        if (Random(75) == 0)
            if (vec2length(vec2subtract(skeleton.pos[2], skeleton.oldpos[2])) > 1.0)
                playsound(SfxEffect::flag + Random(2), skeleton.pos[2]);
#endif

    // Parachute
    if (style == object_parachute)
    {
        if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
        {
            auto &spriteVelocity = SpriteSystem::Get().GetVelocity(holdingsprite);
            skeleton.pos[4] = SpriteSystem::Get().GetSprite(holdingsprite).skeleton.pos[12];
            skeleton.forces[1].y = -spriteVelocity.y;
            SpriteSystem::Get().GetSprite(holdingsprite).holdedthing = num;

            if (skeleton.pos[3].x < skeleton.pos[4].x)
            {
                a = skeleton.pos[4];
                skeleton.pos[4] = skeleton.pos[3];
                skeleton.oldpos[4] = skeleton.pos[3];
                skeleton.pos[3] = a;
                skeleton.oldpos[3] = a;
                auto &spriteForces = SpriteSystem::Get().GetForces(holdingsprite);
                spriteForces.y = grav;
            }
        }
        else
        {
#ifndef SERVER
            if (timeout > 180)
                timeout = 180;
#endif
        }
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
            if (SpriteSystem::Get().GetSprite(owner).active)
                if (SpriteSystem::Get().GetSprite(owner).visible == 0)
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
            if (SpriteSystem::Get().GetSprite(holdingsprite).visible == 0)
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
            if (SpriteSystem::Get().GetSprite(holdingsprite).visible == 0)
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
    std::int32_t b = 0;
    tvector2 pos, perp, posdiff, posdiffperp;
    float d = 0.0;
    float posdifflen;
    bool teamcol;

    bool result;

#ifdef SERVER
    LogTraceG("TThing.CheckMapCollision");
#endif

    result = false;
    pos.x = x;
    pos.y = y - 0.5;

    auto &map = GS::GetGame().GetMap();

    // iterate through map polygons
    const auto sector = map.GetSector(pos);
    if (sector.IsValid())
    {
        bgstate.backgroundtestbigpolycenter(pos);

        for (const auto &w : sector.GetPolys())
        {

            teamcol = true;

            const auto &polytype = w.Type;

            if ((owner > 0) && (owner < max_sprites + 1))
            {
                teamcol = teamcollides(polytype, SpriteSystem::Get().GetSprite(owner).player->team,
                                       false);
            }

            if ((style < object_ussocom) && (polytype > poly_type_lava) &&
                (polytype < poly_type_bouncy))
                teamcol = false;

            if (teamcol && (polytype != poly_type_only_bullets) &&
                (polytype != poly_type_only_player) && (polytype != poly_type_doesnt) &&
                (polytype != poly_type_only_flaggers) && (polytype != poly_type_not_flaggers))
            {
                if (map.pointinpolyedges(pos.x, pos.y, w.Index))
                {
                    if (bgstate.backgroundtest(w))
                        continue;

                    perp = map.closestperpendicular(w.Index, pos, d, b);

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
                            playsound(SfxEffect::weaponhit, skeleton.pos[i]);
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
                            playsound(SfxEffect::kit_fall + Random(2), skeleton.pos[i]);
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
                            playsound(SfxEffect::flag + Random(2), skeleton.pos[i]);
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
    auto things = GS::GetThingSystem().GetThings();

    if (num <= 0) // skip uninited Things
        return;
    things[num].skeleton.destroy();
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
    auto &map = GS::GetGame().GetMap();

    bound = map.sectorsnum * map.GetSectorsDivision() - 10;

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
                    SpriteSystem::Get().GetSprite(holdingsprite).holdedthing = 0;
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
    auto things = GS::GetThingSystem().GetThings();

#ifdef SERVER
    LogTraceG("TThing.Respawn");
#endif

    if ((holdingsprite > 0) && (holdingsprite < max_sprites + 1))
    {
        SpriteSystem::Get().GetSprite(holdingsprite).holdedthing = 0;
        if (SpriteSystem::Get().GetSprite(holdingsprite).player->team == team_alpha)
            SpriteSystem::Get().GetSprite(holdingsprite).brain.pathnum = 1;
        if (SpriteSystem::Get().GetSprite(holdingsprite).player->team == team_bravo)
            SpriteSystem::Get().GetSprite(holdingsprite).brain.pathnum = 2;
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
    things[num].timeout = flag_timeout;
    things[num].interest = default_interest_time;
    things[num].statictype = false;

    for (i = 1; i <= 4; i++)
        things[num].collidecount[i] = 0;

    if (style == object_rambo_bow)
        things[num].interest = bow_interest_time;
    if (style < object_ussocom)
        things[num].interest = flag_interest_time;

        // send net info
#ifdef SERVER
    serverthingmustsnapshot(num);
#endif
}

template <Config::Module M>
void Thing<M>::moveskeleton(float x1, float y1, bool fromzero)
{
    std::int32_t i;

    LogTraceG("TThing.MoveSkeleton");

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
    auto &guns = GS::GetWeaponSystem().GetGuns();
    auto things = GS::GetThingSystem().GetThings();

    result = -1;

    a = vec2subtract(skeleton.pos[1], skeleton.pos[2]);
    k = (float)(vec2length(a)) / 2;
    vec2normalize(a, a);
    vec2scale(a, a, -k);
    pos = vec2add(skeleton.pos[1], a);

    // iterate through sprites
    closestdist = 9999999;
    closestplayer = -1;
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
        if (!sprite.deadmeat && sprite.isnotspectator())
        {
            auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(sprite.num);
            colpos = spritePartsPos;
            norm = vec2subtract(pos, colpos);
            if (vec2length(norm) >= radius)
            {
                pos = skeleton.pos[1];
                colpos = spritePartsPos;
                norm = vec2subtract(pos, colpos);

                if (vec2length(norm) >= radius)
                {
                    pos = skeleton.pos[2];
                    colpos = spritePartsPos;
                    norm = vec2subtract(pos, colpos);
                }
            }

            dist = vec2length(norm);
            if (dist < radius)
                if (dist < closestdist)
#ifdef SERVER
                    if (!((style == object_medical_kit) &&
                          (sprite.GetHealth() == GS::GetGame().GetStarthealth())))
                        if (!((style == object_grenade_kit) &&
                              (sprite.tertiaryweapon.ammocount == CVar::sv_maxgrenades) &&
                              (sprite.tertiaryweapon.num == fraggrenade_num)))
#endif
                            if (!((style < object_ussocom) && (sprite.ceasefirecounter > 0)))
                            {
                                closestdist = dist;
                                closestplayer = sprite.num;
                            }
        } // for j
    }

    j = closestplayer;

    if (j > 0) // collision
    {
        if (((((style > object_pointmatch_flag) && (style < object_rambo_bow) &&
               (SpriteSystem::Get().GetSprite(j).bodyanimation.id != AnimationType::Change)) ||
              ((style > object_parachute) &&
               (SpriteSystem::Get().GetSprite(j).bodyanimation.id != AnimationType::Change))) &&
             (SpriteSystem::Get().GetSprite(j).weapon.num == noweapon_num) &&
             (SpriteSystem::Get().GetSprite(j).brain.favweapon != noweapon_num) &&
             (timeout < gunresisttime - 30)) ||
            ((style == 15) && (SpriteSystem::Get().GetSprite(j).weapon.num == noweapon_num) &&
             (timeout < (CVar::sv_respawntime * flag_timeout - 100))) ||
            (((style == object_medical_kit) &&
              (SpriteSystem::Get().GetSprite(j).GetHealth() < GS::GetGame().GetStarthealth())
#ifdef SERVER
              && (SpriteSystem::Get().GetSprite(j).haspack == false)
#endif
                  ) ||
             ((style == object_grenade_kit) &&
              (SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount < CVar::sv_maxgrenades) &&
              ((SpriteSystem::Get().GetSprite(j).tertiaryweapon.num != clustergrenade_num) ||
               (SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount == 0)))) ||
            ((((style == object_flamer_kit) &&
               (SpriteSystem::Get().GetSprite(j).weapon.num != bow_num) &&
               (SpriteSystem::Get().GetSprite(j).weapon.num != bow2_num)) ||
              (style == object_predator_kit) || (style == object_berserk_kit)) &&
             (SpriteSystem::Get().GetSprite(j).bonusstyle == bonus_none) &&
             (SpriteSystem::Get().GetSprite(j).ceasefirecounter < 1)) ||
            ((style == object_vest_kit)
#ifdef SERVER
             && (SpriteSystem::Get().GetSprite(j).vest < defaultvest)
#endif
                 ) ||
            ((style == object_cluster_kit)
#ifdef SERVER
             && ((SpriteSystem::Get().GetSprite(j).tertiaryweapon.num == fraggrenade_num) ||
                 (SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount == 0))
#endif
                 ))
        {
#ifndef SERVER
            auto &spritePartsPos =
                SpriteSystem::Get().GetSpritePartsPos(SpriteSystem::Get().GetSprite(j).num);
            if (((style > object_pointmatch_flag) && (style < object_rambo_bow)) ||
                (style > object_parachute)) // take sound
                playsound(SfxEffect::takegun, spritePartsPos);
            else if (style == object_rambo_bow) // rambo sound
                playsound(SfxEffect::takebow, spritePartsPos);
            else if (style == object_medical_kit) // take medikit sound
                playsound(SfxEffect::takemedikit, spritePartsPos);
            else if (style == object_grenade_kit) // take grenade kit sound
                playsound(SfxEffect::pickupgun, spritePartsPos);
            else if (style == object_flamer_kit) // take flamer kit sound
                playsound(SfxEffect::godflame, spritePartsPos);
            else if (style == object_predator_kit) // take predator kit sound
                playsound(SfxEffect::predator, spritePartsPos);
            else if (style == object_vest_kit) // take vest kit sound
                playsound(SfxEffect::vesttake, spritePartsPos);
            else if (style == object_berserk_kit) // take berserker kit sound
                playsound(SfxEffect::berserker, spritePartsPos);
            else if (style == object_cluster_kit) // take cluster kit sound
                playsound(SfxEffect::pickupgun, spritePartsPos);
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
            if (GS::GetGame().GetSurvivalEndRound() &&
                ((style == object_alpha_flag) || (style == object_bravo_flag) ||
                 (style == object_pointmatch_flag)))
                return result;

            statictype = false;
            timeout = flag_timeout;
            interest = flag_interest_time;

            if ((SpriteSystem::Get().GetSprite(j).player->team != style) || !inbase)
            {
                if ((holdingsprite == 0) && (SpriteSystem::Get().GetSprite(j).flaggrabcooldown < 1))
                {
#ifndef SERVER
                    // capture sound
                    playsound(SfxEffect::capture, skeleton.pos[1]);
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
                        switch (SpriteSystem::Get().GetSprite(j).player->team)
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
                        smallcaptextstr =
                            SpriteSystem::Get().GetSprite(j).player->name + " got the Yellow Flag";
                        if (CVar::sv_gamemode == gamestyle_htf)
                        {
                            SpriteSystem::Get().GetSprite(j).player->grabbedinbase =
                                things[2].inbase;
                            SpriteSystem::Get().GetSprite(j).player->grabspersecond += 1;
                        }
#else
                        bigcaptext =
                            iif(j == mysprite, _("You got the Flag!"), _("Yellow Flag captured!"));
                        smallcaptext = _("{} got the Yellow Flag");
#endif
#ifdef SCRIPT
                        scrptdispatcher.onflaggrab(
                            j, style, SpriteSystem::Get().GetSprite(j).player->grabbedinbase);
#endif
                    }
                    break;
                    case gamestyle_ctf:
                        if (SpriteSystem::Get().GetSprite(j).player->team == style)
                        {
                            switch (SpriteSystem::Get().GetSprite(j).player->team)
                            {
                            case team_alpha: {
#ifdef SERVER
                                smallcaptextstr = SpriteSystem::Get().GetSprite(j).player->name +
                                                  " returned the Red Flag";
#else
                                bigcaptext = _("Red flag returned!");
                                smallcaptext = _("%s returned the Red Flag");
#endif
                            }
                            break;
                            case team_bravo: {
#ifdef SERVER
                                smallcaptextstr = SpriteSystem::Get().GetSprite(j).player->name +
                                                  " returned the Blue Flag";
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
                            switch (SpriteSystem::Get().GetSprite(j).player->team)
                            {
                            case team_alpha: {
#ifdef SERVER
                                smallcaptextstr = SpriteSystem::Get().GetSprite(j).player->name +
                                                  " captured the Blue Flag";
                                SpriteSystem::Get().GetSprite(j).player->grabbedinbase =
                                    things[2].inbase;
                                SpriteSystem::Get().GetSprite(j).player->grabspersecond += 1;
#else
                                bigcaptext = iif(j == mysprite, _("You got the Blue Flag!"),
                                                 _("Blue Flag captured!"));
                                smallcaptext = _("%s captured the Blue Flag");
#endif
                            }
                            break;
                            case team_bravo: {
#ifdef SERVER
                                smallcaptextstr = SpriteSystem::Get().GetSprite(j).player->name +
                                                  " captured the Red Flag";
                                SpriteSystem::Get().GetSprite(j).player->grabbedinbase =
                                    things[1].inbase;
                                SpriteSystem::Get().GetSprite(j).player->grabspersecond += 1;
#else
                                bigcaptext = iif(j == mysprite, _("You got the Red Flag!"),
                                                 _("Red Flag captured!"));
                                smallcaptext = _("%s captured the Red Flag");
#endif
                            }
                            break;
                            }
#ifdef SCRIPT
                            scrptdispatcher.onflaggrab(
                                j, style, SpriteSystem::Get().GetSprite(j).player->grabbedinbase);
#endif
                        }
                        break;
                    case gamestyle_inf:
                        if (SpriteSystem::Get().GetSprite(j).player->team == style)
                        {
                            if (SpriteSystem::Get().GetSprite(j).player->team == team_bravo)
                            {
#ifdef SERVER
                                smallcaptextstr = SpriteSystem::Get().GetSprite(j).player->name +
                                                  " returned the Objective";
                                SpriteSystem::Get().GetSprite(j).player->grabbedinbase =
                                    things[2].inbase;
                                SpriteSystem::Get().GetSprite(j).player->grabspersecond += 1;
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
                            if (SpriteSystem::Get().GetSprite(j).player->team == team_alpha)
                            {
#ifdef SERVER
                                smallcaptextstr = SpriteSystem::Get().GetSprite(j).player->name +
                                                  " captured the Objective";
                                SpriteSystem::Get().GetSprite(j).player->grabbedinbase =
                                    things[2].inbase;
                                SpriteSystem::Get().GetSprite(j).player->grabspersecond += 1;
#else
                                bigcaptext = iif(j == mysprite, _("You got the Objective!"),
                                                 _("Objective captured!"));
                                smallcaptext = _("%s captured the Objective");
#endif
                            }
#ifdef SCRIPT
                            scrptdispatcher.onflaggrab(
                                j, style, SpriteSystem::Get().GetSprite(j).player->grabbedinbase);
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
                            wideformat(smallcaptext,
                                       set::of(SpriteSystem::Get().GetSprite(j).player->name, eos)),
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
            if (SpriteSystem::Get().GetSprite(j).weapon.num == noweapon_num)
                if (SpriteSystem::Get().GetSprite(j).brain.favweapon != noweapon_num)
                    if (SpriteSystem::Get().GetSprite(j).bodyanimation.id != AnimationType::Change)
                        if (timeout < gunresisttime - 30)
                        {
                            // Objects 1-3 are flags, so we need for WeaponIndex subtract by flags+1
                            weaponindex = weaponnumtoindex(style - (object_num_flags + 1), guns);
#ifdef SCRIPT
                            // event must be before actual weapon apply.
                            // script might've called ForceWeapon, which we should check.
                            // if it did, we don't apply snapshot weapon's as they were already
                            // applied by force weapon.
                            forceweaponcalled = false;
                            scrptdispatcher.onweaponchange(
                                j, guns[weaponindex].num,
                                SpriteSystem::Get().GetSprite(j).secondaryweapon.num, ammocount,
                                SpriteSystem::Get().GetSprite(j).secondaryweapon.ammocount);

                            if (!forceweaponcalled)
                            {
                                ;
#endif
                                SpriteSystem::Get().GetSprite(j).applyweaponbynum(
                                    guns[weaponindex].num, 1);
                                SpriteSystem::Get().GetSprite(j).weapon.ammocount = ammocount;
                                SpriteSystem::Get().GetSprite(j).weapon.fireintervalprev =
                                    SpriteSystem::Get().GetSprite(j).weapon.fireinterval;
                                SpriteSystem::Get().GetSprite(j).weapon.fireintervalcount =
                                    SpriteSystem::Get().GetSprite(j).weapon.fireinterval;
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
            if (SpriteSystem::Get().GetSprite(j).weapon.num == noweapon_num)
                if (SpriteSystem::Get().GetSprite(j).bodyanimation.id != AnimationType::Change)
                    if (timeout < flag_timeout - 100)
                    {
#ifdef SCRIPT
                        // event must be before actual weapon apply.
                        // script might've called ForceWeapon, which we should check.
                        // if it did, we don't apply snapshot weapon's as they were already applied
                        // by force weapon.
                        forceweaponcalled = false;
                        scrptdispatcher.onweaponchange(j, bow_num, bow2_num, 1, 1);

                        if (!forceweaponcalled)
                        {
#endif
                            SpriteSystem::Get().GetSprite(j).applyweaponbynum(bow_num, 1);
                            SpriteSystem::Get().GetSprite(j).applyweaponbynum(bow2_num, 2);
                            // BUG: shouldn't this be Guns[BOW].Ammo? Somebody might've set more
                            // than one
                            SpriteSystem::Get().GetSprite(j).weapon.ammocount = 1;
                            SpriteSystem::Get().GetSprite(j).weapon.fireintervalprev =
                                SpriteSystem::Get().GetSprite(j).weapon.fireinterval;
                            SpriteSystem::Get().GetSprite(j).weapon.fireintervalcount =
                                SpriteSystem::Get().GetSprite(j).weapon.fireinterval;
                            SpriteSystem::Get().GetSprite(j).wearhelmet = 1;
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
                            bigmessage(
                                wideformat(
                                    _("%s got the Bow!"),
                                    set::of(SpriteSystem::Get().GetSprite(j).player->name, eos)),
                                capturemessagewait, capture_message_color);
#endif
                    }
            break;
        case object_medical_kit: {
            if (SpriteSystem::Get().GetSprite(j).GetHealth() < GS::GetGame().GetStarthealth())
            {
                // pickup health pack
#ifdef SERVER
                if (SpriteSystem::Get().GetSprite(j).haspack == false)
#endif
                {
                    team = SpriteSystem::Get().GetSprite(j).player->team;
#ifdef SERVER
                    if (CVar::sv_healthcooldown > 0)
                        SpriteSystem::Get().GetSprite(j).haspack = true;
#endif
                    SpriteSystem::Get().GetSprite(j).SetHealth(GS::GetGame().GetStarthealth());
                    respawn();

#ifdef SCRIPT
                    scrptdispatcher.onkitpickup(j, num);
#endif
                }
            }
        }
        break;
        case object_grenade_kit: {
            if ((SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount <
                 CVar::sv_maxgrenades) &&
                ((SpriteSystem::Get().GetSprite(j).tertiaryweapon.num != clustergrenade_num) ||
                 (SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount == 0)))
            {
                team = SpriteSystem::Get().GetSprite(j).player->team;
                SpriteSystem::Get().GetSprite(j).SetThirdWeapon(guns[fraggrenade]);
                SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount = CVar::sv_maxgrenades;
                respawn();

#ifdef SCRIPT
                scrptdispatcher.onkitpickup(j, num);
#endif
            }
        }
        break;
        case object_flamer_kit:
            if ((SpriteSystem::Get().GetSprite(j).bonusstyle == bonus_none) &&
                (SpriteSystem::Get().GetSprite(j).ceasefirecounter < 1))
            {
                if ((SpriteSystem::Get().GetSprite(j).weapon.num != bow_num) &&
                    (SpriteSystem::Get().GetSprite(j).weapon.num != bow2_num))
                {
#ifdef SCRIPT
                    // event must be before actual weapon apply.
                    // script might've called ForceWeapon, which we should check.
                    // if it did, we don't apply snapshot weapon's as they were already applied
                    // by force weapon.
                    forceweaponcalled = false;
                    scrptdispatcher.onweaponchange(
                        j, flamer_num, SpriteSystem::Get().GetSprite(j).secondaryweapon.num,
                        ammocount, SpriteSystem::Get().GetSprite(j).secondaryweapon.ammocount);
                    scrptdispatcher.onkitpickup(j, num);

                    if (!forceweaponcalled)
                    {
                        ;
#endif
                        SpriteSystem::Get().GetSprite(j).applyweaponbynum(
                            SpriteSystem::Get().GetSprite(j).weapon.num, 2
#ifndef SERVER
                            ,
                            -1, true
#endif
                        );
                        SpriteSystem::Get().GetSprite(j).applyweaponbynum(flamer_num, 1);
                        SpriteSystem::Get().GetSprite(j).bonustime = flamerbonustime;
                        SpriteSystem::Get().GetSprite(j).bonusstyle = bonus_flamegod;

#ifndef SERVER
                        if (j == mysprite)
                        {
                            bigmessage(_("Flame God Mode!"), capturemessagewait,
                                       bonus_message_color);
                            clientspritesnapshot();
                        }
#endif

                        SpriteSystem::Get().GetSprite(j).SetHealth(GS::GetGame().GetStarthealth());
#ifdef SCRIPT
                    }
#endif
                }
            }
            break;
        case object_predator_kit:
            if ((SpriteSystem::Get().GetSprite(j).bonusstyle == bonus_none) &&
                (SpriteSystem::Get().GetSprite(j).ceasefirecounter < 1))
            {
                SpriteSystem::Get().GetSprite(j).alpha = predatoralpha;
                SpriteSystem::Get().GetSprite(j).bonustime = predatorbonustime;
                SpriteSystem::Get().GetSprite(j).bonusstyle = bonus_predator;

#ifndef SERVER
                if (j == mysprite)
                    bigmessage(_("Predator Mode!"), capturemessagewait, bonus_message_color);
#endif

                SpriteSystem::Get().GetSprite(j).SetHealth(GS::GetGame().GetStarthealth());

#ifdef SCRIPT
                scrptdispatcher.onkitpickup(j, num);
#endif
            }
            break;
        case object_vest_kit: {
            SpriteSystem::Get().GetSprite(j).vest = defaultvest;

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
            if ((SpriteSystem::Get().GetSprite(j).bonusstyle == 0) &&
                (SpriteSystem::Get().GetSprite(j).ceasefirecounter < 1))
            {
                SpriteSystem::Get().GetSprite(j).bonusstyle = bonus_berserker;
                SpriteSystem::Get().GetSprite(j).bonustime = berserkerbonustime;

#ifndef SERVER
                if (j == mysprite)
                    bigmessage(_("Berserker Mode!"), capturemessagewait, bonus_message_color);
#endif

                SpriteSystem::Get().GetSprite(j).SetHealth(GS::GetGame().GetStarthealth());

#ifdef SCRIPT
                scrptdispatcher.onkitpickup(j, num);
#endif
            }
            break;
        case object_cluster_kit:
#ifdef SERVER
            if ((SpriteSystem::Get().GetSprite(j).tertiaryweapon.num == fraggrenade_num) ||
                (SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount == 0))
#endif
            {
                SpriteSystem::Get().GetSprite(j).SetThirdWeapon(guns[clustergrenade]);
                SpriteSystem::Get().GetSprite(j).tertiaryweapon.ammocount = cluster_grenades;

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
            if (SpriteSystem::Get().GetSprite(j).weapon.num == noweapon_num)
                if (SpriteSystem::Get().GetSprite(j).brain.favweapon != noweapon_num)
                    if (SpriteSystem::Get().GetSprite(j).bodyanimation.id != AnimationType::Change)
                        if (timeout < gunresisttime - 30)
                        {
                            // There are in total OBJECT_NUM_NONWEAPON non-weapon objects before the
                            // knife so we need to subtract it+1 for the WeaponIndex (like before)
                            weaponindex =
                                weaponnumtoindex(style - (object_num_nonweapon + 1), guns);
#ifdef SCRIPT
                            // event must be before actual weapon apply.
                            // script might've called ForceWeapon, which we should check.
                            // if it did, we don't apply snapshot weapon's as they were already
                            // applied by force weapon.
                            forceweaponcalled = false;
                            scrptdispatcher.onweaponchange(
                                j, guns[weaponindex].num,
                                SpriteSystem::Get().GetSprite(j).secondaryweapon.num, ammocount,
                                SpriteSystem::Get().GetSprite(j).secondaryweapon.ammocount);

                            if (!forceweaponcalled)
                            {
#endif
                                SpriteSystem::Get().GetSprite(j).applyweaponbynum(
                                    guns[weaponindex].num, 1);
                                SpriteSystem::Get().GetSprite(j).weapon.ammocount = ammocount;
                                SpriteSystem::Get().GetSprite(j).weapon.fireintervalprev =
                                    SpriteSystem::Get().GetSprite(j).weapon.fireinterval;
                                SpriteSystem::Get().GetSprite(j).weapon.fireintervalcount =
                                    SpriteSystem::Get().GetSprite(j).weapon.fireinterval;
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
    std::int32_t k;
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
        if (GS::GetGame().GetMainTickCounter() % 8 == 0)
            interest -= 1;

    pos = skeleton.pos[1];

    auto &guns = GS::GetWeaponSystem().GetGuns();

    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
        if (!sprite.deadmeat && sprite.isnotspectator())
        {
            if (sprite.stat == num)
            {
                auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(sprite.num);
                colpos = spritePartsPos;
                norm = vec2subtract(pos, colpos);
                if (vec2length(norm) < radius) // collision
                {
                    if (sprite.player->controlmethod == bot)
                        if (sprite.brain.camper > 0)
                            if (sprite.holdedthing == 0)
                            {
                                sprite.control.right = false;
                                sprite.control.left = false;
                                sprite.control.up = false;
                                sprite.control.down = false;
                                if (sprite.legsanimation.id == AnimationType::Prone)
                                    sprite.control.prone = true;
                            }

                    statictype = true;

                    pos.x = sprite.control.mouseaimx;
                    pos.y = sprite.control.mouseaimy;
                    norm = vec2subtract(pos, sprite.skeleton.pos[15]);
                    vec2normalize(norm, norm);
                    vec2scale(norm, norm, 3);
                    norm.x = -norm.x;
                    skeleton.oldpos[4] = skeleton.pos[4];
                    skeleton.pos[4] = vec2add(skeleton.pos[1], norm);

                    interest = sprite.usetime;

                    if (sprite.control.fire)
                        if (sprite.legsanimation.id == AnimationType::Stand)
                            if (GS::GetGame().GetMainTickCounter() % guns[m2].fireinterval == 0)
                            {
                                if (sprite.usetime > m2gun_overheat)
                                {
#ifndef SERVER
                                    playsound(SfxEffect::m2overheat, spritePartsPos);
#endif
                                    return result;
                                }

                                k = 0;
                                if (sprite.usetime > m2gun_overaim)
                                {
                                    k = (sprite.usetime / 11);
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

                                createbullet(pos, norm, guns[m2].num, sprite.num, 255,
                                             guns[m2].hitmultiply, true, false);

#ifndef SERVER
                                pos = vec2add(pos, norm);
                                vec2scale(norm, norm, 0.1);
                                createspark(pos, norm, 35, sprite.num, 15); // smoke

                                colpos = norm;
                                norm.y = -sprite.direction * norm.x;
                                norm.x = sprite.direction * colpos.y;
                                vec2scale(norm, norm, 0.2);
                                pos.x = skeleton.pos[3].x + 18;
                                pos.y = skeleton.pos[3].y - 20;
                                createspark(pos, norm, 22, sprite.num, 255); // hull

                                playsound(SfxEffect::m2fire, spritePartsPos);
#endif

                                sprite.usetime += 1;
                            }
                    return result;
                }
                else
                {
                    sprite.stat = 0;
                    statictype = false;
                    return result;
                }
            }
        }
    }

    // iterate through sprites
#ifndef SERVER
    if (clientcheck)
#endif
        if (!statictype)
        {
            for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
            {
                if (!sprite.deadmeat && sprite.isnotspectator())
                {
                    auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(sprite.num);
                    colpos = spritePartsPos;
                    norm = vec2subtract(pos, colpos);
                    if (vec2length(norm) < radius) // collision
                    {
                        switch (style)
                        {
                        case object_stationary_gun: {
                            if (sprite.player->controlmethod == bot)
                                if (sprite.brain.camper > 0)
                                {
                                    sprite.control.right = false;
                                    sprite.control.left = false;
                                    sprite.control.up = false;
                                    sprite.control.down = false;
                                    sprite.legsapplyanimation(AnimationType::Stand, 1);
                                }

                            if (sprite.legsanimation.id == AnimationType::Stand)
                            {
#ifndef SERVER
                                playsound(SfxEffect::m2use, spritePartsPos);
#endif

                                statictype = true;
                                sprite.stat = num;
                                sprite.brain.oneplacecount = 0;
#ifdef SERVER
                                serverthingtaken(num, sprite.num);
#endif
                            }
                        }
                        break;
                        }

                        result = sprite.num;
                        return result;
                    }
                    else if (sprite.stat == num)
                        sprite.stat = 0;

                    statictype = false;
                } // for j
            }
        }
    return result;
}

template class Thing<>;
template std::int32_t creatething(tvector2 spos, std::uint8_t owner, std::uint8_t sstyle,
                                  std::uint8_t n);
template tthingcollision thingcollision(std::uint8_t thingnum, std::uint32_t cooldownend);
template bool spawnboxes(tvector2 &start, std::uint8_t team, std::uint8_t num);
template bool randomizestart(tvector2 &start, std::uint8_t team);
