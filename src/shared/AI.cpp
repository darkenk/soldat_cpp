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

void simpledecision(std::uint8_t snum)
{
    tvector2 m, t, tv;
    std::int32_t disttotargetx, disttotargety, dist;
    std::int32_t gr, i;

    {
        auto &with = sprite[snum];

        m = spriteparts.pos[snum];
        t = spriteparts.pos[with.brain.targetnum];

        if (!sprite[snum].brain.gothing)
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
            if (!sprite[snum].brain.gothing)
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
            if (!sprite[snum].brain.gothing)
            {
                with.control.right = false;
                with.control.left = false;
            }
            with.control.fire = true;

            // if reloading
            if (with.weapon.ammocount == 0)
            {
                if (!sprite[snum].brain.gothing)
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
            if (!sprite[snum].brain.gothing)
            {
                with.control.right = false;
                with.control.left = false;
            }
            with.control.down = true;
            with.control.fire = true;

            // if reloading
            if (with.weapon.ammocount == 0)
            {
                if (!sprite[snum].brain.gothing)
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
                if (!sprite[snum].brain.gothing)
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
                if (!sprite[snum].brain.gothing)
                {
                    with.control.up = false;
                    with.control.down = true;
                }
            }
        }

        else if (disttotargetx == dist_very_far)
        {
            with.control.up = true;
            if ((Random(2) == 0) || (with.weapon.num == guns[minigun].num))
                with.control.fire = true;

            if (with.brain.camper > 0)
            {
                if (Random(250) == 0)
                    if (with.bodyanimation.id != prone.id)
                        with.control.prone = true;

                if (!sprite[snum].brain.gothing)
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
            if ((Random(4) == 0) || (with.weapon.num == guns[minigun].num))
                with.control.fire = true;

            if (with.brain.camper > 0)
            {
                if (Random(300) == 0)
                    if (with.bodyanimation.id != prone.id)
                        with.control.prone = true;

                if (!sprite[snum].brain.gothing)
                {
                    with.control.right = false;
                    with.control.left = false;
                    with.control.up = false;
                    with.control.down = true;
                }
            }
        }

        // move when other player camps
        if (!sprite[snum].brain.gothing)
            if ((sprite[with.brain.targetnum].brain.currentwaypoint > 0) &&
                (botpath.waypoint[sprite[with.brain.targetnum].brain.currentwaypoint].c1 > 0))
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
                    if ((Random(4) == 0) || (with.weapon.num == guns[minigun].num))
                        with.control.fire = true;
                }

                if (with.bodyanimation.id == handsupaim.id)
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
            if ((sprite[with.brain.targetnum].colliderdistance < 255) &&
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
        if (((sprite[snum].weapon.num == guns[noweapon].num) ||
             (sprite[snum].weapon.num == guns[knife].num) ||
             (sprite[snum].weapon.num == guns[chainsaw].num)) &&
            (((sprite[with.brain.targetnum].weapon.num != guns[noweapon].num) &&
              (sprite[with.brain.targetnum].weapon.num != guns[knife].num) &&
              (sprite[with.brain.targetnum].weapon.num != guns[chainsaw].num)) ||
             (spriteparts.pos[with.brain.targetnum].y > spriteparts.pos[snum].y)))
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

        if (!sprite[snum].brain.gothing)
            if ((disttotargety >= dist_rock_throw) && (m.y > t.y))
                with.control.jetpack = true;

        // Flame god see
        if (sprite[with.brain.targetnum].bonusstyle == bonus_flamegod)
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
            if (with.weapon.num != guns[minigun].num)
                if (with.burstcount > 3)
                {
                    with.control.fire = false;
                    if (maintickcounter % second == 0)
                        with.burstcount = 0;
                }

            if (with.weapon.num == guns[minigun].num)
                if (with.burstcount > 30)
                {
                    with.control.fire = false;
                    if (maintickcounter % second == 0)
                        with.burstcount = 0;
                }
        }

        if (sprite[snum].stat > 0)
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
        if ((sprite[snum].ceasefirecounter < 30) && (sprite[snum].weapon.num == guns[knife].num) &&
            (sprite[snum].brain.favweapon == guns[knife].num))
        {
            with.control.fire = false;
            with.control.throwweapon = true;
        }

        vec2scale(tv, spriteparts.velocity[with.brain.targetnum], 10);
        vec2add(t, tv);

        with.control.mouseaimx = round(t.x);
        if (disttotargetx < dist_far)
            with.control.mouseaimy = round(t.y - (0.5 * disttotargetx / (with.weapon.speed)) -
                                           with.brain.accuracy + Random(with.brain.accuracy));
        else
            with.control.mouseaimy = round(t.y - (1.75 * disttotargetx / (with.weapon.speed)) -
                                           with.brain.accuracy + Random(with.brain.accuracy));

        if (sprite[snum].stat > 0)
            with.control.mouseaimy = round(t.y - (0.5 * disttotargetx / (30)) -
                                           with.brain.accuracy + Random(with.brain.accuracy));

        // impossible
        if (CVar::bots_difficulty < 60)
            if ((sprite[with.brain.targetnum].weapon.num == guns[barrett].num) ||
                (sprite[with.brain.targetnum].weapon.num == guns[ruger77].num))
            {
                dist = round(sqrt(sqr(m.x - t.x) + sqr(m.y - t.y)));
                with.control.mouseaimx = round(t.x);
                with.control.mouseaimy = round(t.y);

                for (i = 1;
                     i <= round(((float)(dist) / sprite[with.brain.targetnum].weapon.speed) * 1.0);
                     i++)
                {
                    with.control.mouseaimx = with.control.mouseaimx +
                                             round(spriteparts.velocity[with.brain.targetnum].x);
                    with.control.mouseaimy = with.control.mouseaimy +
                                             round(spriteparts.velocity[with.brain.targetnum].y);
                }

                if (with.weapon.fireintervalcount < 3)
                {
                    with.freecontrols();
                    with.control.fire = true;
                    with.control.down = true;

                    if ((with.bodyanimation.id != stand.id) &&
                        (with.bodyanimation.id != recoil.id) &&
                        (with.bodyanimation.id != prone.id) &&
                        (with.bodyanimation.id != shotgun.id) &&
                        (with.bodyanimation.id != barret.id) &&
                        (with.bodyanimation.id != smallrecoil.id) &&
                        (with.bodyanimation.id != aimrecoil.id) &&
                        (with.bodyanimation.id != handsuprecoil.id) &&
                        (with.bodyanimation.id != aim.id) &&
                        (with.bodyanimation.id != handsupaim.id))
                        with.control.fire = false;
                }
            }

        if (CVar::sv_realisticmode)
            with.control.mouseaimy = with.control.mouseaimy - with.burstcount * 3;
    }
}

