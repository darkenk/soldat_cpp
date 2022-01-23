// automatically converted

#include "AI.hpp"
#include "../../server/Server.hpp"
#include "Game.hpp"
#include "common/Calc.hpp"
#include "common/Vector.hpp"
#include "common/Waypoints.hpp"
#include "common/Weapons.hpp"
#include "mechanics/Bullets.hpp"
#include "mechanics/Sprites.hpp"
#include "network/NetworkServer.hpp"
#include "network/NetworkServerMessages.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <Tracy.hpp>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

// Checks the distance on one axis
std::int32_t checkdistance(float posa, float posb)
{
    float distance;

    std::int32_t result;
    result = dist_away;

    distance = fabs(posa - posb);

    if (distance <= dist_too_close)
        result = dist_too_close;

    else if (distance <= dist_very_close)
        result = dist_very_close;

    else if (distance <= dist_close)
        result = dist_close;

    else if (distance <= dist_rock_throw)
        result = dist_rock_throw;

    else if (distance <= dist_far)
        result = dist_far;

    else if (distance <= dist_very_far)
        result = dist_very_far;

    else if (distance <= dist_too_far)
        result = dist_too_far;
    return result;
}

void simpledecision(std::uint8_t snum, const twaypoints &botpath)
{
    tvector2 tv;
    std::int32_t disttotargetx, disttotargety, dist;
    std::int32_t gr, i;

    {
        auto &with = SpriteSystem::Get().GetSprite(snum);
        auto &targetVelocity = SpriteSystem::Get().GetVelocity(with.brain.targetnum);

        const auto &m = SpriteSystem::Get().GetSpritePartsPos(snum);
        const auto &t = SpriteSystem::Get().GetSpritePartsPos(with.brain.targetnum);

        if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
        {
            with.control.right = false;
            with.control.left = false;
            if (t.x > m.x)
                with.control.right = true;
            if (t.x < m.x)
                with.control.left = true;
        }

        // X - Distance
        disttotargetx = checkdistance(m.x, t.x);

        if (disttotargetx == dist_too_close)
        {
            if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
            {
                with.control.right = false;
                with.control.left = false;
                if (t.x < m.x)
                    with.control.right = true;
                if (t.x > m.x)
                    with.control.left = true;
            }
            with.control.fire = true;
        }

        else if (disttotargetx == dist_very_close)
        {
            if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
            {
                with.control.right = false;
                with.control.left = false;
            }
            with.control.fire = true;

            // if reloading
            if (with.weapon.ammocount == 0)
            {
                if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
                {
                    with.control.right = false;
                    with.control.left = false;
                    if (t.x < m.x)
                        with.control.right = true;
                    if (t.x > m.x)
                        with.control.left = true;
                }
                with.control.fire = false;
            }
        }

        else if (disttotargetx == dist_close)
        {
            if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
            {
                with.control.right = false;
                with.control.left = false;
            }
            with.control.down = true;
            with.control.fire = true;

            // if reloading
            if (with.weapon.ammocount == 0)
            {
                if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
                {
                    with.control.right = false;
                    with.control.left = false;
                    if (t.x < m.x)
                        with.control.right = true;
                    if (t.x > m.x)
                        with.control.left = true;
                }
                with.control.down = false;
                with.control.fire = false;
            }
        }

        else if (disttotargetx == dist_rock_throw)
        {
            with.control.down = true;
            with.control.fire = true;

            // if reloading
            if (with.weapon.ammocount == 0)
            {
                if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
                {
                    with.control.right = false;
                    with.control.left = false;
                    if (t.x < m.x)
                        with.control.right = true;
                    if (t.x > m.x)
                        with.control.left = true;
                }
                with.control.down = false;
                with.control.fire = false;
            }
        }

        else if (disttotargetx == dist_far)
        {
            with.control.fire = true;

            if (with.brain.camper > 127)
            {
                if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
                {
                    with.control.up = false;
                    with.control.down = true;
                }
            }
        }

        else if (disttotargetx == dist_very_far)
        {
            with.control.up = true;
            if ((Random(2) == 0) || (with.weapon.num == minigun_num))
                with.control.fire = true;

            if (with.brain.camper > 0)
            {
                if (Random(250) == 0)
                    if (with.bodyanimation.id != AnimationType::Prone)
                        with.control.prone = true;

                if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
                {
                    with.control.right = false;
                    with.control.left = false;
                    with.control.up = false;
                    with.control.down = true;
                }
            }
        }

        else if (disttotargetx == dist_too_far)
        {
            if ((Random(4) == 0) || (with.weapon.num == minigun_num))
                with.control.fire = true;

            if (with.brain.camper > 0)
            {
                if (Random(300) == 0)
                    if (with.bodyanimation.id != AnimationType::Prone)
                        with.control.prone = true;

                if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
                {
                    with.control.right = false;
                    with.control.left = false;
                    with.control.up = false;
                    with.control.down = true;
                }
            }
        }

        // move when other player camps
        if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
            if ((SpriteSystem::Get().GetSprite(with.brain.targetnum).brain.currentwaypoint > 0) &&
                (botpath
                     .waypoint
                         [SpriteSystem::Get().GetSprite(with.brain.targetnum).brain.currentwaypoint]
                     .c1 > 0))
            {
                with.control.right = false;
                with.control.left = false;
                if (t.x > m.x)
                    with.control.right = true;
                if (t.x < m.x)
                    with.control.left = true;
            }

        // hide yourself behind collider
        if (CVar::bots_difficulty < 101)
            if (with.colliderdistance < 255)
            {
                with.control.down = true;

                if (with.brain.camper > 0)
                {
                    with.control.left = false;
                    with.control.right = false;

                    // shoot!
                    if ((Random(4) == 0) || (with.weapon.num == minigun_num))
                        with.control.fire = true;
                }

                if (with.bodyanimation.id == AnimationType::HandSupAim)
                    if (with.bodyanimation.currframe != 11)
                        with.control.fire = false;

                /*if Brain.Camper > 128 then
                if ColliderDistance < DIST_COLLIDE then
                  control.Prone := true;*/
            }

        // get up if not behind collider
        /*if ColliderDistance > DIST_STOP_PRONE then
          if BodyAnimation.Name = Prone.Name then
            control.Prone := True;*/

        // if target behind collider and bot doesn't escape
        if (CVar::bots_difficulty < 201)
            if ((SpriteSystem::Get().GetSprite(with.brain.targetnum).colliderdistance < 255) &&
                (with.colliderdistance > 254))
                if (with.brain.camper > 0)
                {
                    if (t.x < m.x)
                        with.control.right = true;
                    if (t.x > m.x)
                        with.control.left = true;
                }

        // go prone
        // Fists!
        if (((SpriteSystem::Get().GetSprite(snum).weapon.num == noweapon_num) ||
             (SpriteSystem::Get().GetSprite(snum).weapon.num == knife_num) ||
             (SpriteSystem::Get().GetSprite(snum).weapon.num == chainsaw_num)) &&
            (((SpriteSystem::Get().GetSprite(with.brain.targetnum).weapon.num != noweapon_num) &&
              (SpriteSystem::Get().GetSprite(with.brain.targetnum).weapon.num != knife_num) &&
              (SpriteSystem::Get().GetSprite(with.brain.targetnum).weapon.num != chainsaw_num)) ||
             (t.y > m.y)))
        {
            with.control.right = false;
            with.control.left = false;
            with.control.down = false;
            with.control.fire = true;
            if (t.x > m.x)
                with.control.right = true;
            if (t.x < m.x)
                with.control.left = true;
        }

        // Y - Distance
        disttotargety = checkdistance(m.y, t.y);

        if (!SpriteSystem::Get().GetSprite(snum).brain.gothing)
            if ((disttotargety >= dist_rock_throw) && (m.y > t.y))
                with.control.jetpack = true;

        // Flame god see
        if (SpriteSystem::Get().GetSprite(with.brain.targetnum).bonusstyle == bonus_flamegod)
        {
            with.control.right = false;
            with.control.left = false;
            if (t.x < m.x)
                with.control.right = true;
            if (t.x > m.x)
                with.control.left = true;
        }

        // Change weapon if reloading long
        /*if Difficulty < 201 then
          if (DistToTargetX < DIST_CLOSE) and (DistToTargetY < DIST_CLOSE) then
            if ((Weapon.AmmoCount = 0) and (Weapon.ReloadTimeCount > 185)) or
                (Weapon.FireIntervalCount > 185) then
              Control.ChangeWeapon := True;*/

        // Realistic Mode - Burst Fire
        if (CVar::sv_realisticmode)
        {
            if (with.weapon.num != minigun_num)
                if (with.burstcount > 3)
                {
                    with.control.fire = false;
                    if (GS::GetGame().GetMainTickCounter() % second == 0)
                        with.burstcount = 0;
                }

            if (with.weapon.num == minigun_num)
                if (with.burstcount > 30)
                {
                    with.control.fire = false;
                    if (GS::GetGame().GetMainTickCounter() % second == 0)
                        with.burstcount = 0;
                }
        }

        if (SpriteSystem::Get().GetSprite(snum).stat > 0)
        {
            with.control.right = false;
            with.control.left = false;
            with.control.up = false;
            with.control.down = false;
            with.control.fire = true;
        }

        // Grenade throw
        if (with.brain.grenadefreq > -1)
        {
            gr = with.brain.grenadefreq;
            if ((with.weapon.ammocount == 0) || (with.weapon.fireintervalcount > 125))
                gr = gr / 2;
            if ((with.brain.currentwaypoint > 0) &&
                (botpath.waypoint[with.brain.currentwaypoint].c1 > 0))
                gr = gr / 2;
            if (CVar::bots_difficulty < 100)
                gr = gr / 2;

            if (CVar::bots_difficulty < 201)
                if ((Random(gr) == 0) && (disttotargetx < dist_far) &&
                    (with.tertiaryweapon.ammocount > 0) &&
                    (((disttotargety < dist_very_close) && (m.y > t.y)) || (m.y < t.y)))
                    with.control.thrownade = true;
        }

        // Knife Throw
        if ((SpriteSystem::Get().GetSprite(snum).ceasefirecounter < 30) &&
            (SpriteSystem::Get().GetSprite(snum).weapon.num == knife_num) &&
            (SpriteSystem::Get().GetSprite(snum).brain.favweapon == knife_num))
        {
            with.control.fire = false;
            with.control.throwweapon = true;
        }

        vec2scale(tv, targetVelocity, 10);
        vec2add(t, tv);

        with.control.mouseaimx = round(t.x);
        if (disttotargetx < dist_far)
            with.control.mouseaimy = round(t.y - (0.5 * disttotargetx / (with.weapon.speed)) -
                                           with.brain.accuracy + Random(with.brain.accuracy));
        else
            with.control.mouseaimy = round(t.y - (1.75 * disttotargetx / (with.weapon.speed)) -
                                           with.brain.accuracy + Random(with.brain.accuracy));

        if (SpriteSystem::Get().GetSprite(snum).stat > 0)
            with.control.mouseaimy = round(t.y - (0.5 * disttotargetx / (30)) -
                                           with.brain.accuracy + Random(with.brain.accuracy));

        // impossible
        if (CVar::bots_difficulty < 60)
            if ((SpriteSystem::Get().GetSprite(with.brain.targetnum).weapon.num == barrett_num) ||
                (SpriteSystem::Get().GetSprite(with.brain.targetnum).weapon.num == ruger77_num))
            {
                dist = round(sqrt(sqr(m.x - t.x) + sqr(m.y - t.y)));
                with.control.mouseaimx = round(t.x);
                with.control.mouseaimy = round(t.y);

                for (i = 1;
                     i <= round(((float)(dist) /
                                 SpriteSystem::Get().GetSprite(with.brain.targetnum).weapon.speed) *
                                1.0);
                     i++)
                {
                    with.control.mouseaimx = with.control.mouseaimx + round(targetVelocity.x);
                    with.control.mouseaimy = with.control.mouseaimy + round(targetVelocity.y);
                }

                if (with.weapon.fireintervalcount < 3)
                {
                    with.freecontrols();
                    with.control.fire = true;
                    with.control.down = true;

                    if ((with.bodyanimation.id != AnimationType::Stand) &&
                        (with.bodyanimation.id != AnimationType::Recoil) &&
                        (with.bodyanimation.id != AnimationType::Prone) &&
                        (with.bodyanimation.id != AnimationType::Shotgun) &&
                        (with.bodyanimation.id != AnimationType::Barret) &&
                        (with.bodyanimation.id != AnimationType::SmallRecoil) &&
                        (with.bodyanimation.id != AnimationType::AimRecoil) &&
                        (with.bodyanimation.id != AnimationType::HandSupRecoil) &&
                        (with.bodyanimation.id != AnimationType::Aim) &&
                        (with.bodyanimation.id != AnimationType::HandSupAim))
                        with.control.fire = false;
                }
            }

        if (CVar::sv_realisticmode)
            with.control.mouseaimy = with.control.mouseaimy - with.burstcount * 3;
    }
}

void gotothing(std::uint8_t snum, std::uint8_t tnum)
{
    std::int32_t disttotargetx, disttotargety;

    {
        auto &with = SpriteSystem::Get().GetSprite(snum);
        auto &thing = things[tnum];

        const auto &m = SpriteSystem::Get().GetSpritePartsPos(snum);
        auto t = thing.skeleton.pos[2];

        if ((thing.skeleton.pos[2].x > thing.skeleton.pos[1].x) && (m.x < thing.skeleton.pos[2].x))
            t = thing.skeleton.pos[2];
        if ((thing.skeleton.pos[2].x > thing.skeleton.pos[1].x) && (m.x > thing.skeleton.pos[1].x))
            t = thing.skeleton.pos[1];
        if ((thing.skeleton.pos[2].x < thing.skeleton.pos[1].x) && (m.x < thing.skeleton.pos[1].x))
            t = thing.skeleton.pos[1];
        if ((thing.skeleton.pos[2].x < thing.skeleton.pos[1].x) && (m.x > thing.skeleton.pos[2].x))
            t = thing.skeleton.pos[2];

        if (thing.holdingsprite > 0)
            t.y = t.y + 5;

        if (t.x >= m.x)
            with.control.right = true;
        if (t.x < m.x)
            with.control.left = true;

        if ((thing.holdingsprite > 0) && (GS::GetGame().GetTeamFlag(with.player->team) > team_none))
            if ((with.player->team ==
                 SpriteSystem::Get().GetSprite(thing.holdingsprite).player->team) and
                (!thing.inbase))
            {
                // X - Distance
                disttotargetx = checkdistance(m.x, t.x);

                if ((disttotargetx == dist_too_close) || (disttotargetx == dist_very_close))
                {
                    with.control.right = false;
                    with.control.left = false;
                    with.control.down = true;
                }

                if (SpriteSystem::Get().GetSprite(thing.holdingsprite).control.jetpack)
                    with.control.jetpack = true;
                else
                    with.control.jetpack = false;
            }

        // Y - Distance
        disttotargety = checkdistance(m.y, t.y);
        if ((disttotargety >= dist_very_close) && (m.y > t.y))
            with.control.jetpack = true;
    }
}