void gotothing(std::uint8_t snum, std::uint8_t tnum)
{
    tvector2 m, t;
    std::int32_t disttotargetx, disttotargety;

    {
        auto &with = sprite[snum];

        m = spriteparts.pos[snum];
        t = thing[tnum].skeleton.pos[2];

        if ((thing[tnum].skeleton.pos[2].x > thing[tnum].skeleton.pos[1].x) &&
            (m.x < thing[tnum].skeleton.pos[2].x))
            t = thing[tnum].skeleton.pos[2];
        if ((thing[tnum].skeleton.pos[2].x > thing[tnum].skeleton.pos[1].x) &&
            (m.x > thing[tnum].skeleton.pos[1].x))
            t = thing[tnum].skeleton.pos[1];
        if ((thing[tnum].skeleton.pos[2].x < thing[tnum].skeleton.pos[1].x) &&
            (m.x < thing[tnum].skeleton.pos[1].x))
            t = thing[tnum].skeleton.pos[1];
        if ((thing[tnum].skeleton.pos[2].x < thing[tnum].skeleton.pos[1].x) &&
            (m.x > thing[tnum].skeleton.pos[2].x))
            t = thing[tnum].skeleton.pos[2];

        if (thing[tnum].holdingsprite > 0)
            t.y = t.y + 5;

        if (t.x >= m.x)
            with.control.right = true;
        if (t.x < m.x)
            with.control.left = true;

        if ((thing[tnum].holdingsprite > 0) && (teamflag[with.player->team] > team_none))
            if ((with.player->team == sprite[thing[tnum].holdingsprite].player->team) and
                (!thing[tnum].inbase))
            {
                // X - Distance
                disttotargetx = checkdistance(m.x, t.x);

                if ((disttotargetx == dist_too_close) || (disttotargetx == dist_very_close))
                {
                    with.control.right = false;
                    with.control.left = false;
                    with.control.down = true;
                }

                if (sprite[thing[tnum].holdingsprite].control.jetpack)
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

void controlbot(tsprite &spritec)
{
    tvector2 b, lookpoint, startpoint;
    std::int32_t k, i;
    bool seeclosest, seething, runaway;
    float d, d2, dt;
    bool tempb;

    if ((spritec.player->controlmethod == bot) && !spritec.deadmeat && !spritec.dummy)
    // if (MainTickCounter mod (SECOND * 2) = 0) then
    {
        tempb = spritec.control.thrownade;

        spritec.freecontrols();

        if (spritec.bodyanimation.id == throw_.id)
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
        for (i = 1; i <= max_sprites; i++)
            if (sprite[i].active && (i != spritec.num) &&
                (sprite[i].player->name != spritec.brain.friend_) &&
                ((sprite[i].alpha == 255) || (sprite[i].holdedthing > 0)) &&
                sprite[i].isnotspectator())
                if (!sprite[i].deadmeat or (sprite[i].deadmeat && ((spritec.brain.deadkill == 1) &&
                                                                   (sprite[i].deadtime < 180))))
                {
                    startpoint = sprite[i].skeleton.pos[12];
                    // check if ray startpoint is not in map
                    if (map.collisiontest(startpoint, b))
                        startpoint.y = startpoint.y + 6;

                    if (!map.raycast(lookpoint, startpoint, d2, 651))
                    {
                        if (CVar::sv_gamemode == gamestyle_rambo)
                            if ((sprite[i].weapon.num == guns[bow].num) ||
                                (sprite[i].weapon.num == guns[bow2].num))
                            {
                                spritec.brain.targetnum = i;
                                seeclosest = true;
                                break;
                            }

                        if (d > d2)
                        {
                            spritec.brain.targetnum = i;

                            dt = d;

                            if (!sprite[i].deadmeat)
                                d = d2;
                            seeclosest = true;

                            // stop throwing grenades and weapons if it's dead
                            if (sprite[i].deadmeat)
                            {
                                spritec.control.thrownade = false;
                                spritec.control.throwweapon = false;
                            }

                            if ((CVar::sv_gamemode == gamestyle_rambo) &&
                                (spritec.weapon.num != guns[bow].num) &&
                                (spritec.weapon.num != guns[bow2].num))
                            {
                                seeclosest = false;
                                d = dt;
                            }
                            if (isteamgame() && spritec.isinsameteam(sprite[i]))
                            {
                                seeclosest = false;
                                // D := dt;
                            }
                        }
                    } // if see
                }
        // <see?

        if (spritec.brain.targetnum > 0)
            if ((sprite[spritec.brain.targetnum].weapon.num == guns[bow].num) ||
                (sprite[spritec.brain.targetnum].weapon.num == guns[bow2].num))
                spritec.brain.pissedoff = 0;

        if (spritec.brain.pissedoff == spritec.num)
            spritec.brain.pissedoff = 0;

        if (spritec.brain.pissedoff > 0)
            if (isteamgame() &&
                (!CVar::sv_friendlyfire) & sprite[spritec.brain.pissedoff].isinsameteam(spritec))
                spritec.brain.pissedoff = 0;

        if (spritec.brain.targetnum > 0)
            if (isteamgame() and
                (CVar::sv_friendlyfire) & sprite[spritec.brain.targetnum].isnotinsameteam(spritec))
                spritec.brain.pissedoff = 0;

        if (spritec.brain.pissedoff > 0)
        {
            lookpoint.x = spritec.skeleton.pos[12].x;
            lookpoint.y = spritec.skeleton.pos[12].y - 2;
            startpoint = sprite[spritec.brain.pissedoff].skeleton.pos[12];
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
                if ((thing[spritec.holdedthing].style == object_alpha_flag) ||
                    (thing[spritec.holdedthing].style == object_bravo_flag))
                    if (sprite[spritec.brain.targetnum].holdedthing == 0)
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

                    k = botpath.findclosest(spriteparts.pos[spritec.num].x,
                                            spriteparts.pos[spritec.num].y, i,
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
                            if ((thing[spritec.holdedthing].style == object_alpha_flag) ||
                                (thing[spritec.holdedthing].style == object_bravo_flag))
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
                            if (thing[spritec.holdedthing].style == object_pointmatch_flag)
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

                        if (!thing[teamflag[2]].inbase)
                            if (spritec.player->team == team_bravo)
                                spritec.brain.pathnum = 2;

                        // i have the flag!
                        if (spritec.holdedthing > 0)
                            if ((thing[spritec.holdedthing].style == object_alpha_flag) ||
                                (thing[spritec.holdedthing].style == object_bravo_flag))
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
                             (thing[teamflag[2]].inbase) & (spritec.holdedthing == 0)) ||
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
                                spritec.control.mouseaimx =
                                    round(spriteparts.pos[spritec.brain.pissedoff].x);
                                spritec.control.mouseaimy =
                                    round(spriteparts.pos[spritec.brain.pissedoff].y -
                                          (1.75 * 100 / spritec.weapon.speed) -
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
                            if (botpath.waypoint[spritec.brain.currentwaypoint].c1 == 0)
                            {
                                if ((spritec.control.left or spritec.control.right) and
                                    !spritec.control.down)
                                {
                                    if (distance(spriteparts.pos[spritec.num],
                                                 spriteparts.oldpos[spritec.num]) < 3)
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

                        if (botpath.waypoint[spritec.brain.currentwaypoint].c1 == 0)
                            if (spritec.brain.oneplacecount > 90)
                            {
                                if (spritec.control.left && spritec.control.right)
                                    spritec.control.right = false;
                                spritec.control.up = true;
                            }

                        // change weapon back
                        if (CVar::bots_difficulty < 201)
                            if (((spritec.weapon.num == guns[colt].num) ||
                                 (spritec.weapon.num == guns[noweapon].num) ||
                                 (spritec.weapon.num == guns[knife].num) ||
                                 (spritec.weapon.num == guns[chainsaw].num) ||
                                 (spritec.weapon.num == guns[law].num)) &&
                                (spritec.secondaryweapon.num != guns[noweapon].num))
                                spritec.control.changeweapon = true;

                        // reload if low ammo
                        if (CVar::bots_difficulty < 201)
                            if ((spritec.weapon.ammocount < 4) && (spritec.weapon.ammo > 3))
                                spritec.control.reload = true;

                        // get up if prone
                        if (Random(150) == 0)
                            if ((spritec.bodyanimation.id == prone.id) ||
                                (spritec.bodyanimation.id == pronemove.id))
                                spritec.control.prone = true;
                    } // SpriteC.CurrentWaypoint>0
                }     // gothing
        }
        else
        {
            if ((spritec.brain.currentwaypoint != 0) &&
                (botpath.waypoint[spritec.brain.currentwaypoint].c1 == 0))
                spritec.brain.currentwaypoint = 0;

            simpledecision(spritec.num);

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
                        std::string("Die ") + (sprite[spritec.brain.targetnum].player->name) + "!",
                        all_players, spritec.num, msgtype_pub);
            }

            spritec.brain.waypointtime = 0;
        }

        seething = false;
        // ThingHolded := False;
        lookpoint.x = spritec.skeleton.pos[12].x;
        lookpoint.y = spritec.skeleton.pos[12].y - 4;
        // look for flag or bow
        for (i = 1; i <= max_things; i++)
            if (!seething && thing[i].active && (thing[i].holdingsprite != spritec.num) &&
                ((thing[i].style == object_alpha_flag) || (thing[i].style == object_bravo_flag) ||
                 (thing[i].style == object_pointmatch_flag) ||
                 (thing[i].style == object_rambo_bow) || (thing[i].style == object_flamer_kit) ||
                 (thing[i].style == object_predator_kit) || (thing[i].style == object_vest_kit) ||
                 (thing[i].style == object_berserk_kit) ||
                 (thing[i].style == object_combat_knife) ||
                 ((thing[i].style == object_medical_kit) && (spritec.health < starthealth)) ||
                 ((thing[i].style == object_grenade_kit) &&
                  (spritec.tertiaryweapon.ammocount < CVar::sv_maxgrenades) &&
                  ((spritec.tertiaryweapon.num != guns[clustergrenade].num) ||
                   (spritec.tertiaryweapon.ammocount == 0)))))
            {
                startpoint.x = thing[i].skeleton.pos[2].x;
                startpoint.y = thing[i].skeleton.pos[2].y - 5;

                if (!map.raycast(lookpoint, startpoint, d2, 651))
                    if (d2 < dist_far)
                    { // i see the flag! or bow or sth
                        seething = true;

                        // dont take it if is my flag in base
                        if (((CVar::sv_gamemode == gamestyle_ctf) ||
                             (CVar::sv_gamemode == gamestyle_inf)) &&
                            (thing[i].style == spritec.player->team) && thing[i].inbase)
                        {
                            seething = false;
                            if ((spritec.holdedthing > 0) && (i != spritec.holdedthing))
                                if (thing[spritec.holdedthing].holdingsprite == spritec.num)
                                    seething = true;
                        }
                        // dont follow this flag if my flag is not inbase
                        if (((CVar::sv_gamemode == gamestyle_ctf) ||
                             (CVar::sv_gamemode == gamestyle_inf)) &&
                            (thing[i].style != spritec.player->team) and
                            !thing[teamflag[spritec.player->team]].inbase)
                            seething = false;
                        // dont take it if is flag in base
                        if (((CVar::sv_gamemode == gamestyle_ctf) ||
                             (CVar::sv_gamemode == gamestyle_inf)) &&
                            (thing[i].style != spritec.player->team) &&
                            (thing[i].style < object_ussocom) && thing[i].inbase &&
                            (d2 > dist_close))
                            seething = false;
                        // or better take it if hurt and medikit is close
                        if ((thing[i].style == object_medical_kit) &&
                            (spritec.health < hurt_health) && (d2 < dist_very_close))
                            seething = true;
                        // dont take it when running away with flag
                        if (((thing[i].style == object_medical_kit) ||
                             (thing[i].style == object_grenade_kit) ||
                             (thing[i].style == object_flamer_kit) ||
                             (thing[i].style == object_predator_kit) ||
                             (thing[i].style == object_berserk_kit)) &&
                            runaway)
                            seething = false;
                        if (((thing[i].style == object_flamer_kit) ||
                             (thing[i].style == object_predator_kit) ||
                             (thing[i].style == object_berserk_kit)) &&
                            (spritec.bonusstyle > bonus_none))
                            seething = false;
                        if (thing[i].style == object_combat_knife)
                            seething = true;

                        // throw away weapon
                        if ((d2 < 30) && (thing[i].style == object_rambo_bow))
                            spritec.control.throwweapon = true;

                        if (seething)
                        {
                            if (thing[i].holdingsprite == 0)
                                thing[i].interest -= 1;

                            if (thing[i].interest > 0)
                            {
                                if (CVar::bots_chat)
                                {
                                    if (thing[i].style < object_pointmatch_flag)
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
                            if ((thing[i].style == object_combat_knife) &&
                                (spritec.weapon.num == guns[noweapon].num) &&
                                (spritec.brain.favweapon == guns[knife].num))
                            {
                                spritec.control.fire = false;
                                spritec.brain.targetnum = 0;
                                // ServerSendStringMessage(' Looking for knife...', SpriteC.Num);
                                spritec.brain.gothing = true;
                                gotothing(spritec.num, i);
                            }
                        }
                    }
            }
        // <see flag?

        if (!seething)
            spritec.brain.gothing = false;

        // Runaway from grenade!
        if (CVar::bots_difficulty < 201)
            for (i = 1; i <= max_bullets; i++)
                if (bullet[i].active && (bullet[i].style == bullet_style_fragnade) &&
                    (distance(bulletparts.pos[i].x, bulletparts.pos[i].y,
                              spriteparts.pos[spritec.num].x, spriteparts.pos[spritec.num].y) <
                     (fraggrenade_explosion_radius * 1.4)))
                {
                    if (bulletparts.pos[i].x > spriteparts.pos[spritec.num].x)
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
        if ((spritec.bodyanimation.id == throw_.id) && (spritec.bodyanimation.currframe > 35))
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
        d = spriteparts.velocity[spritec.num].y;
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
                if (sortedplayers[1].playernum == spritec.num)
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
            if SpriteC.Weapon.Num <> Guns[NOWEAPON].Num then
              SpriteC.Weapon := Guns[NOWEAPON];*/
    } // Bot
}