void controlbot(tsprite &spritec, const twaypoints &botpath)
{
    ZoneScopedN("ControlBot");
    tvector2 b, lookpoint, startpoint;
    std::int32_t k, i;
    bool seeclosest, seething, runaway;
    float d, d2, dt;
    bool tempb;

    auto &map = GS::GetGame().GetMap();

    if ((spritec.player->controlmethod == bot) && !spritec.deadmeat && !spritec.dummy)
    // if (GS::GetGame().GetMainTickCounter() mod (SECOND * 2) = 0) then
    {
        tempb = spritec.control.thrownade;

        spritec.freecontrols();

        if (spritec.bodyanimation.id == AnimationType::Throw)
            spritec.control.thrownade = tempb;
        else
            spritec.control.thrownade = false;

        lookpoint.x = spritec.skeleton.pos[12].x;
        lookpoint.y = spritec.skeleton.pos[12].y - 2;

        // >see?
        // See := False;
        seeclosest = false;
        d = 999999;
        d2 = 0.0;

        {
            ZoneScopedN("SearchingForTarget");
            for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
            {
                if ((sprite.num != spritec.num) && (sprite.player->name != spritec.brain.friend_) &&
                    ((sprite.alpha == 255) || (sprite.holdedthing > 0)) && sprite.isnotspectator())
                {
                    if (!sprite.deadmeat or (sprite.deadmeat && ((spritec.brain.deadkill == 1) &&
                                                                 (sprite.deadtime < 180))))
                    {
                        startpoint = sprite.skeleton.pos[12];
                        // check if ray startpoint is not in map
                        if (map.collisiontest(startpoint, b))
                            startpoint.y = startpoint.y + 6;

                        if (!map.raycast(lookpoint, startpoint, d2, 651))
                        {
                            if (CVar::sv_gamemode == gamestyle_rambo)
                            {
                                if ((sprite.weapon.num == bow_num) ||
                                    (sprite.weapon.num == bow2_num))
                                {
                                    spritec.brain.targetnum = sprite.num;
                                    seeclosest = true;
                                    break;
                                }
                            }

                            if (d > d2)
                            {
                                spritec.brain.targetnum = sprite.num;

                                dt = d;

                                if (!sprite.deadmeat)
                                {
                                    d = d2;
                                }
                                seeclosest = true;

                                // stop throwing grenades and weapons if it's dead
                                if (sprite.deadmeat)
                                {
                                    spritec.control.thrownade = false;
                                    spritec.control.throwweapon = false;
                                }

                                if ((CVar::sv_gamemode == gamestyle_rambo) &&
                                    (spritec.weapon.num != bow_num) &&
                                    (spritec.weapon.num != bow2_num))
                                {
                                    seeclosest = false;
                                    d = dt;
                                }
                                if (GS::GetGame().isteamgame() && spritec.isinsameteam(sprite))
                                {
                                    seeclosest = false;
                                    // D := dt;
                                }
                            }
                        } // if see
                    }
                }
            }
        }

        if (spritec.brain.targetnum > 0)
            if ((SpriteSystem::Get().GetSprite(spritec.brain.targetnum).weapon.num == bow_num) ||
                (SpriteSystem::Get().GetSprite(spritec.brain.targetnum).weapon.num == bow2_num))
                spritec.brain.pissedoff = 0;

        if (spritec.brain.pissedoff == spritec.num)
            spritec.brain.pissedoff = 0;

        if (spritec.brain.pissedoff > 0)
            if (GS::GetGame().isteamgame() &&
                (!CVar::sv_friendlyfire) &
                    SpriteSystem::Get().GetSprite(spritec.brain.pissedoff).isinsameteam(spritec))
                spritec.brain.pissedoff = 0;

        if (spritec.brain.targetnum > 0)
            if (GS::GetGame().isteamgame() and
                (CVar::sv_friendlyfire) &
                    SpriteSystem::Get().GetSprite(spritec.brain.targetnum).isnotinsameteam(spritec))
                spritec.brain.pissedoff = 0;

        if (spritec.brain.pissedoff > 0)
        {
            lookpoint.x = spritec.skeleton.pos[12].x;
            lookpoint.y = spritec.skeleton.pos[12].y - 2;
            startpoint = SpriteSystem::Get().GetSprite(spritec.brain.pissedoff).skeleton.pos[12];
            if (!map.raycast(lookpoint, startpoint, d2, 651))
            {
                spritec.brain.targetnum = spritec.brain.pissedoff;
                seeclosest = true;
            }
            else
                spritec.brain.pissedoff = 0;
        }

        // have flag and not hurt, runaway!!!
        runaway = false;
        if (seeclosest)
            if (spritec.holdedthing > 0)
                if ((things[spritec.holdedthing].style == object_alpha_flag) ||
                    (things[spritec.holdedthing].style == object_bravo_flag))
                    if (SpriteSystem::Get().GetSprite(spritec.brain.targetnum).holdedthing == 0)
                    {
                        seeclosest = false;
                        runaway = true;
                    }

        // GO WITH WAYPOINTS
        if (!seeclosest) // it doesn't see any target
        {
            if (!spritec.brain.gothing)
                if (spritec.stat == 0)
                {
                    if (spritec.brain.currentwaypoint == 0)
                        i = 350;
                    else
                        i = waypointseekradius; // Radius of waypoint seeking

                    auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(spritec.num);
                    k = botpath.findclosest(spritePartsPos.x, spritePartsPos.y, i,
                                            spritec.brain.currentwaypoint);

                    spritec.brain.oldwaypoint = spritec.brain.currentwaypoint;

                    // current pathnum
                    // FIXME set an initial waypoint. this previously did an out-of-bounds read, so
                    // the next assignment doesn't make it worse...
                    if (spritec.brain.nextwaypoint == 0)
                        spritec.brain.nextwaypoint = 1;
                    spritec.brain.pathnum = botpath.waypoint[spritec.brain.nextwaypoint].pathnum;

                    // pathnum for CTF
                    if (CVar::sv_gamemode == gamestyle_ctf)
                    {
                        spritec.brain.pathnum = spritec.player->team;

                        // i have the flag!
                        if (spritec.holdedthing > 0)
                            if ((things[spritec.holdedthing].style == object_alpha_flag) ||
                                (things[spritec.holdedthing].style == object_bravo_flag))
                            {
                                if (spritec.player->team == team_alpha)
                                    spritec.brain.pathnum = 2;
                                if (spritec.player->team == team_bravo)
                                    spritec.brain.pathnum = 1;
                            }
                    }

                    // pathnum for HTF
                    if (CVar::sv_gamemode == gamestyle_htf)
                    {
                        spritec.brain.pathnum = spritec.player->team;

                        // i have the flag!
                        if (spritec.holdedthing > 0)
                            if (things[spritec.holdedthing].style == object_pointmatch_flag)
                            {
                                if (spritec.player->team == team_alpha)
                                    spritec.brain.pathnum = 2;
                                if (spritec.player->team == team_bravo)
                                    spritec.brain.pathnum = 1;
                            }
                    }

                    // pathnum for Infiltration
                    if (CVar::sv_gamemode == gamestyle_inf)
                    {
                        if (spritec.player->team == team_alpha)
                            spritec.brain.pathnum = 1;
                        if (spritec.player->team == team_bravo)
                            spritec.brain.pathnum = 2;

                        if (!things[GS::GetGame().GetTeamFlag(2)].inbase)
                            if (spritec.player->team == team_bravo)
                                spritec.brain.pathnum = 2;

                        // i have the flag!
                        if (spritec.holdedthing > 0)
                            if ((things[spritec.holdedthing].style == object_alpha_flag) ||
                                (things[spritec.holdedthing].style == object_bravo_flag))
                            {
                                if (spritec.player->team == team_alpha)
                                    spritec.brain.pathnum = 2;
                                if (spritec.player->team == team_bravo)
                                    spritec.brain.pathnum = 1;
                            }
                    }

                    if ((spritec.brain.currentwaypoint == 0) || (k > 0))
                        if ((spritec.brain.pathnum == botpath.waypoint[k].pathnum) ||
                            (spritec.brain.currentwaypoint == 0))
                        {
                            spritec.brain.currentwaypoint = k;
                        }

                    if ((spritec.brain.currentwaypoint > 0) &&
                        (spritec.brain.currentwaypoint < max_waypoints))
                    {
                        if (spritec.brain.oldwaypoint != spritec.brain.currentwaypoint)
                        /*and (BotPath.Waypoint[SpriteC.CurrentWaypoint].ConnectionsNum > 0)*/
                        {
                            k = Random(botpath.waypoint[spritec.brain.currentwaypoint]
                                           .connectionsnum) +
                                1;
                            if ((k > 0) && (k < max_waypoints) &&
                                (botpath.waypoint[spritec.brain.currentwaypoint].connections[k] >
                                 0) &&
                                (botpath.waypoint[spritec.brain.currentwaypoint].connections[k] <
                                 max_waypoints))
                            {
                                spritec.brain.nextwaypoint =
                                    botpath.waypoint[spritec.brain.currentwaypoint].connections[k];

                                // face target
                                spritec.control.mouseaimx =
                                    round(botpath.waypoint[spritec.brain.nextwaypoint].x);
                                spritec.control.mouseaimy =
                                    round(botpath.waypoint[spritec.brain.nextwaypoint].y);
                            }
                        }

                        // apply waypoint movements to sprite
                        spritec.control.left = botpath.waypoint[spritec.brain.nextwaypoint].left;
                        spritec.control.right = botpath.waypoint[spritec.brain.nextwaypoint].right;
                        spritec.control.up = botpath.waypoint[spritec.brain.nextwaypoint].up;
                        spritec.control.down = botpath.waypoint[spritec.brain.nextwaypoint].down;
                        spritec.control.jetpack = botpath.waypoint[spritec.brain.nextwaypoint].m2;

                        // Special waypoint
                        if (((CVar::sv_gamemode == gamestyle_inf) &&
                             (spritec.player->team == team_bravo) &&
                             (things[GS::GetGame().GetTeamFlag(2)].inbase) &
                                 (spritec.holdedthing == 0)) ||
                            ((CVar::sv_gamemode == gamestyle_ctf) && (spritec.holdedthing == 0)) ||
                            ((CVar::sv_gamemode != gamestyle_inf) &&
                             (CVar::sv_gamemode != gamestyle_ctf) &&
                             (CVar::sv_gamemode != gamestyle_htf)))
                            // not infiltration escape path
                            if ((botpath.waypoint[spritec.brain.currentwaypoint].c1 == 1) ||
                                ((botpath.waypoint[spritec.brain.currentwaypoint].c1 == 2) &&
                                 (spritec.brain.oneplacecount < 60)) ||
                                ((botpath.waypoint[spritec.brain.currentwaypoint].c1 == 3) &&
                                 (spritec.brain.oneplacecount < 300)) ||
                                ((botpath.waypoint[spritec.brain.currentwaypoint].c1 == 4) &&
                                 (spritec.brain.oneplacecount < 600)) ||
                                ((botpath.waypoint[spritec.brain.currentwaypoint].c1 == 5) &&
                                 (spritec.brain.oneplacecount < 900)) ||
                                ((botpath.waypoint[spritec.brain.currentwaypoint].c1 == 6) &&
                                 (spritec.brain.oneplacecount < 1200)))
                            {
                                spritec.control.left = false;
                                spritec.control.right = false;
                                spritec.control.up = false;
                                spritec.control.down = false;
                                spritec.control.jetpack = false;

                                if (spritec.stat == 0)
                                    if (spritec.brain.camper > 0)
                                        if (spritec.brain.oneplacecount > 180)
                                            spritec.control.down = true;
                            }

                        // fire at guy that is shooting me while running away
                        if (runaway)
                            if (spritec.brain.pissedoff > 0)
                            {
                                auto &spritePartsPos =
                                    SpriteSystem::Get().GetSpritePartsPos(spritec.brain.pissedoff);
                                spritec.control.mouseaimx = round(spritePartsPos.x);
                                spritec.control.mouseaimy =
                                    round(spritePartsPos.y - (1.75 * 100 / spritec.weapon.speed) -
                                          spritec.brain.accuracy + Random(spritec.brain.accuracy));
                                spritec.control.fire = true;
                            }

                        if (spritec.brain.lastwaypoint == spritec.brain.currentwaypoint)
                            spritec.brain.waypointtime += 1;
                        else
                            spritec.brain.waypointtime = 0;
                        spritec.brain.lastwaypoint = spritec.brain.currentwaypoint;

                        // check if standing in place because stuck or sth
                        if (spritec.brain.currentwaypoint > 0)
                        {
                            if (botpath.waypoint[spritec.brain.currentwaypoint].c1 == 0)
                            {
                                if ((spritec.control.left or spritec.control.right) and
                                    !spritec.control.down)
                                {
                                    auto &spritePartsPos =
                                        SpriteSystem::Get().GetSpritePartsPos(spritec.num);
                                    auto &spritePartsOldPos =
                                        SpriteSystem::Get().GetSpritePartsOldPos(spritec.num);
                                    if (distance(spritePartsPos, spritePartsOldPos) < 3)
                                    {
                                        spritec.brain.oneplacecount += 1;
                                    }
                                    else
                                        spritec.brain.oneplacecount = 0;
                                }
                                else
                                    spritec.brain.oneplacecount = 0;
                            }
                            else
                                spritec.brain.oneplacecount += 1;
                        }

                        if (botpath.waypoint[spritec.brain.currentwaypoint].c1 == 0)
                            if (spritec.brain.oneplacecount > 90)
                            {
                                if (spritec.control.left && spritec.control.right)
                                    spritec.control.right = false;
                                spritec.control.up = true;
                            }

                        // change weapon back
                        if (CVar::bots_difficulty < 201)
                            if (((spritec.weapon.num == colt_num) ||
                                 (spritec.weapon.num == noweapon_num) ||
                                 (spritec.weapon.num == knife_num) ||
                                 (spritec.weapon.num == chainsaw_num) ||
                                 (spritec.weapon.num == law_num)) &&
                                (spritec.secondaryweapon.num != noweapon_num))
                                spritec.control.changeweapon = true;

                        // reload if low ammo
                        if (CVar::bots_difficulty < 201)
                            if ((spritec.weapon.ammocount < 4) && (spritec.weapon.ammo > 3))
                                spritec.control.reload = true;

                        // get up if prone
                        if (Random(150) == 0)
                            if ((spritec.bodyanimation.id == AnimationType::Prone) ||
                                (spritec.bodyanimation.id == AnimationType::ProneMove))
                                spritec.control.prone = true;
                    } // SpriteC.CurrentWaypoint>0
                }     // gothing
        }
        else
        {
            if ((spritec.brain.currentwaypoint != 0) &&
                (botpath.waypoint[spritec.brain.currentwaypoint].c1 == 0))
                spritec.brain.currentwaypoint = 0;

            simpledecision(spritec.num, botpath);

            // Camp
            if ((spritec.brain.currentwaypoint > 0) &&
                (((CVar::sv_gamemode == gamestyle_inf) && (spritec.player->team == team_bravo)) ||
                 ((CVar::sv_gamemode == gamestyle_ctf) && (spritec.holdedthing == 0)) ||
                 ((CVar::sv_gamemode != gamestyle_inf) && (CVar::sv_gamemode != gamestyle_ctf))))

                // not infiltration escape path
                if (botpath.waypoint[spritec.brain.currentwaypoint].c1 == 1)
                {
                    spritec.control.left = false;
                    spritec.control.right = false;
                    spritec.control.up = false;
                    spritec.control.down = false;
                    spritec.control.jetpack = false;
                }

            if (CVar::bots_chat)
            {
                if (Random(115 * spritec.brain.chatfreq) == 0)
                    serversendstringmessage((spritec.brain.chatseeenemy), all_players, spritec.num,
                                            msgtype_pub);

                if (Random(790 * spritec.brain.chatfreq) == 0)
                    serversendstringmessage(
                        std::string("Die ") +
                            (SpriteSystem::Get().GetSprite(spritec.brain.targetnum).player->name) +
                            "!",
                        all_players, spritec.num, msgtype_pub);
            }

            spritec.brain.waypointtime = 0;
        }

        seething = false;
        // ThingHolded := False;
        lookpoint.x = spritec.skeleton.pos[12].x;
        lookpoint.y = spritec.skeleton.pos[12].y - 4;
        // look for flag or bow
        {
            ZoneScopedN("LookingForFlagOrBow");
            for (i = 1; i <= max_things; i++)
            {
                auto &thing = things[i];
                if (!seething && thing.active && (thing.holdingsprite != spritec.num) &&
                    ((thing.style == object_alpha_flag) || (thing.style == object_bravo_flag) ||
                     (thing.style == object_pointmatch_flag) || (thing.style == object_rambo_bow) ||
                     (thing.style == object_flamer_kit) || (thing.style == object_predator_kit) ||
                     (thing.style == object_vest_kit) || (thing.style == object_berserk_kit) ||
                     (thing.style == object_combat_knife) ||
                     ((thing.style == object_medical_kit) &&
                      (spritec.GetHealth() < GS::GetGame().GetStarthealth())) ||
                     ((thing.style == object_grenade_kit) &&
                      (spritec.tertiaryweapon.ammocount < CVar::sv_maxgrenades) &&
                      ((spritec.tertiaryweapon.num != clustergrenade_num) ||
                       (spritec.tertiaryweapon.ammocount == 0)))))
                {
                    startpoint.x = thing.skeleton.pos[2].x;
                    startpoint.y = thing.skeleton.pos[2].y - 5;

                    if (!map.raycast(lookpoint, startpoint, d2, 651))
                        if (d2 < dist_far)
                        { // i see the flag! or bow or sth
                            seething = true;

                            // dont take it if is my flag in base
                            if (((CVar::sv_gamemode == gamestyle_ctf) ||
                                 (CVar::sv_gamemode == gamestyle_inf)) &&
                                (thing.style == spritec.player->team) && thing.inbase)
                            {
                                seething = false;
                                if ((spritec.holdedthing > 0) && (i != spritec.holdedthing))
                                    if (things[spritec.holdedthing].holdingsprite == spritec.num)
                                        seething = true;
                            }
                            // dont follow this flag if my flag is not inbase
                            if (((CVar::sv_gamemode == gamestyle_ctf) ||
                                 (CVar::sv_gamemode == gamestyle_inf)) &&
                                (thing.style != spritec.player->team) and
                                !things[GS::GetGame().GetTeamFlag(spritec.player->team)].inbase)
                                seething = false;
                            // dont take it if is flag in base
                            if (((CVar::sv_gamemode == gamestyle_ctf) ||
                                 (CVar::sv_gamemode == gamestyle_inf)) &&
                                (thing.style != spritec.player->team) &&
                                (thing.style < object_ussocom) && thing.inbase && (d2 > dist_close))
                                seething = false;
                            // or better take it if hurt and medikit is close
                            if ((thing.style == object_medical_kit) &&
                                (spritec.GetHealth() < hurt_health) && (d2 < dist_very_close))
                                seething = true;
                            // dont take it when running away with flag
                            if (((thing.style == object_medical_kit) ||
                                 (thing.style == object_grenade_kit) ||
                                 (thing.style == object_flamer_kit) ||
                                 (thing.style == object_predator_kit) ||
                                 (thing.style == object_berserk_kit)) &&
                                runaway)
                                seething = false;
                            if (((thing.style == object_flamer_kit) ||
                                 (thing.style == object_predator_kit) ||
                                 (thing.style == object_berserk_kit)) &&
                                (spritec.bonusstyle > bonus_none))
                                seething = false;
                            if (thing.style == object_combat_knife)
                                seething = true;

                            // throw away weapon
                            if ((d2 < 30) && (thing.style == object_rambo_bow))
                                spritec.control.throwweapon = true;

                            if (seething)
                            {
                                if (thing.holdingsprite == 0)
                                    thing.interest -= 1;

                                if (thing.interest > 0)
                                {
                                    if (CVar::bots_chat)
                                    {
                                        if (thing.style < object_pointmatch_flag)
                                            if (Random(400 * spritec.brain.chatfreq) == 0)
                                                serversendstringmessage("Flag!", all_players,
                                                                        spritec.num, msgtype_pub);
                                    }

                                    spritec.brain.gothing = true;
                                    gotothing(spritec.num, i);
                                }
                                else
                                    spritec.brain.gothing = false;

                                // Pickup knife!
                                if ((thing.style == object_combat_knife) &&
                                    (spritec.weapon.num == noweapon_num) &&
                                    (spritec.brain.favweapon == knife_num))
                                {
                                    spritec.control.fire = false;
                                    spritec.brain.targetnum = 0;
                                    // ServerSendStringMessage(' Looking for knife...',
                                    // SpriteC.Num);
                                    spritec.brain.gothing = true;
                                    gotothing(spritec.num, i);
                                }
                            }
                        }
                }
                // <see flag?
            }
        }

        if (!seething)
            spritec.brain.gothing = false;

        auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(spritec.num);
        // Runaway from grenade!
        if (CVar::bots_difficulty < 201)
            for (i = 1; i <= max_bullets; i++)
                if (bullet[i].active && (bullet[i].style == bullet_style_fragnade) &&
                    (distance(GetBulletParts().pos[i].x, GetBulletParts().pos[i].y,
                              spritePartsPos.x,
                              spritePartsPos.y) < (fraggrenade_explosion_radius * 1.4)))
                {
                    if (GetBulletParts().pos[i].x > spritePartsPos.x)
                    {
                        spritec.control.left = true;
                        spritec.control.right = false;
                    }
                    else
                    {
                        spritec.control.right = true;
                        spritec.control.left = false;
                    }
                }

        // release grenade
        if ((spritec.bodyanimation.id == AnimationType::Throw) &&
            (spritec.bodyanimation.currframe > 35))
            spritec.control.thrownade = false;

        spritec.brain.waypointtimeoutcounter -= 1;
        if (spritec.brain.waypointtimeoutcounter < 0)
        {
            spritec.brain.currentwaypoint = spritec.brain.oldwaypoint;
            spritec.brain.waypointtimeoutcounter = waypointtimeout;
            spritec.freecontrols();
            spritec.control.up = true;
        }

        // waypoint is shit
        if (spritec.brain.waypointtime > waypoint_timeout)
        {
            spritec.freecontrols();
            spritec.brain.currentwaypoint = 0;
            spritec.brain.gothing = false;
            spritec.brain.waypointtime = 0;
        }

        // fall damage save
        auto &spriteVelocity = SpriteSystem::Get().GetVelocity(spritec.num);
        d = spriteVelocity.y;
        if (d > 3.35)
            spritec.brain.fallsave = 1;
        if (d < 1.35)
            spritec.brain.fallsave = 0;
        if (spritec.brain.fallsave > 0)
            spritec.control.jetpack = true;

        // Bot Chat
        if (CVar::bots_chat)
            if (Random(spritec.brain.chatfreq * 150) == 0)
            {
                if (GS::GetGame().GetSortedPlayers(1).playernum == spritec.num)
                    serversendstringmessage((spritec.brain.chatwinning), all_players, spritec.num,
                                            msgtype_pub);
            }

        if (Random(190) == 0)
            spritec.brain.pissedoff = 0;

        if (spritec.stat > 0)
        {
            spritec.brain.oneplacecount += 1;
            if (((spritec.brain.oneplacecount > 120) && (spritec.brain.oneplacecount < 220)) ||
                ((spritec.brain.oneplacecount > 350) && (spritec.brain.oneplacecount < 620)) ||
                ((spritec.brain.oneplacecount > 700) && (spritec.brain.oneplacecount < 740)) ||
                ((spritec.brain.oneplacecount > 900) && (spritec.brain.oneplacecount < 1100)) ||
                ((spritec.brain.oneplacecount > 1300) && (spritec.brain.oneplacecount < 1500)))
            {
                spritec.control.fire = true;
                if (Random(2) == 0)
                    spritec.control.mouseaimy = spritec.control.mouseaimy + Random(4);
                else
                    spritec.control.mouseaimy = spritec.control.mouseaimy - Random(4);
            }

            if (spritec.brain.oneplacecount > 1500)
                spritec.brain.oneplacecount = 0;
        }

        // destroy weapon if fav weapon hands
        /*if NoWeapon.Name = SpriteC.Brain.FavWeapon then
            if SpriteC.Weapon.Num <> noweapon_num then
              SpriteC.Weapon := Guns[NOWEAPON];*/
    } // Bot
}
