// automatically converted

#include "Sprites.hpp"
#ifndef SERVER
#include "../../client/Client.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/GameStrings.hpp"
#include "../../client/Sound.hpp"
#include "../../shared/network/NetworkClientSprite.hpp"
#else
#include "../../server/Server.hpp"
#include "../../server/ServerHelper.hpp"
#include "../LogFile.hpp"
#include "../network/NetworkServerConnection.hpp"
#include "../network/NetworkServerGame.hpp"
#include "../network/NetworkServerMessages.hpp"
#include "../network/NetworkServerSprite.hpp"
#include "../network/NetworkServerThing.hpp"
#endif
#include "../Cvar.hpp"
#include "../Game.hpp"
#include "Control.hpp"
#include "common/Calc.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include <Tracy.hpp>
#include <client/ClientGame.hpp>
#include <numbers>

//clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
//clang-format on

#ifndef SERVER
bool wasreloading;
#endif

auto constexpr LOG = "sprites";

using std::numbers::pi;

template <Config::Module M>
std::int32_t createsprite(tvector2 &spos, tvector2 &svelocity, std::uint8_t sstyle, std::uint8_t n,
                          tplayer *player, bool transferownership)
{
    std::int32_t i, j;
    std::int32_t secwep;

    std::int32_t result;
    LogDebug(LOG, "CreateSprite");

    if (n == 255)
    {
        for (i = 1; i <= max_sprites; i++)
        {
            if (i == max_sprites)
            {
                result = -1;
                return result;
            }
            if (!sprite[i].active)
                break;
        }
    }
    else
    {
        i = n; // i is now the active sprite
    }

    result = i;

    // replace player object
    if (sprite[i].player != nullptr)
    {
        sprite[i].player->spritenum = 0;
        if (sprite[i].isplayerobjectowner)
        {
            delete sprite[i].player;
            sprite[i].player = nullptr;
        }
    }
    sprite[i].player = player;
    sprite[i].player->spritenum = i;
    sprite[i].isplayerobjectowner = transferownership;

    sprite[i].active = true;
    sprite[i].style = sstyle;
    sprite[i].num = i;
    sprite[i].deadmeat = false;
    sprite[i].respawncounter = 0;
    sprite[i].ceasefirecounter = ceasefiretime;

    if (CVar::sv_survivalmode)
        sprite[i].ceasefirecounter = sprite[i].ceasefirecounter * 3;

    sprite[i].alpha = 255;
    sprite[i].brain.pissedoff = 0;
    sprite[i].vest = 0;
    sprite[i].bonusstyle = bonus_none;
    sprite[i].bonustime = 0;
    sprite[i].multikills = 0;
    sprite[i].multikilltime = 0;
    sprite[i].tertiaryweapon = guns[fraggrenade];
    sprite[i].hascigar = 0;
    sprite[i].idletime = default_idletime;
    sprite[i].idlerandom = -1;
    sprite[i].position = pos_stand;
    sprite[i].bodyanimation = stand;
    sprite[i].legsanimation = stand;
    sprite[i].onfire = 0;
    sprite[i].holdedthing = 0;
    sprite[i].selweapon = 0;
    sprite[i].stat = 0;

#ifndef SERVER
    sprite[i].olddeadmeat = false;
    sprite[i].halfdead = false;
#endif

    sprite[i].bgstate.backgroundstatus = background_transition;
    sprite[i].bgstate.backgroundpoly = background_poly_unknown;

    svelocity.x = 0;
    svelocity.y = 0;

    if (sprite[i].player->team == team_spectator)
    {
        spos.x = min_sectorz * map.sectorsdivision * 0.8;
        spos.y = min_sectorz * map.sectorsdivision * 0.8;
    }

    // activate sprite part
    spriteparts.createpart(spos, svelocity, 1, i);

    // create skeleton
    sprite[i].skeleton.timestep = 1;
    sprite[i].skeleton.gravity = 1.06 * grav;
    sprite[i].skeleton = gostekskeleton;
    sprite[i].skeleton.vdamping = 0.9945;

    sprite[i].SetHealth(starthealth);
    sprite[i].aimdistcoef = defaultaimdist;

    sprite[i].weapon = guns[noweapon];

    secwep = sprite[i].player->secwep + 1;
    if ((secwep >= 1) && (secwep <= secondary_weapons) &&
        (weaponactive[primary_weapons + secwep] == 1))
        sprite[i].secondaryweapon = guns[primary_weapons + secwep];
    else
        sprite[i].secondaryweapon = guns[noweapon];

    sprite[i].jetscount = map.startjet;
#ifndef SERVER
    sprite[i].jetscountprev = map.startjet;
#endif
    sprite[i].tertiaryweapon.ammocount = CVar::sv_maxgrenades / 2;

    sprite[i].wearhelmet = 1;
    if (sprite[i].player->headcap == 0)
        sprite[i].wearhelmet = 0;

    sprite[i].brain.targetnum = 1;
    sprite[i].brain.waypointtimeoutcounter = waypointtimeout;

    sprite[i].deadcollidecount = 0;

#ifndef SERVER
    sprite[i].reloadsoundchannel = i - 1;
    sprite[i].jetssoundchannel = 1 * max_sprites + i - 1;
    sprite[i].gattlingsoundchannel = 2 * max_sprites + i - 1;
    sprite[i].gattlingsoundchannel2 = 3 * max_sprites + i - 1;
    sprite[i].moveskeleton(spos.x, spos.y, false);
#else
    sprite[i].moveskeleton(0, 0, false);
#endif

    // clear push wait list
    for (j = 0; j <= max_pushtick; j++)
    {
        sprite[i].nextpush[j].x = 0;
        sprite[i].nextpush[j].y = 0;
    }

    sprite[i].bulletcount = Random(std::numeric_limits<std::uint16_t>::max()); // FIXME wat?
    sprite[i].freecontrols();

    sortplayers(); // sort the players frag list
    return result;
}

template <Config::Module M>
bool teamcollides(std::int32_t poly, std::int32_t team, bool bullet)
{
    bool result;
    result = true;
    if (bullet)
    {
        if ((map.polytype[poly] == poly_type_red_bullets) ||
            (map.polytype[poly] == poly_type_red_player))
            if ((team == team_alpha) && (map.polytype[poly] == poly_type_red_bullets))
                result = true;
            else
                result = false;
        else if ((map.polytype[poly] == poly_type_blue_bullets) ||
                 (map.polytype[poly] == poly_type_blue_player))
            if ((team == team_bravo) && (map.polytype[poly] == poly_type_yellow_bullets))
                result = true;
            else
                result = false;
        else if ((map.polytype[poly] == poly_type_yellow_bullets) ||
                 (map.polytype[poly] == poly_type_yellow_player))
            if ((team == team_charlie) && (map.polytype[poly] == poly_type_yellow_bullets))
                result = true;
            else
                result = false;
        else if ((map.polytype[poly] == poly_type_green_bullets) ||
                 (map.polytype[poly] == poly_type_green_player))
        {
            if ((team == team_delta) && (map.polytype[poly] == poly_type_green_bullets))
                result = true;
            else
                result = false;
        }
    }
    else
    {
        if (((map.polytype[poly] == poly_type_red_bullets) && (team == team_alpha)) ||
            (((map.polytype[poly] == poly_type_red_bullets) ||
              (map.polytype[poly] == poly_type_red_player)) &&
             (team != team_alpha)))
            result = false;
        else if (((map.polytype[poly] == poly_type_blue_bullets) && (team == team_bravo)) ||
                 (((map.polytype[poly] == poly_type_blue_bullets) ||
                   (map.polytype[poly] == poly_type_blue_player)) &&
                  (team != team_bravo)))
            result = false;
        else if (((map.polytype[poly] == poly_type_yellow_bullets) && (team == team_charlie)) ||
                 (((map.polytype[poly] == poly_type_yellow_bullets) ||
                   (map.polytype[poly] == poly_type_yellow_player)) &&
                  (team != team_charlie)))
            result = false;
        else if (((map.polytype[poly] == poly_type_green_bullets) && (team == team_delta)) ||
                 (((map.polytype[poly] == poly_type_green_bullets) ||
                   (map.polytype[poly] == poly_type_green_player)) &&
                  (team != team_delta)))
            result = false;
    }
    if (map.polytype[poly] == poly_type_non_flagger_collides)
        result = false;
    return result;
}

template <Config::Module M>
void Sprite<M>::update()
{
    ZoneScopedN("Sprite::Update");
    std::int32_t i;
#ifndef SERVER
    std::int32_t k;
    std::int32_t rnd;
    tvector2 m3, m4;
    std::int32_t weaponreloadsound;
#endif
    tvector2 mouseaim, p, m;
    // rotation vars
    tvector2 rnorm, legvector;
    float bodyy, arms;
    float legdistance = 0.0;

#ifdef SERVER
    LogTraceG("TSprite.Update");
#endif

    jetscountprev = jetscount;
    weapon.reloadtimeprev = weapon.reloadtimecount;
    weapon.fireintervalprev = weapon.fireintervalcount;

    bodyy = 0;

    spriteparts.velocity[num] = vec2add(spriteparts.velocity[num], nextpush[0]);
#ifndef SERVER
    for (i = 0; i <= max_pushtick - 1; i++)
        nextpush[i] = nextpush[i + 1];
#endif
    nextpush[max_pushtick].x = 0;
    nextpush[max_pushtick].y = 0;

    // reload spas after shooting delay is over
    if (sprite[num].autoreloadwhencanfire && ((sprite[num].weapon.num != guns[spas12].num) ||
                                              (sprite[num].weapon.fireintervalcount == 0)))
    {
        sprite[num].autoreloadwhencanfire = false;

        if ((sprite[num].weapon.num == guns[spas12].num) &&
            (sprite[num].bodyanimation.id != roll.id) &&
            (sprite[num].bodyanimation.id != rollback.id) &&
            (sprite[num].bodyanimation.id != change.id) &&
            (sprite[num].weapon.ammocount != sprite[num].weapon.ammo))
        {
            sprite[num].bodyapplyanimation(reload, 1);
        }
    }

#ifndef SERVER
    if (clientstopmovingcounter > 0)
#else
    if (((player->controlmethod == human) && (noclientupdatetime[num] < clientstopmove_retrys)) ||
        (player->controlmethod == bot))
#endif
        controlsprite(sprite[num]);

    if (isspectator())
    {
        deadmeat = true;

#ifndef SERVER
        if (num == mysprite)
        {
            respawncounter = 19999;
            gamemenushow(limbomenu, false);
        }
#endif
    }

    skeleton.oldpos[21] = skeleton.pos[21];
    skeleton.oldpos[23] = skeleton.pos[23];
    skeleton.oldpos[25] = skeleton.pos[25];
    skeleton.pos[21] = skeleton.pos[9];
    skeleton.pos[23] = skeleton.pos[12];
    skeleton.pos[25] = skeleton.pos[5];
    if (!deadmeat)
    {
        vec2add(skeleton.pos[21], spriteparts.velocity[num]);
        vec2add(skeleton.pos[23], spriteparts.velocity[num]);
        vec2add(skeleton.pos[25], spriteparts.velocity[num]);
    }

    switch (position)
    {
    case pos_stand:
        bodyy = 8;
        break;
    case pos_crouch:
        bodyy = 9;
        break;
    case pos_prone: {
        if (bodyanimation.id == prone.id)
        {
            if (bodyanimation.currframe > 9)
                bodyy = -2;
            else
                bodyy = 14 - bodyanimation.currframe;
        }
        else
            bodyy = 9;

        if (bodyanimation.id == pronemove.id)
            bodyy = 0;
    }
    break;
    }

    if (bodyanimation.id == getup.id)
    {
        if (bodyanimation.currframe > 18)
            bodyy = 8;
        else
            bodyy = 4;
    }

    if (flaggrabcooldown > 0)
        flaggrabcooldown -= 1;

    // Reset the background poly test before collision checks on the corpse
    if (deadmeat)
        bgstate.backgroundtestprepare();

    if (control.mouseaimx >= spriteparts.pos[num].x)
        direction = 1;
    else
        direction = -1;

    for (i = 1; i <= 20; i++)
    {
        if (skeleton.active[i] && !deadmeat)
        {
            skeleton.oldpos[i] = skeleton.pos[i];

            if (!halfdead)
                // legs
                if ((i == 1) || (i == 4) || (i == 2) || (i == 3) || (i == 5) || (i == 6) ||
                    (i == 17) || (i == 18))
                {
                    skeleton.pos[i].x =
                        spriteparts.pos[num].x +
                        direction * legsanimation.frames[legsanimation.currframe].pos[i].x;
                    skeleton.pos[i].y = spriteparts.pos[num].y +
                                        legsanimation.frames[legsanimation.currframe].pos[i].y;
                }

            // body
            if ((i == 7) || (i == 8) || (i == 9) || (i == 10) || (i == 11) || (i == 12) ||
                (i == 13) || (i == 14) || (i == 15) || (i == 16) || (i == 19) || (i == 20))
            {
                skeleton.pos[i].x =
                    spriteparts.pos[num].x +
                    direction * bodyanimation.frames[bodyanimation.currframe].pos[i].x;
                if (!halfdead)
                    skeleton.pos[i].y = (skeleton.pos[6].y - (spriteparts.pos[num].y - bodyy)) +
                                        spriteparts.pos[num].y +
                                        bodyanimation.frames[bodyanimation.currframe].pos[i].y;
                else
                    skeleton.pos[i].y = 9 + spriteparts.pos[num].y +
                                        bodyanimation.frames[bodyanimation.currframe].pos[i].y;
            }
        }
    }

    if (!deadmeat)
    {
        // Rotate parts
        // head
        i = 12;
        {
            p.x = skeleton.pos[i].x;
            p.y = skeleton.pos[i].y;
            mouseaim.x = control.mouseaimx;
            mouseaim.y = control.mouseaimy;
            rnorm = vec2subtract(p, mouseaim);
            vec2normalize(rnorm, rnorm);
            vec2scale(rnorm, rnorm, 0.1);
            skeleton.pos[i].x = skeleton.pos[9].x - direction * rnorm.y;
            skeleton.pos[i].y = skeleton.pos[9].y + direction * rnorm.x;

            vec2scale(rnorm, rnorm, 50);
            skeleton.pos[23].x = skeleton.pos[9].x - direction * rnorm.y;
            skeleton.pos[23].y = skeleton.pos[9].y + direction * rnorm.x;
        }

        if (bodyanimation.id == throw_.id)
            arms = -5;
        else
            arms = -7;

        // arm
        i = 15;
        if ((bodyanimation.id != reload.id) && (bodyanimation.id != reloadbow.id) &&
            (bodyanimation.id != clipin.id) && (bodyanimation.id != clipout.id) &&
            (bodyanimation.id != slideback.id) && (bodyanimation.id != change.id) &&
            (bodyanimation.id != throwweapon.id) && (bodyanimation.id != weaponnone.id) &&
            (bodyanimation.id != punch.id) && (bodyanimation.id != roll.id) &&
            (bodyanimation.id != rollback.id) && (bodyanimation.id != cigar.id) &&
            (bodyanimation.id != match.id) && (bodyanimation.id != smoke.id) &&
            (bodyanimation.id != wipe.id) && (bodyanimation.id != takeoff.id) &&
            (bodyanimation.id != groin.id) && (bodyanimation.id != piss.id) &&
            (bodyanimation.id != mercy.id) && (bodyanimation.id != mercy2.id) &&
            (bodyanimation.id != victory.id) && (bodyanimation.id != own.id) &&
            (bodyanimation.id != melee.id))
        {
            p.x = skeleton.pos[i].x;
            p.y = skeleton.pos[i].y;
            mouseaim.x = control.mouseaimx;
            mouseaim.y = control.mouseaimy;
            rnorm = vec2subtract(p, mouseaim);
            vec2normalize(rnorm, rnorm);
            vec2scale(rnorm, rnorm, arms);
            m.x = skeleton.pos[16].x;
            m.y = skeleton.pos[16].y;
            p = vec2add(m, rnorm);
            skeleton.pos[i].x = p.x;
            skeleton.pos[i].y = p.y;
        }

        if (bodyanimation.id == throw_.id)
            arms = -6;
        else
            arms = -8;

        // arm
        i = 19;
        if ((bodyanimation.id != reload.id) && (bodyanimation.id != reloadbow.id) &&
            (bodyanimation.id != clipin.id) && (bodyanimation.id != clipout.id) &&
            (bodyanimation.id != slideback.id) && (bodyanimation.id != change.id) &&
            (bodyanimation.id != throwweapon.id) && (bodyanimation.id != weaponnone.id) &&
            (bodyanimation.id != punch.id) && (bodyanimation.id != roll.id) &&
            (bodyanimation.id != rollback.id) && (bodyanimation.id != cigar.id) &&
            (bodyanimation.id != match.id) && (bodyanimation.id != smoke.id) &&
            (bodyanimation.id != wipe.id) && (bodyanimation.id != takeoff.id) &&
            (bodyanimation.id != groin.id) && (bodyanimation.id != piss.id) &&
            (bodyanimation.id != mercy.id) && (bodyanimation.id != mercy2.id) &&
            (bodyanimation.id != victory.id) && (bodyanimation.id != own.id) &&
            (bodyanimation.id != melee.id))
        {
            p.x = skeleton.pos[i].x;
            p.y = skeleton.pos[i].y;
            mouseaim.x = control.mouseaimx;
            mouseaim.y = control.mouseaimy;
            rnorm = vec2subtract(p, mouseaim);
            vec2normalize(rnorm, rnorm);
            vec2scale(rnorm, rnorm, arms);
            m.x = skeleton.pos[16].x;
            m.y = skeleton.pos[16].y - 4;
            p = vec2add(m, rnorm);
            skeleton.pos[i].x = p.x;
            skeleton.pos[i].y = p.y;
        }
    }

    for (i = 1; i <= 20; i++)
    {
        // dead part
        if (deadmeat || (halfdead && isnotspectator()))
        {
            if ((i != 17) && (i != 18) && (i != 19) && (i != 20) && (i != 8) && (i != 7) &&
                (i < 21))
            {
                onground = checkskeletonmapcollision(i, skeleton.pos[i].x, skeleton.pos[i].y);
            }

#ifndef SERVER
            // bleed
            // check where constraints are cut then BLEED
            for (k = 1; k <= skeleton.constraintcount; k++)
            {
                if (!skeleton.constraints[k].active and
                    ((skeleton.constraints[k].parta == i) || (skeleton.constraints[k].partb == i)))
                {
                    m4 = skeleton.pos[i];
                    m4.y = m4.y + 2;
                    m3 = vec2subtract(skeleton.pos[i], skeleton.oldpos[i]);
                    vec2scale(m3, m3, 0.35);

                    if (sparkscount > 300)
                        rnd = blood_random_low;
                    else if (sparkscount > 50)
                        rnd = blood_random_normal;
                    else
                        rnd = blood_random_high;

                    if (deadtime > lessbleed_time)
                        rnd = 2 * rnd;
                    if (deadtime > nobleed_time)
                        rnd = 100 * rnd;

                    if (CVar::r_maxsparks < (max_sparks - 10))
                        rnd = 2 * rnd;

                    if ((k != 10) && (k != 11))
                    {
                        if (Random(rnd) == 0)
                            createspark(m4, m3, 5, num, 85 - Random(25));
                        else if (Random(rnd / 3) == 0)
                            createspark(m4, m3, 4, num, 85 - Random(25));
                    }
                }
            } // bleed

            // fire
            if (deadtime < onfire_time)
                if (onfire > 0)
                    if (i % onfire == 0)
                    {
                        m4 = skeleton.pos[i];
                        m4.y = m4.y + 3;
                        m3 = vec2subtract(skeleton.pos[i], skeleton.oldpos[i]);
                        vec2scale(m3, m3, 0.3);

                        rnd = fire_random_normal;
                        if (sparkscount > 170)
                            rnd = fire_random_low;
                        if (sparkscount < 17)
                            rnd = fire_random_high;

                        if (CVar::r_maxsparks < (max_sparks - 10))
                            rnd = 2 * rnd;

                        if (Random(rnd) == 0)
                        {
                            createspark(m4, m3, 36, num, 35);
                            if (Random(8) == 0)
                                playsound(sfx_onfire, spriteparts.pos[num]);
                            if (Random(2) == 0)
                                playsound(sfx_firecrack, spriteparts.pos[num]);
                        }
                        else if (Random(rnd / 3) == 0)
                            createspark(m4, m3, 37, num, 75);
                    }
#endif
        }
    }

    // If no background poly contact in CheckSkeletonMapCollision() then reset any background poly
    // status
    if (deadmeat)
        bgstate.backgroundtestreset();

#ifdef SERVER
    LogTraceG("TSprite.Update 2");
#endif

    if (!deadmeat)
    {
        switch (style)
        {
        case 1: {
            bodyanimation.doanimation();
            legsanimation.doanimation();

            checkoutofbounds();

            onground = false;

#ifndef SERVER
            if (olddeadmeat)
            {
                respawn();
                olddeadmeat = deadmeat;
            }
#endif

            // Reset the background poly test before collision checks
            bgstate.backgroundtestprepare();

            // head
            checkmapcollision(spriteparts.pos[num].x - 3.5, spriteparts.pos[num].y - 12, 1);

            checkmapcollision(spriteparts.pos[num].x + 3.5, spriteparts.pos[num].y - 12, 1);

            bodyy = 0;
            arms = 0;

            // Walking either left or right (though only one can be active at once)
            if (control.left ^ control.right)
            {
                // If walking in facing direction
                if (control.left ^ (direction == 1))
                    arms = 0.25;
                else // Walking backwards
                    bodyy = 0.25;
            }

            // If a leg is inside a polygon, caused by the modification of ArmS and
            // BodyY, this is there to not lose contact to ground on slope polygons
            if (bodyy == 0)
            {
                legvector = vector2(spriteparts.pos[num].x + 2, spriteparts.pos[num].y + 1.9);
                if (map.raycast(legvector, legvector, legdistance, 10))
                    bodyy = 0.25;
            }
            if (arms == 0)
            {
                legvector = vector2(spriteparts.pos[num].x - 2, spriteparts.pos[num].y + 1.9);
                if (map.raycast(legvector, legvector, legdistance, 10))
                    arms = 0.25;
            }

            // Legs collison check. If collided then don't check the other side as a possible double
            // CheckMapCollision collision would result in too much of a ground repelling force.
            onground = checkmapcollision(spriteparts.pos[num].x + 2,
                                         spriteparts.pos[num].y + 2 - bodyy, 0);

            onground = onground || checkmapcollision(spriteparts.pos[num].x - 2,
                                                     spriteparts.pos[num].y + 2 - arms, 0);

            // radius collison check
            ongroundforlaw = checkradiusmapcollision(spriteparts.pos[num].x,
                                                     spriteparts.pos[num].y - 1, onground);

            onground = checkmapverticescollision(spriteparts.pos[num].x, spriteparts.pos[num].y, 3,
                                                 onground || ongroundforlaw) ||
                       onground;

            // Change the permanent state if the player has had the same OnGround state for two
            // frames in a row
            if (!(onground ^ ongroundlastframe))
                ongroundpermanent = onground;

            ongroundlastframe = onground;

            // If no background poly contact then reset any background poly status
            bgstate.backgroundtestreset();

            // WEAPON HANDLING
#ifndef SERVER
            if ((num == mysprite) || (weapon.fireinterval <= fireinterval_net) or
                !pointvisible(spriteparts.pos[num].x, spriteparts.pos[num].y, camerafollowsprite))
#endif
                if ((weapon.fireintervalcount > 0) &&
                    ((weapon.ammocount > 0) || (weapon.num == guns[spas12].num)))
                {
                    weapon.fireintervalprev = weapon.fireintervalcount;
                    weapon.fireintervalcount -= 1;
                }

            // If fire button is released, then the reload can begin
            if (!sprite[num].control.fire)
                canautoreloadspas = true;

            // reload
            if ((weapon.ammocount == 0) &&
                ((weapon.num == guns[chainsaw].num) ||
                 ((bodyanimation.id != roll.id) && (bodyanimation.id != rollback.id) &&
                  (bodyanimation.id != melee.id) && (bodyanimation.id != change.id) &&
                  (bodyanimation.id != throw_.id) && (bodyanimation.id != throwweapon.id))))
            {
#ifndef SERVER
                if (reloadsoundchannel > -2)
                    setsoundpaused(reloadsoundchannel, false);
#endif

                if (bodyanimation.id != getup.id)
                {
                    // spas is unique - it does the fire interval delay AND THEN reloads. all other
                    // weapons do the opposite.
                    if (weapon.num == guns[spas12].num)
                    {
                        if ((weapon.fireintervalcount == 0) && canautoreloadspas)
                            bodyapplyanimation(reload, 1);
                    }
                    else if ((weapon.num == guns[bow].num) || (weapon.num == guns[bow2].num))
                        bodyapplyanimation(reloadbow, 1);
                    else if ((bodyanimation.id != clipin.id) && (bodyanimation.id != slideback.id))
                    {
                        // Don't show reload animation for chainsaw if one of these
                        // animations are already ongoing
                        if ((weapon.num != guns[chainsaw].num) ||
                            ((bodyanimation.id != roll.id) && (bodyanimation.id != rollback.id) &&
                             (bodyanimation.id != melee.id) && (bodyanimation.id != change.id) &&
                             (bodyanimation.id != throw_.id) &&
                             (bodyanimation.id != throwweapon.id)))
                            bodyapplyanimation(clipout, 1);
                    }

                    burstcount = 0;
                }

#ifndef SERVER
                // play reload sound
                if (weapon.reloadtimecount == weapon.reloadtime)
                {

                    if (weapon.num == guns[eagle].num)
                        weaponreloadsound = sfx_deserteagle_reload;
                    else if (weapon.num == guns[mp5].num)
                        weaponreloadsound = sfx_mp5_reload;
                    else if (weapon.num == guns[ak74].num)
                        weaponreloadsound = sfx_ak74_reload;
                    else if (weapon.num == guns[steyraug].num)
                        weaponreloadsound = sfx_steyraug_reload;
                    else if (weapon.num == guns[ruger77].num)
                        weaponreloadsound = sfx_ruger77_reload;
                    else if (weapon.num == guns[m79].num)
                        weaponreloadsound = sfx_m79_reload;
                    else if (weapon.num == guns[barrett].num)
                        weaponreloadsound = sfx_barretm82_reload;
                    else if (weapon.num == guns[m249].num)
                        weaponreloadsound = sfx_m249_reload;
                    else if (weapon.num == guns[minigun].num)
                        weaponreloadsound = sfx_minigun_reload;
                    else if (weapon.num == guns[colt].num)
                        weaponreloadsound = sfx_colt1911_reload;
                    else
                        weaponreloadsound = -1;

                    if (weaponreloadsound != -1)
                        playsound(weaponreloadsound, spriteparts.pos[num], reloadsoundchannel);
                }

                m3.x = skeleton.pos[15].x;
                m3.y = skeleton.pos[15].y + 6;
                m4.x = spriteparts.velocity[num].x;
                m4.y = spriteparts.velocity[num].y - 0.001;
                if (weapon.reloadtimecount == weapon.clipouttime)
                {
                    if (weapon.num == guns[eagle].num)
                    {
                        createspark(m3, m4, 18, num, 255);
                        m3.x = skeleton.pos[15].x - 2;
                        m3.y = skeleton.pos[15].y + 7;
                        m4.x = spriteparts.velocity[num].x + 0.3;
                        m4.y = spriteparts.velocity[num].y - 0.003;
                        createspark(m3, m4, 18, num, 255);
                    }
                    else if (weapon.num == guns[mp5].num)
                        createspark(m3, m4, 11, num, 255);
                    else if (weapon.num == guns[ak74].num)
                        createspark(m3, m4, 9, num, 255);
                    else if (weapon.num == guns[steyraug].num)
                        createspark(m3, m4, 19, num, 255);
                    else if (weapon.num == guns[barrett].num)
                        createspark(m3, m4, 20, num, 255);
                    else if (weapon.num == guns[m249].num)
                        createspark(m3, m4, 10, num, 255);
                    else if (weapon.num == guns[colt].num)
                        createspark(m3, m4, 23, num, 255);
                }
#endif

                if (weapon.num != guns[spas12].num)
                {
                    // Spas doesn't use the reload time.
                    // If it ever does, be sure to put this back outside.
                    weapon.reloadtimeprev = weapon.reloadtimecount;
                    if (weapon.reloadtimecount > 0)
                        weapon.reloadtimecount -= 1;

                    // spas waits for fire interval to hit 0.
                    // doing this next line for the spas would cause it to never reload when empty.
                    weapon.fireintervalprev = weapon.fireinterval;
                    weapon.fireintervalcount = weapon.fireinterval;

                    if (weapon.reloadtimecount < 1)
                    {
                        weapon.reloadtimeprev = weapon.reloadtime;
                        weapon.fireintervalprev = weapon.fireinterval;
                        weapon.reloadtimecount = weapon.reloadtime;
                        weapon.fireintervalcount = weapon.fireinterval;
                        weapon.startuptimecount = weapon.startuptime;
                        weapon.ammocount = weapon.ammo;
                    }
                }
            }

            // weapon jam fix?
            // TODO: check if server or client do stuff wrong here...
            if (weapon.ammocount == 0)
            {
#ifdef SERVER
                if (weapon.reloadtimecount < 1)
                {
                    weapon.reloadtimecount = weapon.reloadtime;
                    weapon.fireintervalcount = weapon.fireinterval;
                    weapon.startuptimecount = weapon.startuptime;
                    weapon.ammocount = weapon.ammo;
                }
                if (weapon.reloadtimecount > weapon.reloadtime)
                    weapon.reloadtimecount = weapon.reloadtime;
#endif

                if (weapon.num != guns[spas12].num)
                {
                    if (weapon.reloadtimecount < 1)
                    {
#ifdef SERVER
                        bodyapplyanimation(change, 36);
#endif
                        weapon.reloadtimeprev = weapon.reloadtime;
                        weapon.fireintervalprev = weapon.fireinterval;
                        weapon.reloadtimecount = weapon.reloadtime;
                        weapon.fireintervalcount = weapon.fireinterval;
                        weapon.startuptimecount = weapon.startuptime;
                        weapon.ammocount = weapon.ammo;
                    }

#ifndef SERVER
                    if (weapon.reloadtimecount > weapon.reloadtime)
                    {
                        weapon.reloadtimeprev = weapon.reloadtime;
                        weapon.reloadtimecount = weapon.reloadtime;
                    }

                    // didn't we just do this right above? :S
                    if (weapon.num != guns[spas12].num)
                        if (weapon.reloadtimecount < 1)
                        {
                            bodyapplyanimation(change, 36);
                            weapon.reloadtimeprev = weapon.reloadtime;
                            weapon.fireintervalprev = weapon.fireinterval;
                            weapon.reloadtimecount = weapon.reloadtime;
                            weapon.fireintervalcount = weapon.fireinterval;
                            weapon.startuptimecount = weapon.startuptime;
                            weapon.ammocount = weapon.ammo;
                        }
#endif
                }
            }

#ifndef SERVER
            // chainsaw smoke
            if ((weapon.num == guns[chainsaw].num) && (stat == 0))
            {
                if (CVar::r_maxsparks > (max_sparks - 10))
                {
                    if (maintickcounter % 15 == 0)
                    {
                        m3 = skeleton.pos[9];
                        m3.y = m3.y - 2;
                        m3.x = m3.x + direction * 3;
                        m4.x = 0;
                        m4.y = -0.25;
                        createspark(m3, m4, 1, num, 20);
                        if (weapon.ammocount == 0)
                            playsound(sfx_chainsaw_o, spriteparts.pos[num], gattlingsoundchannel);
                        else
                            playsound(sfx_chainsaw_m, spriteparts.pos[num], defaultchannel);
                    }
                }

                if (control.fire)
                {
                    if (weapon.ammocount > 0)
                        playsound(sfx_chainsaw_r, spriteparts.pos[num], gattlingsoundchannel);
                }
            }

            // LAW, chansaw smoke
            if ((weapon.num == guns[law].num) || (weapon.num == guns[chainsaw].num))
            {
                if (weapon.ammocount == 0)
                    if (CVar::r_maxsparks > (max_sparks - 10))
                        if (Random(4) == 0)
                        {
                            m3 = skeleton.pos[9];
                            m3.y = m3.y - 2 - 1 + (float)(Random(60)) / 10;
                            m3.x = m3.x + direction * 3 - 8 + (float)(Random(80)) / 10;
                            m4.x = 0;
                            m4.y = -0.3;
                            createspark(m3, m4, 1, num, 20);
                        }
            }

            // flame arrow on fire
            if (weapon.num == guns[bow2].num)
                if (Random(10) == 0)
                {
                    m3.x = skeleton.pos[15].x + direction * 6;
                    m3.y = skeleton.pos[15].y - 5;
                    createspark(m3, vector2(0, -0.5), 36, num, 40);
                }
#endif

            // JETS
            if
#ifndef SERVER
                ((clientstopmovingcounter > 0))
#else
                (((player->controlmethod == human) &&
                  (noclientupdatetime[num] < clientstopmove_retrys)) ||
                 (player->controlmethod == bot))
#endif
                if ((jetscount < map.startjet) && !control.jetpack)
                {
                    if (onground || (maintickcounter % 2 == 0))
                        jetscount += 1;
                }

            if (ceasefirecounter > -1)
            {
                ceasefirecounter = ceasefirecounter - 1;
                alpha = round(fabs(100 + 70 * sin(sinuscounter)));
            }
            else
            {
                alpha = 255;
            }

            if (bonusstyle == bonus_predator)
                alpha = predatoralpha;

#ifndef SERVER
            // bleed when BERSERKER
            if (bonusstyle == bonus_berserker)
            {
                m4 = skeleton.pos[19];
                m4.x = m4.x - 5 + Random(11);
                m4.y = m4.y - 5 + Random(11);
                m3 = vec2subtract(skeleton.pos[19], skeleton.oldpos[19]);
                m3.y = m3.y - 1.38;
                rnd = blood_random_high;

                if (CVar::r_maxsparks < (max_sparks - 10))
                    rnd = 2 * rnd;

                if (Random(rnd) == 0)
                    createspark(m4, m3, 5, num, 55 - Random(20));
            }

            if (bonusstyle == bonus_flamegod)
            {
                m4 = skeleton.pos[19];
                m4.x = m4.x - 5 + Random(11);
                m4.y = m4.y - 5 + Random(11);
                m3 = vec2subtract(skeleton.pos[19], skeleton.oldpos[19]);
                m3.y = m3.y - 1.38;
                rnd = blood_random_high;

                if (CVar::r_maxsparks < (max_sparks - 10))
                    rnd = 2 * rnd;

                if (Random(rnd) == 0)
                    createspark(m4, m3, 36, num, 40 - Random(10));
            }

            // bleed when hurt
            if (Health < hurt_health)
            {
                m4 = skeleton.pos[5];
                m4.x = m4.x + 2;
                m4.y = m4.y;
                m3 = vec2subtract(skeleton.pos[5], skeleton.oldpos[5]);
                rnd = blood_random_normal;

                if (CVar::r_maxsparks < (max_sparks - 10))
                    rnd = 2 * rnd;

                if (Random(rnd) == 0)
                    createspark(m4, m3, 4, num, 65 - Random(10));
            }
#endif

            // BONUS time
            if (bonustime > -1)
            {
                bonustime = bonustime - 1;
                if (bonustime < 1)
                {
                    switch (bonusstyle)
                    {
                    case bonus_predator:
                        alpha = 255;
                        break;
                    }
                    bonusstyle = bonus_none;
                }
            }
            else
                bonusstyle = bonus_none;

            // MULITKILL TIMER

            if (multikilltime > -1)
            {
                multikilltime = multikilltime - 1;
            }
            else
            {
                multikills = 0;
            }

            // gain health from bow
            if ((maintickcounter % 3 == 0) &&
                ((weapon.num == guns[bow].num) || (weapon.num == guns[bow2].num)) &&
                (Health < (starthealth)))
                Health = Health + 1;

#ifndef SERVER
            // smoke
            if (hascigar == 10)
                if (maintickcounter % 160 == 0)
                {
                    {
                        m3 = skeleton.pos[9];
                        m3.y = m3.y - 2;
                        m3.x = m3.x + direction * 4;
                        m4.x = 0;
                        m4.y = -0.75;
                        createspark(m3, m4, 31, num, 55);
                        if (Random(2) == 0)
                        {
                            m3 = skeleton.pos[9];
                            m3.y = m3.y - 2;
                            m3.x = m3.x + direction * 4.1;
                            m4.x = 0;
                            m4.y = -0.69;
                            createspark(m3, m4, 31, num, 55);
                            playsound(sfx_smoke, spriteparts.pos[num]);
                            if (Random(2) == 0)
                            {
                                m3 = skeleton.pos[9];
                                m3.y = m3.y - 2;
                                m3.x = m3.x + direction * 3.9;
                                m4.x = 0;
                                m4.y = -0.81;
                                createspark(m3, m4, 31, num, 55);
                            }
                        }
                    }
                }

            // winter breath
            if (map.weather == 3)
                if (CVar::r_maxsparks > (max_sparks - 10))
                    if (maintickcounter % 160 == 0)
                    {
                        {
                            m3 = skeleton.pos[9];
                            m3.y = m3.y - 2;
                            m3.x = m3.x + direction * 4;
                            m4.x = 0;
                            m4.y = -0.75;
                            createspark(m3, m4, 31, num, 55);
                        }
                    }
#endif

            // parachuter
            para = 0;
            if ((holdedthing > 0) && (holdedthing < max_things + 1))
                if (thing[holdedthing].style == object_parachute)
                    para = 1;

            if (para == 1)
            {
                spriteparts.forces[num].y = para_speed;
#ifdef SERVER
                if (ceasefirecounter < 1)
#else
                if (((CVar::sv_survivalmode) && (ceasefirecounter < ceasefiretime * 3 - 30)) ||
                    (ceasefirecounter < ceasefiretime - 30))
#endif
                    if (onground || control.jetpack)
                        if ((holdedthing > 0) && (holdedthing < max_things + 1))
                        {
                            thing[holdedthing].holdingsprite = 0;
                            thing[holdedthing].skeleton.constraintcount -= 1;
                            thing[holdedthing].timeout = 3 * 60;
                            holdedthing = 0;
                        }
            }

#ifdef SERVER
            LogTraceG("TSprite.Update 3e");
#endif

            skeleton.doverlettimestepfor(22, 29);
            skeleton.doverlettimestepfor(24, 30);

#ifndef SERVER
            // Ping Impr
            for (i = max_oldpos; i >= 1; i--)
                oldspritepos[num][i] = oldspritepos[num][i - 1];

            oldspritepos[num][0] = spriteparts.pos[num];
#endif
        }
        break; // 1
        }      // case
    }

    if (deadmeat)
    {
        if (isnotspectator())
        {
            // physically integrate skeleton particles
            skeleton.doverlettimestep();

            spriteparts.pos[num] = skeleton.pos[12];

            // Ping Impr
            for (i = max_oldpos; i >= 1; i--)
                oldspritepos[num][i] = oldspritepos[num][i - 1];

            oldspritepos[num][0] = spriteparts.pos[num];

            checkskeletonoutofbounds();

            // Respawn Countdown
#ifdef SERVER
            if (respawncounter < 1)
            {
                respawn();
                serverspritesnapshotmajorfloat(num, netw);
            }
#endif

            respawncounter = respawncounter - 1;

#ifndef SERVER
            if (respawncounter < -360)
            {
                respawncounter = 0;
                respawn();
            }
#endif

#ifdef SERVER
            if (CVar::sv_survivalmode)
            {
                if (respawncounter == 1)
                {
                    if (!survivalendround)
                        respawncounter += 2;
                    else
                    {
                        if (respawncounter < 3)
                        {
                            for (i = 1; i <= max_sprites; i++)
                                if (sprite[i].active && !sprite[i].deadmeat)
                                {
                                    sprite[i].healthhit(4000, i, 1, -1, p);
                                    sprite[i].player->deaths -= 1;
                                }
                        }

                        if (CVar::sv_gamemode != gamestyle_htf)
                        {
                            if ((teamflag[1] > 0) && (teamflag[2] > 0))
                            {
                                if (!thing[teamflag[1]].inbase)
                                {
                                    thing[teamflag[1]].respawn();
                                }
                                if (!thing[teamflag[2]].inbase)
                                {
                                    thing[teamflag[2]].respawn();
                                }
                            }
                        }
                    }
                }
            }
#endif

            // parachuter
            para = 0;
            if ((holdedthing > 0) && (holdedthing < max_things + 1))
                if (thing[holdedthing].style == object_parachute)
                    para = 1;

            if (para == 1)
            {
                skeleton.forces[12].y = 25 * para_speed;
                if (onground)
                    if ((holdedthing > 0) && (holdedthing < max_things + 1))
                    {
                        thing[holdedthing].holdingsprite = 0;
                        thing[holdedthing].skeleton.constraintcount -= 1;
                        thing[holdedthing].timeout = 3 * 60;
                        holdedthing = 0;
                    }
            }

            deadtime += 1;
        }
    } // DeadMeat

    // Safety
    if (spriteparts.velocity[num].x > max_velocity)
        spriteparts.velocity[num].x = max_velocity;
    if (spriteparts.velocity[num].x < -max_velocity)
        spriteparts.velocity[num].x = -max_velocity;
    if (spriteparts.velocity[num].y > max_velocity)
        spriteparts.velocity[num].y = max_velocity;
    if (spriteparts.velocity[num].y < -max_velocity)
        spriteparts.velocity[num].y = -max_velocity;
}

template <Config::Module M>
void Sprite<M>::kill()
{
    std::int32_t i;
    bool left;

#ifdef SERVER
    LogTraceG("TSprite.Kill");
#endif

    // Debug('[Sprite] Deactivate ' + IntToStr(Num));
    active = false;
#ifndef SERVER
    muted = false;
#endif

#ifndef SERVER
    stopsound(reloadsoundchannel);
    stopsound(jetssoundchannel);
    stopsound(gattlingsoundchannel);
    stopsound(gattlingsoundchannel2);
#endif

    if (num > 0)
    {
        sprite[num].skeleton.destroy();
        spriteparts.active[num] = false;
    }

    if ((holdedthing > 0) && (holdedthing < max_things + 1))
        if (thing[holdedthing].style < object_ussocom)
        {
            thing[holdedthing].holdingsprite = 0;
            holdedthing = 0;
        }

    if (stat > 0)
    {
        thing[stat].statictype = false;
        stat = 0;
    }

    if (isnotsolo())
    {
        left = false;
        for (i = 1; i <= max_players; i++)
            if (sprite[i].active && isinsameteam(sprite[i]) && (i != num))
                left = true;

        if (left == false)
            teamscore[player->team] = team_none;
    }

#ifdef SERVER
    if (num > 0)
    {
        noclientupdatetime[num] = 0;
        messagesasecnum[num] = 0;
        floodwarnings[num] = 0;
        pingwarnings[num] = 0;
    }
#endif

    // sort the players frag list
    sortplayers();
}

#ifndef SERVER
// TODO move into Sprite
void selectdefaultweapons(std::uint8_t mysprite)
{
    std::int32_t i, j, k;

    i = 0;
    for (j = 1; j <= primary_weapons; j++)
        if (weaponactive[j] == 1)
            i += 1;

    if (i == 1)
    {
        for (j = 1; j <= primary_weapons; j++)
        {
            if (weaponactive[j] == 1)
            {
                weaponsel[mysprite][j] = 1;
                limbomenu->button[j - 1].active = true;
                sprite[mysprite].selweapon = j;

                if (limbomenu->active && !sprite[mysprite].deadmeat)
                {
                    sprite[mysprite].applyweaponbynum(sprite[mysprite].selweapon, 1);
                    clientspritesnapshot();
                }
                break;
            }
        }
    }

    k = 0;
    for (j = 1; j <= secondary_weapons; j++)
        if (weaponactive[j + primary_weapons] == 1)
            k += 1;

    if (k == 1)
    {
        for (j = primary_weapons + 1; j <= main_weapons; j++)
        {
            if (weaponactive[j] == 1)
            {
                weaponsel[mysprite][j] = 1;
                limbomenu->button[j - 1].active = true;
                sprite[mysprite].player->secwep = j - primary_weapons - 1;

                CVar::cl_player_secwep = (sprite[mysprite].player->secwep);

                if (limbomenu->active && !sprite[mysprite].deadmeat)
                    sprite[mysprite].applyweaponbynum(guns[j].num, 2);
                clientspritesnapshot();
                break;
            }
        }
    }
}

float deg2rad(float deg)
{
    float result;
    result = deg / (180 / std::numbers::pi);
    return result;
}
#endif

template <Config::Module M>
void Sprite<M>::die(std::int32_t how, std::int32_t who, std::int32_t where, std::int32_t what,
                    tvector2 impact)
{
    std::int32_t i, j;
    float k;
    std::string s;
    tvector2 a;
#ifndef SERVER
    tvector2 b;
#else
    std::string s2;
#endif

#ifdef SERVER
    LogTraceG("TSprite.Die");
#endif

    if ((who < 1) || (who > max_sprites))
        return;
    if (what > max_bullets)
        return;

    if (!deadmeat)
    {
        // bullet time
        if (CVar::sv_bullettime)
            if (goalticks == default_goalticks)
            {
                k = 0;
                for (i = 1; i <= max_sprites; i++)
                    if (sprite[i].active && (i != who) && (!sprite[i].player->demoplayer) and
                        sprite[i].isnotspectator())
                        if (distance(spriteparts.pos[i], spriteparts.pos[who]) >
                            bullettime_mindistance)
                            k = 1;

                if (k < 1)
                {
                    togglebullettime(true);
                }
            }

#ifdef SERVER
        if ((CVar::sv_gamemode == gamestyle_inf) || (CVar::sv_gamemode == gamestyle_teammatch) ||
            (CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_htf))
            respawncounter = waverespawncounter + CVar::sv_respawntime_minwave;
        else
            respawncounter = CVar::sv_respawntime;
#endif
        player->deaths += 1;

#ifdef SERVER
        if (what > 0)
            if (((bullet[what].style == bullet_style_arrow) &&
                 (CVar::sv_gamemode != gamestyle_rambo)) ||
                ((bullet[what].style == bullet_style_flamearrow) &&
                 (CVar::sv_gamemode != gamestyle_rambo)))
                if (bullet[what].dontcheat == false)
                {
                    kickplayer(sprite[who].num, true, kick_cheat, day, "Not allowed weapon");
                    return;
                }

        // Anti-Team Killer Protection
        if (CVar::sv_punishtk)
        {
            if (isinsameteam(sprite[who]) && !(CVar::sv_gamemode == gamestyle_deathmatch) &&
                !(CVar::sv_gamemode == gamestyle_rambo) &&
                !(player->name == sprite[who].player->name))
            {
                sprite[who].player->tkwarnings += 1;
                GetMainConsole().console(sprite[who].player->name + " Team Killed " + player->name +
                                             " (Warning #" +
                                             inttostr(sprite[who].player->tkwarnings) + ')',
                                         game_message_color);
                serversendstringmessage(std::string("TK Warning #") +
                                            (inttostr(sprite[who].player->tkwarnings)) +
                                            ". Max Warnings: " + (inttostr(CVar::sv_warnings_tk)),
                                        who, 255, msgtype_pub);
                if (sprite[who].player->tkwarnings > (CVar::sv_warnings_tk / 2))
                {
                    sprite[who].vest = 0;
                    sprite[who].healthhit(200, who, 1, 1, a);
                    serversendstringmessage((sprite[who].player->name) +
                                                " has been punished for TeamKilling. (" +
                                                (inttostr(sprite[who].player->tkwarnings)) + '/' +
                                                (inttostr(CVar::sv_warnings_tk)) + ')',
                                            0, 255, msgtype_pub);
                }
                if (sprite[who].player->tkwarnings > (CVar::sv_warnings_tk - 1))
                    kickplayer(who, true, kick_console, 3600 * 15, "Team Killing");
            }
        }
#endif

        if (who != num)
        {
            if (CVar::sv_gamemode == gamestyle_deathmatch)
            {
                sprite[who].player->kills += 1;

                // mulitkill count
#ifdef SERVER
                sprite[who].multikilltime = multikillinterval;
                sprite[who].multikills += 1;
#endif
            }
            if (CVar::sv_gamemode == gamestyle_pointmatch)
            {
                // add point for kill
                i = 1;

                // add another point for holding the flag
                if ((sprite[who].holdedthing > 0) && (sprite[who].holdedthing < max_things + 1))
                    if (thing[sprite[who].holdedthing].style == object_pointmatch_flag)
                        i = i * 2;

                        // add points for multikill
#ifdef SERVER
                if (sprite[who].multikilltime > 0)
                {
                    if (sprite[who].multikills == 2)
                        i = i * 2;
                    if (sprite[who].multikills == 3)
                        i = i * 4;
                    if (sprite[who].multikills == 4)
                        i = i * 8;
                    if (sprite[who].multikills == 5)
                        i = i * 16;
                    if (sprite[who].multikills > 5)
                        i = i * 32;
                }
#endif

                sprite[who].player->kills += i;

                // mulitkill count
#ifdef SERVER
                sprite[who].multikilltime = multikillinterval;
                sprite[who].multikills += 1;
#endif
            }
            if (CVar::sv_gamemode == gamestyle_teammatch)
            {
                if (isnotinsameteam(sprite[who]))
                {
                    sprite[who].player->kills += 1;
                    teamscore[sprite[who].player->team] += 1;
#ifdef SERVER
                    // mulitkill count
                    sprite[who].multikilltime = multikillinterval;
                    sprite[who].multikills += 1;
#endif
                }
            }
            if (CVar::sv_gamemode == gamestyle_ctf)
            {
                if (isnotinsameteam(sprite[who]))
                {
                    sprite[who].player->kills += 1;
#ifdef SERVER
                    // mulitkill count
                    sprite[who].multikilltime = multikillinterval;
                    sprite[who].multikills += 1;
#endif
                }
            }
            if (CVar::sv_gamemode == gamestyle_inf)
            {
                if (isnotinsameteam(sprite[who]))
                {
                    sprite[who].player->kills += 1;
#ifdef SERVER
                    // mulitkill count
                    sprite[who].multikilltime = multikillinterval;
                    sprite[who].multikills += 1;
#endif
                }
            }
            if (CVar::sv_gamemode == gamestyle_htf)
            {
                if (isnotinsameteam(sprite[who]))
                {
                    sprite[who].player->kills += 1;
#ifdef SERVER
                    // mulitkill count
                    sprite[who].multikilltime = multikillinterval;
                    sprite[who].multikills += 1;
#endif
                }
            }
            if (CVar::sv_gamemode == gamestyle_rambo)
            {
                if (what > 0)
                    i = bullet[what].ownerweapon;
                else
                    i = -1;
                if ((i == guns[bow].num) ||                                   // Shooter is Rambo
                    (i == guns[bow2].num) || (weapon.num == guns[bow].num) || // Shootee is Rambo
                    (weapon.num == guns[bow2].num))
                {
                    sprite[who].player->kills += 1;
#ifdef SERVER
                    // mulitkill count
                    sprite[who].multikilltime = multikillinterval;
                    sprite[who].multikills += 1;
#endif
                }
                else
                {
                    // Punish for killing non-Rambos when someone is Rambo
                    for (i = 1; i <= max_players; i++)
                        if ((sprite[i].weapon.num == guns[bow].num) ||
                            (sprite[i].weapon.num == guns[bow2].num))
                            if (sprite[who].player->kills > 0)
                            {
                                sprite[who].player->kills -= 1;
                                break;
                            }
                }
            }
        }

        if (idlerandom == 7)
        {
            if (weapon.num == guns[noweapon].num)
                how = brutal_death;
        }

        bodyanimation.currframe = 0;

        // console message about kill
        // game log
        if (what > 0)
        {
#ifdef SERVER
            s = weaponnamebynum(bullet[what].ownerweapon, guns);
            if (bullet[what].ownerweapon == 0)
                s = "USSOCOM";
            if (bullet[what].style == bullet_style_fragnade)
                s = "Grenade";
            if (bullet[what].style == bullet_style_cluster)
                s = "Cluster Grenades";
            if (bullet[what].style == bullet_style_punch)
                s = guns[noweapon].name;
            if (bullet[what].style == bullet_style_m2)
                s = "Stationary gun";
                // if Bullet[What].OwnerWeapon = Guns[NOWEAPON].Num then S := 'Selfkill';
#else
            s = weaponnamebynum(what, guns);
            if (what == 222)
                s = "Grenade";
            if (what == 210)
                s = "Clusters";
            if (what == 211)
                s = guns[knife].name;
            if (what == 212)
                s = guns[chainsaw].name;
            if (what == 224)
                s = guns[law].name;
            if (what == 225)
                s = "Stationary gun";
            if (what == 205)
                s = guns[flamer].name;
            if (what == 207)
                s = guns[bow].name;
            if (what == 208)
                s = guns[bow2].name;
            if (what == 206)
                s = guns[noweapon].name;
            if (what == 250)
                s = "Selfkill";
#endif
        }
        else
        {
            s = "Selfkill";
        }

#ifndef SERVER
        if ((who != num) && (who == mysprite))
        {
            for (auto &w : wepstats)
            {
                if (w.name == s)
                {
                    w.kills = w.kills + 1;
                    if (where == 12) /*head*/
                    {
                        w.headshots = w.headshots + 1;
                    }
                }
            }
        }
#endif

#ifdef SERVER
        // console message for kills
        if ((CVar::sv_echokills) && !(sprite[who].player->name == player->name))
        {
            GetMainConsole().console(std::string("(") + inttostr(sprite[who].player->team) + ") " +
                                         sprite[who].player->name + " killed (" +
                                         inttostr(player->team) + ") " + player->name + " with " +
                                         s,
                                     0);
        }
#endif

#ifdef SCRIPT
        // COMMENT: Sprite[Num].Num = Num?
        scrptdispatcher.onplayerkill(sprite[who].num, sprite[num].num, (std::uint8_t)(what));
#endif

        // console message about kill
        // game log
#ifdef SERVER
        {
            if (CVar::log_enable)
            {
                NotImplemented(NITag::OTHER, "No date");
#if 0
                s2 = formatdatetime("yy/mm/dd", get_date());
                s2 = s2 + ' ' + formatdatetime("hh:nn:ss", get_time());
#endif
                addlinetologfile(&GetKillLog(), std::string("--- ") + s2, GetKillLogFilename(),
                                 false);
                addlinetologfile(&GetKillLog(), sprite[who].player->name, GetKillLogFilename(),
                                 false);
                addlinetologfile(&GetKillLog(), sprite[num].player->name, GetKillLogFilename(),
                                 false);
                addlinetologfile(&GetKillLog(), s, GetKillLogFilename(), false);
            }

            // Bot Chat
            if (CVar::bots_chat)
            {
                if (player->controlmethod == bot)
                    if (Random(brain.chatfreq / 2) == 0)
                        serversendstringmessage((brain.chatdead), all_players, num, msgtype_pub);
                if ((who != num) && (sprite[who].player->controlmethod == bot))
                    if (Random(sprite[who].brain.chatfreq / 3) == 0)
                        serversendstringmessage((sprite[who].brain.chatkill), all_players, who,
                                                msgtype_pub);
            }

            k = weapon.hitmultiply;

            lastweaponhm = weapon.hitmultiply;
            lastweaponstyle = weapon.bulletstyle;
            lastweaponspeed = weapon.speed;
            lastweaponfire = weapon.fireinterval;
            lastweaponreload = weapon.reloadtime;

            i = dropweapon();
            weapon.hitmultiply = k;

            if ((i > 0) && (weapon.num != guns[flamer].num) && (weapon.num != guns[noweapon].num))
            {
                thing[i].skeleton.forces[2] = impact;
            }

            freecontrols();
        }
#endif

#ifndef SERVER
        if (((who == mysprite) || (num == mysprite)) && (what > 0) &&
            ispointonscreen(skeleton.pos[9]))
        {
            if ((who == mysprite) && (num == mysprite))
                ;
            else
            {
                screencounter = 5;
                capscreen = 4;
            }
        }
#endif
    }

#ifdef SERVER
    if (what > 0)
    {
        if (((what == 1) && (where == 1)) || (bullet[what].style == bullet_style_flame) || deadmeat)
            ;
        else
        {
            a = vec2subtract(bulletparts.pos[what], bullet[what].initial);
            shotdistance = (float)(vec2length(a)) / 14;
            shotricochet = bullet[what].ricochetcount;
            shotlife = (float)((maintickcounter - bullet[what].startuptime)) / 60;
        }
    }

    LogTraceG("TSprite.Die 2");
#endif

#ifndef SERVER
    if (what > 0)
        if ((where == 12) && ((bullet[what].ownerweapon == guns[ruger77].num)))
            how = headchop_death;
#endif
    switch (how)
    {
    case normal_death: {
#ifndef SERVER
        // the sound of death...
        if (!deadmeat)
            playsound(sfx_death + Random(3), spriteparts.pos[num]);
#endif
    }
    break;

    case headchop_death: {
#ifndef SERVER
        if (deadmeat)
#endif
        {
            if (where == 12)
                skeleton.constraints[20].active = false;
            if (where == 3)
                skeleton.constraints[2].active = false;
            if (where == 4)
                skeleton.constraints[4].active = false;
        }

#ifndef SERVER
        if (what > 0)
            if (!deadmeat && (where == 12) &&
                ((bullet[what].ownerweapon == guns[barrett].num) ||
                 (bullet[what].ownerweapon == guns[ruger77].num)))
            {
                Randomize();
                if (Random(100) > 50)
                {
                    skeleton.constraints[20].active = true; // Keep head attached to corpse
                    for (i = 0; i <= 50; i++)
                    {
                        a.x = skeleton.pos[9].x + (cos(deg2rad((float)(360) / 50 * i)) * 2);
                        a.y = skeleton.pos[9].y + (sin(deg2rad((float)(360) / 50 * i)) * 2);
                        Randomize();
                        // FIXME: Causes range check error
                        // RandSeed := RandSeed * i;
                        b.x = (cos(deg2rad((float)(360) / 50 * i)) * randomrange(1, 3));
                        b.y = (sin(deg2rad((float)(360) / 50 * i)) * randomrange(1, 3));
                        createspark(a, b, iif(i < 25, randomrange(4, 5), 5), num, 100 - Random(20));
                    }
                }

                if (bullet[what].ownerweapon == guns[barrett].num)
                    // corpse explode
                    playsound(sfx_bryzg, sprite[num].skeleton.pos[12]);

                if (who == mysprite)
                    playsound(sfx_boomheadshot);
            }

        // siup leb!
        if (!deadmeat)
        {
            playsound(sfx_headchop, sprite[num].skeleton.pos[12]);
        }
#endif
    }
    break;

    case brutal_death: {
#ifndef SERVER
        if (deadmeat)
#endif
        {
            skeleton.constraints[2].active = false;
            skeleton.constraints[4].active = false;
            skeleton.constraints[20].active = false;
            skeleton.constraints[21].active = false;
            skeleton.constraints[23].active = false;
        }

#ifndef SERVER
        // play bryzg sound!
        playsound(sfx_bryzg, sprite[num].skeleton.pos[12]);
#endif
    }
    break;
    } // case

#ifndef SERVER
    if (deadmeat)
#endif
        if (sprite[who].bonusstyle == bonus_berserker)
        {
            skeleton.constraints[2].active = false;
            skeleton.constraints[4].active = false;
            skeleton.constraints[20].active = false;
            skeleton.constraints[21].active = false;
            skeleton.constraints[23].active = false;

#ifndef SERVER
            playsound(sfx_killberserk, sprite[num].skeleton.pos[12]);
#endif
        }

#ifndef SERVER
    if (!deadmeat && (what > 0))
        if (bullet[what].ownerweapon == guns[flamer].num)
            playsound(sfx_burn, sprite[num].skeleton.pos[12]);
#endif

    if (!deadmeat && (hascigar == 10))
    {
#ifndef SERVER
        createspark(skeleton.pos[12], impact, 34, num, 245);
#endif
        hascigar = 0;
    }

    // Survival Mode
    if (CVar::sv_survivalmode)
        if (!deadmeat)
        {
            if ((CVar::sv_gamemode == gamestyle_deathmatch) ||
                (CVar::sv_gamemode == gamestyle_rambo))
            {
                alivenum = 0;

                for (i = 1; i <= max_sprites; i++)
                    if (sprite[i].active && !sprite[i].deadmeat && sprite[i].isnotspectator())
                        alivenum += 1;

                alivenum -= 1;

                if (alivenum < 2)
                {
                    for (i = 1; i <= max_sprites; i++)
                        if (sprite[i].active)
                            sprite[i].respawncounter = survival_respawntime;

                    survivalendround = true;

#ifndef SERVER
                    for (i = 1; i <= max_sprites; i++)
                        if (sprite[i].active && !sprite[i].deadmeat and
                            (num != i) && // not the current player
                            sprite[i].isnotspectator())
                        {
                            playsound(sfx_roar, spriteparts.pos[i]);
                        }
#endif
                }

#ifndef SERVER
                if (!sprite[mysprite].deadmeat)
#endif
                    GetMainConsole().console(
#ifdef SERVER
                        std::string("Players left: ") +
#else
                    _("Players left:") + ' ' +
#endif
                            (inttostr(alivenum)),
                        game_message_color);
            }

            if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
                (CVar::sv_gamemode == gamestyle_htf) || (CVar::sv_gamemode == gamestyle_teammatch))
            {
                teamalivenum[1] = 0;
                teamalivenum[2] = 0;
                teamalivenum[3] = 0;
                teamalivenum[4] = 0;

                for (i = 1; i <= max_sprites; i++)
                {
                    if (sprite[i].active && !sprite[i].deadmeat and
                        (sprite[i].player->team == team_alpha))
                        teamalivenum[team_alpha] += 1;
                    if (sprite[i].active && !sprite[i].deadmeat and
                        (sprite[i].player->team == team_bravo))
                        teamalivenum[team_bravo] += 1;
                    if (sprite[i].active && !sprite[i].deadmeat and
                        (sprite[i].player->team == team_charlie))
                        teamalivenum[team_charlie] += 1;
                    if (sprite[i].active && !sprite[i].deadmeat and
                        (sprite[i].player->team == team_delta))
                        teamalivenum[team_delta] += 1;
                }

                teamalivenum[player->team] -= 1;

                alivenum = teamalivenum[1] + teamalivenum[2] + teamalivenum[3] + teamalivenum[4];

                if (((teamalivenum[1] > 0) && (teamalivenum[2] < 1) && (teamalivenum[3] < 1) &&
                     (teamalivenum[4] < 1)) ||
                    ((teamalivenum[2] > 0) && (teamalivenum[1] < 1) && (teamalivenum[3] < 1) &&
                     (teamalivenum[4] < 1)) ||
                    ((teamalivenum[3] > 0) && (teamalivenum[1] < 1) && (teamalivenum[2] < 1) &&
                     (teamalivenum[4] < 1)) ||
                    ((teamalivenum[4] > 0) && (teamalivenum[1] < 1) && (teamalivenum[2] < 1) &&
                     (teamalivenum[3] < 1)) ||
                    ((teamalivenum[1] < 1) && (teamalivenum[2] < 1) && (teamalivenum[3] < 1) &&
                     (teamalivenum[4] < 1)))
                {
                    for (i = 1; i <= max_sprites; i++)
                        if (sprite[i].active)
                            sprite[i].respawncounter = survival_respawntime;

                    if (!survivalendround)
                        if (CVar::sv_gamemode == gamestyle_ctf)
                        {
                            if (teamalivenum[1] > 0)
                                teamscore[1] += 1;
                            if (teamalivenum[2] > 0)
                                teamscore[2] += 1;
                        }
                    if (!survivalendround)
                        if (CVar::sv_gamemode == gamestyle_inf)
                        {
                            if (teamalivenum[1] > 0)
                                teamscore[1] += CVar::sv_inf_redaward;

                            // penalty
                            if (playersteamnum[1] > playersteamnum[2])
                                teamscore[1] -= 5 * (playersteamnum[1] - playersteamnum[2]);
                            if (teamscore[1] < 0)
                                teamscore[1] = 0;
                        }

                    survivalendround = true;

                    for (i = 1; i <= max_sprites; i++)
                        if (sprite[i].active && !sprite[i].deadmeat)
                        {
                            sprite[i].idlerandom = 5;
                            sprite[i].idletime = 1;
                        }
                }

#ifndef SERVER
                if (mysprite > 0)
                    if (isinsameteam(sprite[mysprite]))
                        if (!sprite[mysprite].deadmeat)
                            GetMainConsole().console(
                                _("Players left on your team:") + ' ' +
                                    (inttostr(teamalivenum[sprite[mysprite].player->team])),
                                game_message_color);
#endif
            }
        }

#ifdef SERVER
    // Fire on from bullet
    if (what > 0)
    {
        if (bullet[what].style == bullet_style_fragnade)
            if (Random(12) == 0)
                onfire = 4;

        if (bullet[what].style == bullet_style_m79)
            if (Random(8) == 0)
                onfire = 2;

        if (bullet[what].style == bullet_style_flame)
            onfire = 1;

        if (bullet[what].style == bullet_style_flamearrow)
            if (Random(4) == 0)
                onfire = 1;

        if (bullet[what].style == bullet_style_cluster)
            if (Random(3) == 0)
                onfire = 3;
    }
#endif

    for (i = 1; i <= max_things; i++)
    {
        if (thing[i].holdingsprite == num)
            if (thing[i].style < object_ussocom)
            {
                thing[i].holdingsprite = 0;
                holdedthing = 0;
#ifndef SERVER
                if ((thing[i].style == object_alpha_flag) || (thing[i].style == object_bravo_flag))
                {
                    GetMainConsole().console(
                        wideformat(_("{} dropped the {} Flag"), player->name,
                                   iif(player->team == team_alpha, _("Blue"), _("Red"))),
                        iif(player->team == team_alpha, bravo_message_color, alpha_message_color));

                    if (isinsameteam(sprite[mysprite]))
                    {
                        if (thing[i].style == object_alpha_flag) // Alpha
                            bigmessage(wideformat(_("{} Flag dropped!"), _("Red")),
                                       capturemessagewait, capture_message_color);
                        else if (thing[i].style == object_bravo_flag) // Bravo
                            bigmessage(wideformat(_("{} Flag dropped!"), _("Blue")),
                                       capturemessagewait, capture_message_color);

                        playsound(sfx_infilt_point);
                    }
                }
#endif
#ifdef SCRIPT
                scrptdispatcher.onflagdrop(num, thing[i].style, false);
#endif
            }

        if (thing[i].owner == num)
            thing[i].owner = 255;

        if (stat == num)
        {
            stat = 0;
            thing[i].statictype = true;
        }
    }

    // send net info, so the death is smooth
#ifdef SERVER
    if (!deadmeat)
        serverspritedeath(num, who, what, where);

    LogTraceG("TSprite.Die 3");
#endif

#ifndef SERVER
    stopsound(reloadsoundchannel);
#endif

    // BREAD
#ifdef SERVER
    if (!CVar::sv_advancemode)
        if (!deadmeat && (num != who))
#else
    if (CVar::sv_advancemode)
        if (!deadmeat)
#endif
        {
            i = CVar::sv_advancemode_amount;

#ifndef SERVER
            if ((num != who) && (sprite[num].isnotinsameteam(sprite[who]) or sprite[num].issolo()))
#endif
            {
                if ((sprite[who].player->kills % i) == 0)
                {
                    j = 0;
                    for (i = 1; i <= primary_weapons; i++)
                        if ((weaponsel[who][i] == 0) && (weaponactive[i] == 1))
                            j = 1;

                    if (j == 1)
                    {
                        do
                        {
                            j = Random(primary_weapons) + 1;
                        } while (!((weaponsel[who][j] == 0) && (weaponactive[j] == 1)));
                        weaponsel[who][j] = 1;
                    }
                }
            }

            i = CVar::sv_advancemode_amount;

            if ((sprite[num].player->deaths % i) == 0)
            {
                j = 0;
                for (i = 1; i <= primary_weapons; i++)
                    if (weaponsel[num][i] == 1)
                        j = 1;

                if (j == 1)
                {
                    do
                    {
                        j = Random(primary_weapons) + 1;
                    } while (!(weaponsel[num][j] == 1));
                    weaponsel[num][j] = 0;
                }
            }

#ifndef SERVER
            if ((num == mysprite) || (who == mysprite))
                for (i = 1; i <= primary_weapons; i++)
                    if (weaponactive[i] == 1)
                        limbomenu->button[i - 1].active = (bool)(weaponsel[mysprite][i]);
#endif
        }

    deadmeat = true;
#ifdef SERVER
    holdedthing = 0;
#endif
    alpha = 255;
    vest = 0;
    bonusstyle = bonus_none;
    bonustime = 0;
    if ((deadtime > 0) && (onfire == 0))
        deadtime = deadtime / 2;
    else
        deadtime = 0;

    spriteparts.velocity[num].x = 0;
    spriteparts.velocity[num].y = 0;
    sprite[who].brain.pissedoff = 0;

    // sort the players frag list
    sortplayers();
}

template <Config::Module M>
std::int32_t Sprite<M>::dropweapon()
{
    std::int32_t result;
    result = -1;

#ifdef SERVER
    LogTraceG("TSprite.DropWeapon");
#endif

    weaponscleaned = false;
    // drop weapon
#ifdef SERVER
    if (weapon.num == guns[colt].num)
        result = creatething(skeleton.pos[16], num, object_ussocom, 255);
    else if (weapon.num == guns[eagle].num)
        result = creatething(skeleton.pos[16], num, object_desert_eagle, 255);
    else if (weapon.num == guns[mp5].num)
        result = creatething(skeleton.pos[16], num, object_hk_mp5, 255);
    else if (weapon.num == guns[ak74].num)
        result = creatething(skeleton.pos[16], num, object_ak74, 255);
    else if (weapon.num == guns[steyraug].num)
        result = creatething(skeleton.pos[16], num, object_steyr_aug, 255);
    else if (weapon.num == guns[spas12].num)
        result = creatething(skeleton.pos[16], num, object_spas12, 255);
    else if (weapon.num == guns[ruger77].num)
        result = creatething(skeleton.pos[16], num, object_ruger77, 255);
    else if (weapon.num == guns[m79].num)
        result = creatething(skeleton.pos[16], num, object_m79, 255);
    else if (weapon.num == guns[barrett].num)
        result = creatething(skeleton.pos[16], num, object_barret_m82a1, 255);
    else if (weapon.num == guns[m249].num)
        result = creatething(skeleton.pos[16], num, object_minimi, 255);
    else if (weapon.num == guns[minigun].num)
        result = creatething(skeleton.pos[16], num, object_minigun, 255);
    else if (weapon.num == guns[knife].num)
        result = creatething(skeleton.pos[16], num, object_combat_knife, 255);
    else if (weapon.num == guns[chainsaw].num)
        result = creatething(skeleton.pos[16], num, object_chainsaw, 255);
    else if (weapon.num == guns[law].num)
        result = creatething(skeleton.pos[16], num, object_law, 255);

    if (CVar::sv_gamemode == gamestyle_rambo)
    {
        if ((weapon.num == guns[bow].num) || (weapon.num == guns[bow2].num))
        {
            result = creatething(skeleton.pos[16], num, object_rambo_bow, 255);
#ifndef SERVER
            gamethingtarget = result;
#endif
        }
    }

    if (result > 0)
        thing[result].ammocount = weapon.ammocount;

        // This should be called before weapon is actually applied
        // so that sprite still holds old values
#ifdef SCRIPT
    if (result > 0)
    {
        // event must be before actual weapon apply.
        // script might've called ForceWeapon, which we should check.
        // if it did, we don't apply snapshot weapon's as they were already applied
        // by force weapon.
        forceweaponcalled = false;
        scrptdispatcher.onweaponchange(num, guns[noweapon].num, secondaryweapon.num,
                                       guns[noweapon].ammocount, secondaryweapon.ammocount);
    }

    if (!forceweaponcalled)
#endif
        applyweaponbynum(guns[noweapon].num, 1);
#endif
    return result;
}

template <Config::Module M>
void Sprite<M>::legsapplyanimation(const tanimation &anim, std::int32_t curr)
{
#ifdef SERVER
    LogTraceG("TSprite.LegsApplyAnimation");
#endif

    if ((legsanimation.id == prone.id) || (legsanimation.id == pronemove.id))
        return;

    if (anim.id != legsanimation.id)
    {
        legsanimation = anim;
        legsanimation.currframe = curr;
    }
}

template <Config::Module M>
void Sprite<M>::bodyapplyanimation(const tanimation &anim, std::int32_t curr)
{
    ZoneScopedN("ApplybodyAnimation");
#ifdef SERVER
    LogTraceG("TSprite.BodyApplyAnimation");
#endif

#ifndef SERVER
    if (anim.id == stand.id)
        if (wasreloading)
        {
            bodyapplyanimation(reload, 1);
            wasreloading = false;
            return;
        }
#endif

    if (anim.id != bodyanimation.id)
    {
        bodyanimation = anim;
        bodyanimation.currframe = curr;
    }
}

template <Config::Module M>
void Sprite<M>::moveskeleton(float x1, float y1, bool fromzero)
{
    std::int32_t i;

#ifdef SERVER
    LogTraceG("TSprite.MoveSkeleton");
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

template <Config::Module M>
bool Sprite<M>::checkradiusmapcollision(float x, float y, bool hascollided)
{
    std::int32_t j, w, k, z, polytype;
    std::int32_t b = 0;
    tvector2 spos, pos, perp, step;
    tvector2 norm;
    tvector2 p1, p2, p3;
    float d = 0.0;
    std::int32_t detacc;
    std::int32_t rx, ry;
    bool teamcol;

#ifdef SERVER
    bool checkradiusmapcollision_result;
    LogTraceG("TSprite.CheckRadiusMapCollision");
#endif

    bool result = false;
    spos.x = x;
    spos.y = y - 3;

    // make step
    detacc = trunc(vec2length(spriteparts.velocity[num]));
    if (detacc == 0)
        detacc = 1;
    vec2scale(step, spriteparts.velocity[num], (float)(1) / detacc);

    // make steps for accurate collision detection
    for (z = 0; z <= detacc - 1; z++)
    {
        spos.x = spos.x + step.x;
        spos.y = spos.y + step.y;

        // iterate through maps sector polygons
        rx = round((float)(spos.x) / map.sectorsdivision);
        ry = round((float)(spos.y) / map.sectorsdivision);
        if ((rx > -map.sectorsnum) && (rx < map.sectorsnum) && (ry > -map.sectorsnum) &&
            (ry < map.sectorsnum))
        {
            for (j = 1; j <= high(map.sectors[rx][ry].polys); j++)
            {
                w = map.sectors[rx][ry].polys[j];
                polytype = map.polytype[w];

                teamcol = teamcollides(w, player->team, false);

                if (((holdedthing == 0) && (polytype == poly_type_only_flaggers)) ||
                    ((holdedthing != 0) && (polytype == poly_type_not_flaggers)))
                    teamcol = false;
                if (teamcol && (polytype != poly_type_doesnt) &&
                    (polytype != poly_type_only_bullets))
                {
                    for (k = 1; k <= 3; k++)
                    {
                        norm = map.perp[w][k];
                        vec2scale(norm, norm, -sprite_col_radius);

                        pos = vec2add(spos, norm);

                        if (map.pointinpolyedges(pos.x, pos.y, w))
                        {
                            if (bgstate.backgroundtest(w))
                                continue;

                            if (!hascollided)
                                handlespecialpolytypes(polytype, pos);

                            perp = map.closestperpendicular(w, spos, d, b);

                            switch (b)
                            {
                            case 1: {
                                p1.x = map.polys[w].vertices[1].x;
                                p1.y = map.polys[w].vertices[1].y;
                                p2.x = map.polys[w].vertices[2].x;
                                p2.y = map.polys[w].vertices[2].y;
                            }
                            break;
                            case 2: {
                                p1.x = map.polys[w].vertices[2].x;
                                p1.y = map.polys[w].vertices[2].y;
                                p2.x = map.polys[w].vertices[3].x;
                                p2.y = map.polys[w].vertices[3].y;
                            }
                            break;
                            case 3: {
                                p1.x = map.polys[w].vertices[3].x;
                                p1.y = map.polys[w].vertices[3].y;
                                p2.x = map.polys[w].vertices[1].x;
                                p2.y = map.polys[w].vertices[1].y;
                            }
                            break;
                            }

                            p3 = pos;
                            d = pointlinedistance(p1, p2, p3);
                            vec2scale(perp, perp, d);

                            spriteparts.pos[num] = spriteparts.oldpos[num];
                            spriteparts.velocity[num] = vec2subtract(spriteparts.forces[num], perp);

                            result = true;
                            return result;
                        } // PointInPolyEdges
                    }
                }
            } // for j
        }
    } // n
    return result;
}

template <Config::Module M>
bool Sprite<M>::checkmapcollision(float x, float y, std::int32_t area)
{
    ZoneScopedN("CheckMapCollision");
    std::int32_t j, w, polytype;
    std::int32_t k = 0;
    tvector2 spos, pos, perp, step;
    float d = 0.0;
    std::int32_t rx, ry;
    bool teamcol;

#ifdef SERVER
    bool checkmapcollision_result;
    LogTraceG("TSprite.CheckMapCollision");
#endif

    bool result = false;
    spos.x = x;
    spos.y = y;

    pos.x = spos.x + spriteparts.velocity[num].x;
    pos.y = spos.y + spriteparts.velocity[num].y;

    // iterate through maps sector polygons
    rx = round((float)(pos.x) / map.sectorsdivision);
    ry = round((float)(pos.y) / map.sectorsdivision);
    if ((rx > -map.sectorsnum) && (rx < map.sectorsnum) && (ry > -map.sectorsnum) &&
        (ry < map.sectorsnum))
    {
        bgstate.backgroundtestbigpolycenter(pos);

        for (j = 1; j <= high(map.sectors[rx][ry].polys); j++)
        {
            w = map.sectors[rx][ry].polys[j];
            polytype = map.polytype[w];

            teamcol = teamcollides(w, player->team, false);

            if (((polytype != poly_type_doesnt) && (polytype != poly_type_only_bullets) &&
                 teamcol && (polytype != poly_type_only_flaggers) &&
                 (polytype != poly_type_not_flaggers)) ||
                ((holdedthing != 0) && (polytype == poly_type_only_flaggers)) ||
                ((holdedthing == 0) && (polytype == poly_type_not_flaggers)))
            {
                if (map.pointinpoly(pos, map.polys[w]))
                {
                    if (bgstate.backgroundtest(w))
                        continue;

#ifdef SERVER
                    sprite[num].player->standingpolytype = polytype;
#endif

                    handlespecialpolytypes(polytype, pos);

#ifndef SERVER
                    if ((fabs(spriteparts.velocity[num].y) > 2.2) &&
                        (fabs(spriteparts.velocity[num].y) < 3.4) && (polytype != poly_type_bouncy))
                        playsound(sfx_fall, spriteparts.pos[num]);
#endif

                    if (fabs(spriteparts.velocity[num].y) > 3.5)
                    {
#ifndef SERVER
                        playsound(sfx_fall_hard, spriteparts.pos[num]);
#endif

                        // Hit ground
                        if (CVar::sv_realisticmode)
                            if ((spriteparts.velocity[num].y > 3.5) &&
                                (polytype != poly_type_bouncy))
                            {
                                healthhit(spriteparts.velocity[num].y * 5, num, 12, -1, spos);
#ifndef SERVER
                                playsound(sfx_fall, spriteparts.pos[num]);
#endif
                            }
                    }

#ifndef SERVER
                    // Run
                    if (((legsanimation.id == run.id) || (legsanimation.id == runback.id)) &&
                        ((legsanimation.currframe == 16) || (legsanimation.currframe == 32)))
                    {
                        if (CVar::r_maxsparks > (max_sparks - 10))
                            if (fabs(spriteparts.velocity[num].x) > 1.0)
                            {
                                spos.x = (float)(spriteparts.velocity[num].x) / 4;
                                spos.y = -0.8;
                                vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
                                createspark(pos, spos, 1, num, 70);
                            }

                        if (CVar::r_maxsparks > (max_sparks - 10))
                            if ((((direction == 1) && (spriteparts.velocity[num].x < 0.01)) ||
                                 ((direction == -1) && (spriteparts.velocity[num].x > 0.01))) &&
                                (legsanimation.id == run.id))
                            {
                                spos.x = (float)(spriteparts.velocity[num].x) / 4;
                                spos.y = -1.3;
                                vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
                                createspark(pos, spos, 1, num, 70);
                            }

                        if (map.steps == 0)
                            playsound(sfx_step + Random(4), spriteparts.pos[num]);
                        if (map.steps == 1)
                            playsound(sfx_step5 + Random(4), spriteparts.pos[num]);

                        if (map.weather == 1)
                            playsound(sfx_water_step, spriteparts.pos[num]);
                    }

                    // Crouch
                    if (((legsanimation.id == crouchrun.id) ||
                         (legsanimation.id == crouchrunback.id)) &&
                        ((legsanimation.currframe == 15) || (legsanimation.currframe == 1)) &&
                        (legsanimation.count == 1))
                    {
                        if (Random(2) == 0)
                            playsound(sfx_crouch_move, spriteparts.pos[num]);
                        else if (Random(2) == 0)
                            playsound(sfx_crouch_movel, spriteparts.pos[num]);
                    }

                    // Prone
                    if ((legsanimation.id == pronemove.id) && (legsanimation.currframe == 8) &&
                        (legsanimation.count == 1))
                    {
                        playsound(sfx_prone_move, spriteparts.pos[num]);
                    }

                    if ((fabs(spriteparts.velocity[num].x) > 2.4) && (legsanimation.id != run.id) &&
                        (legsanimation.id != runback.id) && (Random(4) == 0))
                    {
                        spos.x = (float)(spriteparts.velocity[num].x) / 4;
                        spos.y = -0.9;
                        vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
                        createspark(pos, spos, 1, num, 70);
                    }
#endif

                    perp = map.closestperpendicular(w, pos, d, k);
                    step = perp;

                    vec2normalize(perp, perp);
                    vec2scale(perp, perp, d);

                    d = vec2length(spriteparts.velocity[num]);
                    if (vec2length(perp) > d)
                    {
                        vec2normalize(perp, perp);
                        vec2scale(perp, perp, d);
                    }

                    if ((area == 0) ||
                        ((area == 1) && ((spriteparts.velocity[num].y < 0) ||
                                         (spriteparts.velocity[num].x > slidelimit) ||
                                         (spriteparts.velocity[num].x < -slidelimit))))
                    {
                        spriteparts.oldpos[num] = spriteparts.pos[num];
                        spriteparts.pos[num] = vec2subtract(spriteparts.pos[num], perp);
                        if (polytype == poly_type_bouncy) // bouncy polygon
                        {
                            vec2normalize(perp, perp);
                            vec2scale(perp, perp, map.bounciness[w] * d);
#ifndef SERVER
                            if (vec2length(perp) > 1)
                            {
                                playsound(sfx_bounce, spriteparts.pos[num]);
                            }
#endif
                        }
                        spriteparts.velocity[num] = vec2subtract(spriteparts.velocity[num], perp);
                    }

                    if (area == 0)
                    {
                        if ((legsanimation.id == stand.id) || (legsanimation.id == crouch.id) ||
                            (legsanimation.id == prone.id) || (legsanimation.id == pronemove.id) ||
                            (legsanimation.id == getup.id) || (legsanimation.id == fall.id) ||
                            (legsanimation.id == mercy.id) || (legsanimation.id == mercy2.id) ||
                            (legsanimation.id == own.id))
                        {
                            if ((spriteparts.velocity[num].x < slidelimit) &&
                                (spriteparts.velocity[num].x > -slidelimit) &&
                                (step.y > slidelimit))
                            {
                                spriteparts.pos[num] = spriteparts.oldpos[num];
                                spriteparts.forces[num].y = spriteparts.forces[num].y - grav;
                            }
                            else
                            {
#ifndef SERVER
                                if (CVar::r_maxsparks > (max_sparks - 10))
                                    if (Random(15) == 0)
                                    {
                                        spos.x = spriteparts.velocity[num].x * 3;
                                        spos.y = -spriteparts.velocity[num].y * 2;
                                        vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
                                        createspark(pos, spos, 1, num, 70);
                                    }
#endif
                            }

                            if ((step.y > slidelimit) && (polytype != poly_type_ice) &&
                                (polytype != poly_type_bouncy))
                            {
                                if ((legsanimation.id == stand.id) ||
                                    (legsanimation.id == fall.id) ||
                                    (legsanimation.id == crouch.id))
                                {
                                    spriteparts.velocity[num].x =
                                        spriteparts.velocity[num].x * standsurfacecoefx;
                                    spriteparts.velocity[num].y =
                                        spriteparts.velocity[num].y * standsurfacecoefy;
                                    spriteparts.forces[num].x =
                                        spriteparts.forces[num].x - spriteparts.velocity[num].x;
                                }
                                else if (legsanimation.id == prone.id)
                                {
                                    if (legsanimation.currframe > 24)
                                    {
                                        if (!(control.down && (control.left || control.right)))
                                        {
                                            spriteparts.velocity[num].x =
                                                spriteparts.velocity[num].x * standsurfacecoefx;
                                            spriteparts.velocity[num].y =
                                                spriteparts.velocity[num].y * standsurfacecoefy;
                                            spriteparts.forces[num].x = spriteparts.forces[num].x -
                                                                        spriteparts.velocity[num].x;
                                        }
                                    }
                                    else
                                    {
                                        spriteparts.velocity[num].x =
                                            spriteparts.velocity[num].x * surfacecoefx;
                                        spriteparts.velocity[num].y =
                                            spriteparts.velocity[num].y * surfacecoefy;
                                    }
                                }
                                else if (legsanimation.id == getup.id)
                                {
                                    spriteparts.velocity[num].x =
                                        spriteparts.velocity[num].x * surfacecoefx;
                                    spriteparts.velocity[num].y =
                                        spriteparts.velocity[num].y * surfacecoefy;
                                }
                                else if (legsanimation.id == pronemove.id)
                                {
                                    spriteparts.velocity[num].x =
                                        spriteparts.velocity[num].x * standsurfacecoefx;
                                    spriteparts.velocity[num].y =
                                        spriteparts.velocity[num].y * standsurfacecoefy;
                                }
                            }
                        }
                        else
                        {
                            if ((legsanimation.id == crouchrun.id) ||
                                (legsanimation.id == crouchrunback.id))
                            {
                                spriteparts.velocity[num].x =
                                    spriteparts.velocity[num].x * crouchmovesurfacecoefx;
                                spriteparts.velocity[num].y =
                                    spriteparts.velocity[num].y * crouchmovesurfacecoefy;
                            }
                            else
                            {
                                spriteparts.velocity[num].x =
                                    spriteparts.velocity[num].x * surfacecoefx;
                                spriteparts.velocity[num].y =
                                    spriteparts.velocity[num].y * surfacecoefy;
                            }
                        }
                    }
                    result = true;
                    return result;
                }
            }
        }
    }
    return result;
}

template <Config::Module M>
bool Sprite<M>::checkmapverticescollision(float x, float y, float r, bool hascollided)
{
    std::int32_t i, j, w, polytype;
    tvector2 pos, dir, vert;
    float d;
    std::int32_t rx, ry;
    bool teamcol;

#ifdef SERVER
    bool checkmapverticescollision_result;
    LogTraceG("TSprite.CheckMapVerticesCollision");
#endif

    bool result = false;
    pos.x = x;
    pos.y = y;

    // iterate through maps sector polygons
    rx = round((float)(pos.x) / map.sectorsdivision);
    ry = round((float)(pos.y) / map.sectorsdivision);
    if ((rx > -map.sectorsnum) && (rx < map.sectorsnum) && (ry > -map.sectorsnum) &&
        (ry < map.sectorsnum))
    {
        for (j = 1; j <= high(map.sectors[rx][ry].polys); j++)
        {
            w = map.sectors[rx][ry].polys[j];
            polytype = map.polytype[w];

            teamcol = teamcollides(w, player->team, false);

            if (((polytype != poly_type_doesnt) && (polytype != poly_type_only_bullets) &&
                 teamcol && (polytype != poly_type_only_flaggers) &&
                 (polytype != poly_type_not_flaggers)) ||
                ((holdedthing != 0) && (polytype == poly_type_only_flaggers)) ||
                ((holdedthing == 0) && (polytype == poly_type_not_flaggers)))
            {
                for (i = 1; i <= 3; i++)
                {
                    vert.x = map.polys[w].vertices[i].x;
                    vert.y = map.polys[w].vertices[i].y;
                    d = distance(vert, pos);
                    if (d < r) // collision
                    {
                        if (bgstate.backgroundtest(w))
                            continue;

                        if (!hascollided)
                            handlespecialpolytypes(polytype, pos);

                        dir = vec2subtract(pos, vert);
                        vec2normalize(dir, dir);
                        spriteparts.pos[num] = vec2add(spriteparts.pos[num], dir);

                        result = true;
                        return result;
                    } // D < R
                }     // i
            }         // if (PolyType...)
        }             // j
    }
    return result;
}

template <Config::Module M>
bool Sprite<M>::checkskeletonmapcollision(std::int32_t i, float x, float y)
{
    ZoneScopedN("CheckSkeletonMapCollision");
    std::int32_t j, w;
    std::int32_t b = 0;
    tvector2 pos, perp;
#ifndef SERVER
    tvector2 a;
#endif
    float d = 0.0;
    std::int32_t rx, ry;
    bool teamcol;

#ifdef SERVER
    bool checkskeletonmapcollision_result;
    LogTraceG("TSprite.CheckSkeletonMapCollision");
#endif

    bool result = false;
    pos.x = x - 1;
    pos.y = y + 4;

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

            teamcol = teamcollides(w, player->team, false);

            if (((map.polytype[w] != poly_type_doesnt) &&
                 (map.polytype[w] != poly_type_only_bullets) && teamcol &&
                 (map.polytype[w] != poly_type_only_flaggers) &&
                 (map.polytype[w] != poly_type_not_flaggers)) ||
                ((holdedthing != 0) && (map.polytype[w] == poly_type_only_flaggers)) ||
                ((holdedthing == 0) && (map.polytype[w] == poly_type_not_flaggers)))
            {
                if (map.pointinpolyedges(pos.x, pos.y, w))
                {
                    if (bgstate.backgroundtest(w))
                        continue;

                    perp = map.closestperpendicular(w, pos, d, b);

                    vec2normalize(perp, perp);
                    vec2scale(perp, perp, d);

                    skeleton.pos[i] = skeleton.oldpos[i];
                    skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);

#ifndef SERVER
                    a = vec2subtract(skeleton.pos[i], skeleton.oldpos[i]);

                    if ((fabs(a.y) > 0.8) && (deadcollidecount < 13))
                        playsound(sfx_bodyfall, skeleton.pos[i]);

                    if ((fabs(a.y) > 2.1) && (deadcollidecount < 4))
                        playsound(sfx_bonecrack, skeleton.pos[i]);
#endif

                    deadcollidecount += 1;

                    result = true;
                }
            }
        }
    }

    if (result)
    {
        pos.x = x;
        pos.y = y + 1;

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

                if ((map.polytype[w] != poly_type_doesnt) &&
                    (map.polytype[w] != poly_type_only_bullets))
                {
                    if (map.pointinpolyedges(pos.x, pos.y, w))
                    {
                        if (bgstate.backgroundtest(w))
                            continue;

                        perp = map.closestperpendicular(w, pos, d, b);

                        vec2normalize(perp, perp);
                        vec2scale(perp, perp, d);

                        skeleton.pos[i] = skeleton.oldpos[i];
                        skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);

                        result = true;
                    }
                }
            }
        }
    }
    return result;
}

template <Config::Module M>
void Sprite<M>::handlespecialpolytypes(std::int32_t polytype, const tvector2 &pos)
{
    tvector2 a, b;

    switch (polytype)
    {
    case poly_type_deadly: {
#ifdef SERVER
        healthhit(50 + GetHealth(), num, 12, -1, spriteparts.velocity[num]);
#endif
    }
    break;
    case poly_type_bloody_deadly: {
#ifdef SERVER
        healthhit(450 + GetHealth(), num, 12, -1, spriteparts.velocity[num]);
#endif
    }
    break;
    case poly_type_hurts:
    case poly_type_lava: // hurts
    {
        if (!deadmeat)
        {
            if (Random(10) == 0)
            {
#ifdef SERVER
                Health = Health - 5;
#else
                if (polytype == poly_type_hurts)
                    playsound(sfx_arg, spriteparts.pos[num]);
                else if (polytype == poly_type_lava)
                    playsound(sfx_lava, spriteparts.pos[num]);
#endif
            }
#ifdef SERVER
            if (GetHealth() < 1)
                healthhit(10, num, 12, -1, spriteparts.velocity[num]);
#endif
        }

        // lava
        if (Random(3) == 0)
            if (polytype == poly_type_lava)
            {
                a = pos;
                a.y = a.y - 3.0;
#ifndef SERVER
                createspark(a, vector2(0, -1.3), 36, num, 40);
#endif

                if (Random(3) == 0)
                {
                    b.x = -spriteparts.velocity[num].x;
                    b.y = -spriteparts.velocity[num].y;
                    createbullet(a, b, guns[flamer].num, num, 255, guns[flamer].hitmultiply, false,
                                 true);
                }
            }
    }
    break;
    case poly_type_regenerates: {
        if (Health < starthealth)
            if (maintickcounter % 12 == 0)
            {
#ifdef SERVER
                healthhit(-2, num, 12, -1, spriteparts.velocity[num]);
#else
                playsound(sfx_regenerate, spriteparts.pos[num]);
#endif
            }
    }
    break;
    case poly_type_explodes: {
        if (!deadmeat)
        {
            a = pos;
            a.y = a.y - 3.0;
            b.x = 0;
            b.y = 0;
#ifndef SERVER
            createspark(a, vector2(0, -1.3), 36, num, 40);
#else
            servercreatebullet(a, b, guns[m79].num, num, 255, guns[m79].hitmultiply, true);
            healthhit(4000, num, 12, -1, spriteparts.velocity[num]);
            Health = -600;
#endif
        }
    }
    break;
    case poly_type_hurts_flaggers: {
        if (!deadmeat && (holdedthing > 0) && (thing[holdedthing].style < object_ussocom))
            if (Random(10) == 0)
            {
#ifdef SERVER
                Health = Health - 10;
#else
                playsound(sfx_arg, spriteparts.pos[num]);
#endif
            }
#ifdef SERVER
        if (Health < 1)
            healthhit(10, num, 12, -1, spriteparts.velocity[num]);
#endif
    }
    break;
    }
}

template <Config::Module M>
bool BackgroundState<M>::backgroundtest(std::uint64_t poly)
{
    std::uint8_t polytype;

    bool result;
    result = false;

    polytype = map.polytype[poly];

    if ((polytype == poly_type_background) && (backgroundstatus == background_transition))
    {
        backgroundtestresult = true;
        backgroundpoly = poly;
        backgroundstatus = background_transition;

        result = true;
    }
    else if (polytype == poly_type_background_transition)
    {
        backgroundtestresult = true;
        if (backgroundstatus == background_normal)
            backgroundstatus = background_transition;

        result = true;
    }
    return result;
}

template <Config::Module M>
void BackgroundState<M>::backgroundtestbigpolycenter(tvector2 pos)
{
    if (backgroundstatus == background_transition)
    {
        if (backgroundpoly == background_poly_unknown)
        {
            backgroundpoly = backgroundfindcurrentpoly(pos);
            if (backgroundpoly != background_poly_none)
                backgroundtestresult = true;
        }
        else if ((backgroundpoly != background_poly_none) and
                 (map.pointinpoly(pos, map.polys[backgroundpoly])))
        {
            backgroundtestresult = true;
        }
    }
}

template <Config::Module M>
std::int16_t BackgroundState<M>::backgroundfindcurrentpoly(tvector2 pos)
{
    std::int32_t i;

    std::int16_t result;
    for (i = 1; i <= map.backpolycount; i++)
    {
        if (map.pointinpoly(pos, *map.backpolys[i]))
        {
            result = i;
            return result;
        }
    }
    result = background_poly_none;
    return result;
}

template <Config::Module M>
void BackgroundState<M>::backgroundtestprepare()
{
    backgroundtestresult = false;
}

template <Config::Module M>
void BackgroundState<M>::backgroundtestreset()
{
    if (!backgroundtestresult)
    {
        backgroundstatus = background_normal;
        backgroundpoly = background_poly_none;
    }
}

template <Config::Module M>
void Sprite<M>::applyweaponbynum(std::uint8_t wnum, std::uint8_t gun, std::int32_t ammo,
                                 bool restoreprimarystate)
{
    std::int32_t weaponindex;

#ifdef SERVER
    LogTraceG("TSprite.ApplyWeaponByNum");

    if (player->knifewarnings > 0)
        player->knifewarnings -= 1;
#endif

    if (restoreprimarystate && (gun == 2))
    {
        secondaryweapon = weapon;
    }
    else
    {
        weaponindex = weaponnumtoindex(wnum, guns);
        if (gun == 1)
            weapon = guns[weaponindex];
        else
            secondaryweapon = guns[weaponindex];
    }

    if (ammo > -1)
        weapon.ammocount = ammo;

#ifndef SERVER
    weapon.startuptimecount = weapon.startuptime;
#endif

#ifdef SERVER
    if (weapon.num == guns[knife].num)
        knifecan[num] = true;
#endif

    if (wnum != guns[noweapon].num)
    {
        lastweaponhm = weapon.hitmultiply;
        lastweaponstyle = weapon.bulletstyle;
        lastweaponspeed = weapon.speed;
        lastweaponfire = weapon.fireinterval;
        lastweaponreload = weapon.reloadtime;
    }

#ifdef SERVER
    LogTraceG("TSprite.ApplyWeaponByNum 2");
#endif
}

template <Config::Module M>
void Sprite<M>::healthhit(float amount, std::int32_t who, std::int32_t where, std::int32_t what,
                          const tvector2 &impact)
{
    tvector2 t;
    float hm;
    std::int32_t j;
#ifndef SERVER
    std::string s;
#endif

#ifdef SERVER
    LogTraceG("TSprite.HealthHit");
#endif
    // Friendly Fire
    if ((!CVar::sv_friendlyfire) && isnotsolo() && isinsameteam(sprite[who]) && (num != who))
        return;

#ifdef SERVER
    if (sprite[who].isspectator() && (sprite[who].player->controlmethod == human))
        return;
#endif

    if (bonusstyle == bonus_flamegod)
        return;

    // no health hit if someone is rambo
    if (CVar::sv_gamemode == gamestyle_rambo)
        if (num != who)
        {
            for (j = 1; j <= max_players; j++)
                if ((sprite[j].active) && (who != j) && (num != j))
                    if ((sprite[j].weapon.num == guns[bow].num) ||
                        (sprite[j].weapon.num == guns[bow2].num))
                        return;
        }

    hm = amount;

    if (vest > 0)
    {
        hm = round(0.33 * amount);
        vest = vest - hm;
        hm = round(0.25 * amount);
    }

    if ((sprite[who].bonusstyle == bonus_berserker)
#ifndef SERVER
        && (who != num))
#else
    )
#endif
        hm = 4 * amount;

#ifdef SCRIPT
    if (!this->deadmeat)
        hm = scrptdispatcher.onplayerdamage(this->num, who, hm, (std::uint8_t)(what));
#endif

    Health = Health - hm;

#ifndef SERVER
    if ((what > 0) && (this->num != mysprite))
    {
        switch (bullet[what].style)
        {
        case bullet_style_fragnade:
            s = "Grenade";
            break;
        case bullet_style_cluster:
            s = "Clusters";
            break;
        case bullet_style_m2:
            s = "Stationary gun";
            break;
        default:
            s = weaponnamebynum(bullet[what].ownerweapon, guns);
        }
    }
    else
    {
        s = "Selfkill";
    }

    if (!this->deadmeat && (what > 0))
        // Check to prevent one AoE explosion counting as multiple hits on one bullet
        if ((who != this->num) && (!bullet[what].hashit) && (who == mysprite))
            for (j = 1; j <= 20; j++)
            {
                if (wepstats[j].name == s)
                {
                    wepstats[j].hits = wepstats[j].hits + 1;
                    bullet[what].hashit = true;
                }
            }
#endif

    // helmet fall off
    if ((Health < helmetfallhealth) && (wearhelmet == 1) && (where == 12) &&
        (weapon.num != guns[bow].num) && (weapon.num != guns[bow2].num) && (player->headcap > 0))
    {
        wearhelmet = 0;
#ifndef SERVER
        createspark(skeleton.pos[12], spriteparts.velocity[num], 6, num, 198);
        playsound(sfx_headchop, sprite[num].skeleton.pos[where]);
#endif
    }

    // safety precautions
    if (Health < (brutaldeathhealth - 1))
        Health = brutaldeathhealth;
    if (Health > starthealth)
        Health = starthealth;

    // death!
    t = impact;
    if ((Health < 1) && (Health > headchopdeathhealth))
        die(normal_death, who, where, what, t);
    else if ((Health < headchopdeathhealth + 1) && (Health > brutaldeathhealth))
        die(headchop_death, who, where, what, t);
    else if (Health < brutaldeathhealth + 1)
        die(brutal_death, who, where, what, t);

    brain.targetnum = who;

    // Bot Chat
#ifdef SERVER
    if (CVar::bots_chat)
        if ((GetHealth() < hurt_health) && (player->controlmethod == bot))
        {
            if (Random(10 * brain.chatfreq) == 0)
                serversendstringmessage((brain.chatlowhealth), all_players, num, msgtype_pub);
        }
#endif
}

template <Config::Module M>
void Sprite<M>::freecontrols()
{
#ifdef SERVER
    LogTraceG("TSprite.FreeControls");
#endif

    control.left = false;
    control.right = false;
    control.up = false;
    control.down = false;
    control.fire = false;
    control.jetpack = false;
    control.thrownade = false;
    control.changeweapon = false;
    control.throwweapon = false;
    control.reload = false;
    control.prone = false;
#ifndef SERVER
    control.mousedist = 150;
#endif
    control.flagthrow = false;
}

template <Config::Module M>
void Sprite<M>::checkoutofbounds()
{
    std::int32_t bound;

#ifdef SERVER
    LogTraceG("TSprite.CheckOutOfBounds");
#endif

    if (survivalendround)
        return;

    bound = map.sectorsnum * map.sectorsdivision - 50;
    tvector2 &spritepartspos = spriteparts.pos[num];

    if ((fabs(spritepartspos.x) > bound) || (fabs(spritepartspos.y) > bound))
    {
#ifndef SERVER
        randomizestart(spriteparts.pos[num], player->team);
#endif
        respawn();
    }
}

template <Config::Module M>
void Sprite<M>::checkskeletonoutofbounds()
{
    std::int32_t i;
    std::int32_t bound;
    struct tvector2 *skeletonpos;

#ifdef SERVER
    LogTraceG("TSprite.CheckSkeletonOutOfBounds");
#endif

    if (survivalendround)
        return;

    bound = map.sectorsnum * map.sectorsdivision - 50;

    for (i = 1; i <= 20; i++)
    {
        skeletonpos = &skeleton.pos[i];

        if ((fabs(skeletonpos->x) > bound) || (fabs(skeletonpos->y) > bound))
        {
#ifndef SERVER
            randomizestart(spriteparts.pos[num], player->team);
#endif
            respawn();
            break;
        }
    }
}

template <Config::Module M>
void Sprite<M>::respawn()
{
    std::int32_t j;
#ifdef SERVER
    std::int32_t k;
    std::int32_t weaponindex;
    std::int16_t favweaponindex;
#endif
    std::int32_t secwep;
    bool deadmeatbeforerespawn;
    bool survivalcheckendround;

#ifdef SERVER
    LogTraceG("TSprite.Respawn");
#endif
    if (CVar::sv_survivalmode_clearweapons)
        if (survivalendround && !weaponscleaned)
        {
            for (j = 1; j <= max_things; j++)
            {
                if (thing[j].active &&
                    ((isInRange((std::int32_t)thing[j].style, object_ussocom, object_minigun)) ||
                     (isInRange((std::int32_t)thing[j].style, object_combat_knife, object_law))))
                {
                    thing[j].kill();
                }
            }
            weaponscleaned = true;
        }

    if (isspectator())
        return;

#ifndef SERVER
    if ((player->name == "") or (player->demoplayer))
        return;

    if (num == mysprite)
        playsound(sfx_wermusic, spriteparts.pos[num]);
#endif

#ifdef SERVER
    randomizestart(spriteparts.pos[num], player->team);
#endif

#ifdef SCRIPT
    spriteparts.pos[num] = scrptdispatcher.onbeforeplayerrespawn(num);
#endif

    deadmeatbeforerespawn = deadmeat;
    deadmeat = false;
    halfdead = false;
    Health = starthealth;
    wearhelmet = 1;

    if (player->headcap == 0)
        wearhelmet = 0;
    skeleton.constraints = gostekskeleton.constraints;
    spriteparts.velocity[num].x = 0;
    spriteparts.velocity[num].y = 0;
    spriteparts.forces[num].x = 0;
    spriteparts.forces[num].y = 0;
    jetscount = map.startjet;
    jetscountprev = map.startjet;
    ceasefirecounter = ceasefiretime;
    if (CVar::sv_advancemode)
        ceasefirecounter = ceasefirecounter * 3;
    brain.pissedoff = 0;
    brain.gothing = false;
    vest = 0;
    bonusstyle = bonus_none;
    bonustime = 0;
    multikills = 0;
    multikilltime = 0;
    tertiaryweapon = guns[fraggrenade];
    tertiaryweapon.ammocount = CVar::sv_maxgrenades / 2;
    hascigar = 0;
    canmercy = true;
    idletime = default_idletime;
    idlerandom = -1;

#ifndef SERVER
    if (num == mysprite)
    {
        camerafollowsprite = mysprite;
        grenadeeffecttimer = 0;
        hitspraycounter = 0;
    }
#endif

    bodyanimation = stand;
    legsanimation = stand;
    position = pos_stand;
    onfire = 0;
    deadcollidecount = 0;
    brain.currentwaypoint = 0;
    respawncounter = 0;
    player->camera = num;
    onground = false;
    ongroundlastframe = false;
    ongroundpermanent = false;

    bgstate.backgroundstatus = background_transition;
    bgstate.backgroundpoly = background_poly_unknown;

#ifdef SERVER
    bullettime[num] = maintickcounter - 10;
    grenadetime[num] = maintickcounter - 10;
    knifecan[num] = true;
#endif

    if ((holdedthing > 0) && (holdedthing < max_things + 1))
    {
        if (thing[holdedthing].style != object_parachute)
            thing[holdedthing].respawn();
        else
            thing[holdedthing].kill();
    }

    holdedthing = 0;

#ifndef SERVER
    if (selweapon > 0)
        if (weaponsel[num][selweapon] == 0)
            selweapon = 0;
#endif

    weapon = guns[noweapon];

    if (selweapon > 0)
#ifndef SERVER
        if ((weaponactive[selweapon] == 1) && (weaponsel[num][selweapon] == 1))
        {
#endif
            applyweaponbynum(selweapon, 1);
#ifndef SERVER
            if (num == mysprite)
                clientspritesnapshot();
        }
#endif

    secwep = player->secwep + 1;

    if ((secwep >= 1) && (secwep <= secondary_weapons) &&
        (weaponactive[primary_weapons + secwep] == 1) &&
        (weaponsel[num][primary_weapons + secwep] == 1))
        secondaryweapon = guns[primary_weapons + secwep];
    else
        secondaryweapon = guns[noweapon];

#ifdef SERVER
    if (CVar::sv_advancemode)
#endif
        if ((selweapon > 0) && ((weaponactive[selweapon] == 0) || (weaponsel[num][selweapon] == 0)))
        {
            weapon = secondaryweapon;
            secondaryweapon = guns[noweapon];
        }

#ifdef SERVER
    if (player->controlmethod == bot)
    {
        brain.currentwaypoint = 0;

        if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
            (CVar::sv_gamemode == gamestyle_htf))
            brain.pathnum = player->team;

#ifdef SERVER
        LogTraceG("TSprite.Respawn 2");
#endif

        // randomize bot weapon
        if ((brain.favweapon != guns[noweapon].num) && (brain.favweapon != guns[knife].num) &&
            (brain.favweapon != guns[chainsaw].num) && (brain.favweapon != guns[law].num) && !dummy)
        {
            if (((weaponactive[1] == 1) && (weaponsel[num][1] == 1)) ||
                ((weaponactive[2] == 1) && (weaponsel[num][2] == 1)) ||
                ((weaponactive[3] == 1) && (weaponsel[num][3] == 1)) ||
                ((weaponactive[4] == 1) && (weaponsel[num][4] == 1)) ||
                ((weaponactive[5] == 1) && (weaponsel[num][5] == 1)) ||
                ((weaponactive[6] == 1) && (weaponsel[num][6] == 1)) ||
                ((weaponactive[7] == 1) && (weaponsel[num][7] == 1)) ||
                ((weaponactive[8] == 1) && (weaponsel[num][8] == 1)) ||
                ((weaponactive[9] == 1) && (weaponsel[num][9] == 1)) ||
                ((weaponactive[10] == 1) && (weaponsel[num][10] == 1)))
            {
                do
                {
                    if (Random(2) == 0)
                        applyweaponbynum(brain.favweapon, 1);
                    else
                    {
                        k = Random(9) + 1;
                        weapon = guns[k];
                    }

                    if ((weaponsingame < 6) && (weaponactive[minigun] == 1) &&
                        (weaponsel[num][minigun] == 1))
                        weapon = guns[minigun];

                    if (CVar::sv_advancemode)
                    {
                        for (j = 1; j <= primary_weapons; j++)
                            if (weaponsel[num][j] == 1)
                                break;
                        applyweaponbynum(j, 1);
                    }
                } while (!((weaponactive[weapon.num] == 1) or (CVar::sv_advancemode)));
            }
        }

        if ((weaponactive[1] == 0) && (weaponactive[2] == 0) && (weaponactive[3] == 0) &&
            (weaponactive[4] == 0) && (weaponactive[5] == 0) && (weaponactive[6] == 0) &&
            (weaponactive[7] == 0) && (weaponactive[8] == 0) && (weaponactive[9] == 0) &&
            (weaponactive[10] == 0) && (weaponsel[num][1] == 0) && (weaponsel[num][2] == 0) &&
            (weaponsel[num][3] == 0) && (weaponsel[num][4] == 0) && (weaponsel[num][5] == 0) &&
            (weaponsel[num][6] == 0) && (weaponsel[num][7] == 0) && (weaponsel[num][8] == 0) &&
            (weaponsel[num][9] == 0) && (weaponsel[num][10] == 0))
            weapon = guns[noweapon];

        favweaponindex = weaponnumtoindex(brain.favweapon, guns);
        if ((brain.favweapon == noweapon_num) or issecondaryweaponindex(favweaponindex) or dummy)
        {
            weapon = guns[favweaponindex];
            secondaryweapon = guns[noweapon];
        }

        if (brain.use != 255)
        {
            if (brain.use == 1)
            {
                idletime = 0;
                idlerandom = 1;
            }
            if (brain.use == 2)
            {
                idletime = 0;
                idlerandom = 0;
            }
        }

        // Disarm bot if the primary weapon isn't allowed and selectable
        weaponindex = weapon.num;
        if ((weaponindex >= 1) && (weaponindex <= primary_weapons))
            if ((weaponactive[weaponindex] == 0) || (weaponsel[num][weaponindex] == 0))
                weapon = guns[noweapon];
    }
#endif

    if (weaponsingame == 0)
        weapon = guns[noweapon];

    parachute(spriteparts.pos[num]);

#ifdef SCRIPT
    scrptdispatcher.onafterplayerrespawn(num);
#endif

#ifdef SERVER
    resetspriteoldpos();
#endif

    // clear push wait list
    for (j = 0; j <= max_pushtick; j++)
    {
        nextpush[j].x = 0;
        nextpush[j].y = 0;
    }

#ifndef SERVER
    // Spawn sound
    if (num != mysprite)
        playsound(sfx_spawn, spriteparts.pos[num]);

    // spawn spark
    createspark(spriteparts.pos[num], spriteparts.velocity[num], 25, num, 33);
#endif

    freecontrols();

    legsapplyanimation(stand, 1);
    bodyapplyanimation(stand, 1);

    if (canrespawn(deadmeatbeforerespawn))
    {
        if (survivalendround)
        {
            survivalcheckendround = false;
            for (j = 1; j <= max_sprites; j++)
                if (sprite[j].active)
                    if (sprite[j].player->team != team_spectator)
                        if (sprite[j].deadmeat)
                        {
                            survivalcheckendround = true;
                            break;
                        }
            survivalendround = survivalcheckendround;
        }
    }
    else
    {
        // CheckSkeletonOutOfBounds would trigger infinitely
        // Respawn if this is not done
        for (j = 1; j <= 20; j++)
        {
            skeleton.pos[j].x = spriteparts.pos[num].x;
            skeleton.pos[j].y = spriteparts.pos[num].y;
            skeleton.oldpos[j] = skeleton.pos[j];
        }
        // TODO: Fix this shouldn't change wepstats
        die(normal_death, num, 1, -1, skeleton.pos[12]);
        player->deaths -= 1;
    }
}

#ifdef SERVER
template <Config::Module M>
void Sprite<M>::resetspriteoldpos()
{
    std::int32_t i;

    for (i = max_oldpos; i >= 1; i--)
        oldspritepos[num][i] = spriteparts.pos[num];
}
#endif

template <Config::Module M>
void Sprite<M>::parachute(tvector2 &a)
{
    tvector2 b;
    std::int32_t n, i;
    float d = 0.0;

#ifdef SERVER
    LogTraceG("Parachute");
#endif
    if (holdedthing > 0)
        return;
    if (isspectator())
        return;

    for (i = 1; i <= max_things; i++)
    {
        if (thing[i].holdingsprite == num)
        {
            thing[i].holdingsprite = 0;
            thing[i].kill();
        }
    }

    b = a;
    b.y = b.y + para_distance;
    if (!map.raycast(a, b, d, para_distance + 50, true, false, false, false, player->team))
        if (d > para_distance - 10)
        {
            a.y = a.y + 70;
            n = creatething(a, num, object_parachute, 255);
            thing[n].holdingsprite = num;
#ifndef SERVER
            thing[n].color = player->shirtcolor;
#endif
            holdedthing = n;
        }
}
#ifdef SERVER
template <Config::Module M>
void Sprite<M>::changeteam(std::int32_t team, bool adminchange, std::uint8_t jointype)
#else
template <Config::Module M>
void Sprite<M>::changeteam(std::int32_t team)
#endif
{
    std::int32_t i;
    tvector2 a, b;
#ifdef SERVER
    std::vector<std::int32_t> teamscount{6};
#endif
#ifdef SCRIPT
    std::uint8_t spriteoldteam;
#endif

    if (team > team_spectator)
        return;

    if (active)
    {
#ifdef SERVER
        for (i = team_none; i <= team_spectator; i++)
            teamscount[i] = 0;

        for (i = 1; i <= max_sprites; i++)
            if (sprite[i].active && sprite[i].isnotspectator())
                teamscount[sprite[i].player->team] = teamscount[sprite[i].player->team] + 1;

        // Check for uneven teams
        if ((CVar::sv_balanceteams) && (adminchange == false))
        {
            if (isspectator() && (team == findlowestteam(teamscount)))
                ;
            else if ((teamscount[team] >= teamscount[player->team]) && (team < team_spectator))
            {
                const char *teams[] = {"Alpha", "Bravo", "Charlie", "Delta"};
                writeconsole(num, std::string(choose(team - 1, teams)) + " team is full",
                             rgb(0, 0, 255));
                return;
            }
        }

        if ((teamscount[team_spectator] >= CVar::sv_maxspectators) && (team == team_spectator) &&
            (adminchange == false))
        {
            writeconsole(num, "Spectators are full", rgb(0, 0, 255));
            return;
        }

        if ((CVar::sv_gamemode != gamestyle_teammatch) &&
            ((team == team_charlie) || (team == team_delta)) && (adminchange == false))
            return;

#ifdef SCRIPT
        spriteoldteam = player->team;
        team = scrptdispatcher.onbeforejointeam(num, team, spriteoldteam);
        if ((team < team_none) || (team > team_spectator))
            return;
#endif
#endif

        dropweapon();

        player->team = team;
#ifdef SERVER
        player->applyshirtcolorfromteam();
#endif
        num = createsprite(a, b, 1, num, player, isplayerobjectowner);

        if (sprite[num].holdedthing > 0)
            if (thing[sprite[num].holdedthing].style < object_ussocom)
            {
                thing[sprite[num].holdedthing].respawn();
                sprite[num].holdedthing = 0;
            }

        for (i = 1; i <= max_things; i++)
        {
            if (thing[i].holdingsprite == num)
                thing[i].respawn();
        }
        sprite[num].respawn();

#ifdef SERVER
        bullettime[num] = maintickcounter - 10;
        grenadetime[num] = maintickcounter - 10;
        knifecan[num] = true;

        if (!sprite[num].player->demoplayer)
            serversendnewplayerinfo(num, jointype);
        sortplayers();
        LogDebugG("Player switched teams");
#endif

        switch (player->team)
        {
#ifdef SERVER
        case team_none:
            GetMainConsole().console(player->name + " has joined the game.", enter_message_color);
            break;
        case team_alpha:
            GetMainConsole().console(player->name + " has joined alpha team.",
                                     alphaj_message_color);
            break;
        case team_bravo:
            GetMainConsole().console(player->name + " has joined bravo team.",
                                     bravoj_message_color);
            break;
        case team_charlie:
            GetMainConsole().console(player->name + " has joined charlie team.",
                                     charliej_message_color);
            break;
        case team_delta:
            GetMainConsole().console(player->name + " has joined delta team.",
                                     deltaj_message_color);
            break;
        case team_spectator:
            GetMainConsole().console(player->name + " has joined spectators.",
                                     deltaj_message_color);
            break;
#else
        case team_none:
            GetMainConsole().console(wideformat(_("%s has joined the game."), player->name),
                                     enter_message_color);
            break;
        case team_alpha:
            GetMainConsole().console(wideformat(_("%s has joined alpha team"), player->name),
                                     alphaj_message_color);
            break;
        case team_bravo:
            GetMainConsole().console(wideformat(_("%s has joined bravo team"), player->name),
                                     bravoj_message_color);
            break;
        case team_charlie:
            GetMainConsole().console(wideformat(_("%s has joined charlie team"), player->name),
                                     charliej_message_color);
            break;
        case team_delta:
            GetMainConsole().console(wideformat(_("%s has joined delta team"), player->name),
                                     deltaj_message_color);
            break;
#endif
        }

        // prevent players from joining alive midround in survival mode
        if ((CVar::sv_survivalmode) && (player->team != team_spectator))
        {
            // TODO: Fix this shouldn't change wepstats
            sprite[num].healthhit(4000, num, 1, 1, a);
            sprite[num].player->deaths -= 1;
        }
#ifndef SERVER
        if (num == mysprite)
        {
            if (player->team == team_spectator)
            {
                camerafollowsprite = 0;
                camerafollowsprite = getcameratarget();
                gamemenushow(limbomenu, false);
            }
            else
                camerafollowsprite = mysprite;
        }
#endif

        // Check if map change is in progress
#ifdef SERVER
        if ((mapchangecounter > -60) && (mapchangecounter < 99999999))
            servermapchange(num);
#endif

#ifdef SCRIPT
        scrptdispatcher.onjointeam(sprite[num].num, team, spriteoldteam, false);
#endif
    }
}

// SPRITE FIRE
template <Config::Module M>
void Sprite<M>::fire()
{
    tvector2 a, b, d, m;
    tvector2 aimdirection;
    std::int32_t i, bn;
    float inaccuracy;
    float maxdeviation;
    tvector2 collisiontestperpendicular;
    tvector2 bnorm;
#ifndef SERVER
    tvector2 muzzlesmokevector;
    tvector2 c;
    float rc;
    bool col;
#endif

    bn = 0;
    inaccuracy = 0;
#ifdef SERVER
    LogTraceG("SpriteFire");
#endif

    // Create a normalized directional vector
    if ((weapon.bulletstyle == bullet_style_knife) || (bodyanimation.id == mercy.id) ||
        (bodyanimation.id == mercy2.id))
        aimdirection = gethandsaimdirection();
    else
        aimdirection = getcursoraimdirection();

    b = aimdirection;

    a.x = skeleton.pos[15].x - (b.x * 4);
    a.y = skeleton.pos[15].y - (b.y * 4) - 2;

#ifndef SERVER
    // TODO(skoskav): Make bink and self-bink sprite-specific so bots can also use it
    if (num == mysprite)
    {
        // Bink && self-bink
        if (hitspraycounter > 0)
        {
            inaccuracy = inaccuracy + hitspraycounter * 0.01;
        }
    }
#endif

    // Moveacc
    inaccuracy = inaccuracy + getmoveacc();

    // Bullet spread
    if ((weapon.num != guns[eagle].num) && (weapon.num != guns[spas12].num) &&
        (weapon.bulletstyle != bullet_style_shotgun))
    {
        if (weapon.bulletspread > 0)
        {
            if ((legsanimation.id == pronemove.id) ||
                ((legsanimation.id == prone.id) && (legsanimation.currframe > 23)))
            {
                inaccuracy = inaccuracy + weapon.bulletspread / 1.625;
            }
            else if ((legsanimation.id == crouchrun.id) || (legsanimation.id == crouchrunback.id) ||
                     ((legsanimation.id == crouch.id) && (legsanimation.currframe > 13)))
            {
                inaccuracy = inaccuracy + weapon.bulletspread / 1.3;
            }
            else
            {
                inaccuracy = inaccuracy + weapon.bulletspread;
            }
        }
    }

    // FIXME(skoskav): Inaccuracy decreased due to altered way of acquiring the directional vector.
    // This should be solved more elegantly.
    inaccuracy = inaccuracy * 0.25;

    if (inaccuracy > max_inaccuracy)
        inaccuracy = max_inaccuracy;

    // Calculate the maximum bullet deviation between 0 and MAX_INACCURACY.
    // The scaling is modeled after Sin(x) where x = 0 -> Pi/2 to gracefully reach
    // the maximum. Then multiply by a float between -1.0 and 1.0.
    maxdeviation =
        max_inaccuracy * sin(((float)(inaccuracy) / max_inaccuracy) * (std::numbers::pi / 2));
    d.x = (float)(Random() * 2 - 1) * maxdeviation;
    d.y = (float)(Random() * 2 - 1) * maxdeviation;

    // Add inaccuracies to directional vector and re-normalize
    b = vec2add(b, d);
    vec2normalize(b, b);

    // Multiply with the weapon speed
    vec2scale(b, b, weapon.speed);

    // Add some of the player's velocity to the bullet
    vec2scale(m, spriteparts.velocity[num], weapon.inheritedvelocity);
    b = vec2add(b, m);

    // Check for immediate collision (could just be head in polygon), if so then
    // offset the bullet origin downward slightly
    if (map.collisiontest(a, collisiontestperpendicular))
    {
        a.y = a.y + 2.5;
    }

    if (((weapon.num != guns[eagle].num) && (weapon.num != guns[spas12].num) &&
         (weapon.num != guns[flamer].num) && (weapon.num != guns[noweapon].num) &&
         (weapon.num != guns[knife].num) && (weapon.num != guns[chainsaw].num) &&
         (weapon.num != guns[law].num)) ||
        (bodyanimation.id == mercy.id) || (bodyanimation.id == mercy2.id))
    {
        bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
    }

#ifdef SERVER
    LogTraceG("SpriteFire 10");
#endif

    if (weapon.num == guns[eagle].num) // Eagles
    {
        bulletcount += 1;
        NotImplemented(NITag::OTHER);
#if 0
        randseed = bulletcount;
#endif

        d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
        d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;

        bn = createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, true, false, bulletcount);

        d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
        d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;

        vec2normalize(bnorm, b);
        a.x = a.x - sign(b.x) * fabs(bnorm.y) * 3.0;
        a.y = a.y + sign(b.y) * fabs(bnorm.x) * 3.0;

        createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, false, false);
    }

    if (weapon.bulletstyle == bullet_style_shotgun) // Shotgun
    {
        bulletcount += 1;
        NotImplemented(NITag::OTHER);
#if 0
        randseed = bulletcount;
#endif

        d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
        d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;

        bn = createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, true, false, bulletcount);

        for (i = 0; i <= 4; i++) // Remaining 5 pellets
        {
            d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
            d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;
            createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, false, false);
        }

        d.x = b.x * 0.0412;
        d.y = b.y * 0.041;
        spriteparts.velocity[num] = vec2subtract(spriteparts.velocity[num], d);
    }

#ifdef SERVER
    LogTraceG("SpriteFire 11");
#endif

    if (weapon.num == guns[minigun].num) // Minigun
    {
        if (control.jetpack && (jetscount > 0))
        {
            d.x = b.x * 0.0012;
            d.y = b.y * 0.0009;
        }
        else
        {
            d.x = b.x * 0.0082;
            d.y = b.y * 0.0078;
        }

        if (holdedthing > 0)
        {
            d.x = d.x * 0.5;
            d.y = d.y * 0.7;
        }
        d.x = d.x * 0.6;

        spriteparts.velocity[num] = vec2subtract(spriteparts.velocity[num], d);
    }

    if (weapon.num == guns[flamer].num) // Flamer
    {
        a.x = a.x + b.x * 2;
        a.y = a.y + b.y * 2;
        bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
#ifndef SERVER
        playsound(sfx_flamer, spriteparts.pos[num], gattlingsoundchannel);
#endif
    }

    if (weapon.num == guns[chainsaw].num) // Chainsaw
    {
        a.x = a.x + b.x * 2;
        a.y = a.y + b.y * 2;
        bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
    }

    if (weapon.num == guns[law].num)
    { // LAW
        if ((onground || ongroundpermanent || ongroundforlaw) &&
            (((legsanimation.id == crouch.id) && (legsanimation.currframe > 13)) ||
             (legsanimation.id == crouchrun.id) || (legsanimation.id == crouchrunback.id) ||
             ((legsanimation.id == prone.id) && (legsanimation.currframe > 23))))
        {
            bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
        }
        else
        {
            return;
        }
    }

    // Mercy animation
    if ((bodyanimation.id == mercy.id) || (bodyanimation.id == mercy2.id))
    {
        if ((bn > 0) && (bn < max_bullets + 1))
            if (bullet[bn].active)
            {
                a = bulletparts.velocity[bn];
                vec2normalize(bulletparts.velocity[bn], bulletparts.velocity[bn]);
                vec2scale(bulletparts.velocity[bn], bulletparts.velocity[bn], 70);
                bullet[bn].hit(2);
                bullet[bn].hit(9);
                // couple more - not sure why
                bullet[bn].hit(2);
                bullet[bn].hit(9);
                bullet[bn].hit(2);
                bullet[bn].hit(9);
                bullet[bn].hitbody = bullet[bn].owner;
                bulletparts.velocity[bn] = a;
            }
    }

    // Shouldn't we dec on server too?
#ifndef SERVER
    if (weapon.ammocount > 0)
        weapon.ammocount -= 1;
#endif

    if (weapon.num == guns[spas12].num)
        canautoreloadspas = false;

    weapon.fireintervalprev = weapon.fireinterval;
    weapon.fireintervalcount = weapon.fireinterval;

    fired = weapon.firestyle;

#ifndef SERVER
    // Spent bullet shell vectors
    c.x = spriteparts.velocity[num].x + direction * aimdirection.y * (Random(0) * 0.5 + 0.8);
    c.y = spriteparts.velocity[num].y - direction * aimdirection.x * (Random(0) * 0.5 + 0.8);
    a.x = skeleton.pos[15].x + 2 - direction * 0.015 * b.x;
    a.y = skeleton.pos[15].y - 2 - direction * 0.015 * b.y;

    col = map.collisiontest(a, b);

    if (CVar::r_maxsparks < (max_sparks - 10))
        if (Random(2) == 0)
            col = true;
#endif

#ifdef SERVER
    LogTraceG("SpriteFire 13");
#endif

    // play fire sound
    if (weapon.num == guns[ak74].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_ak74_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
#ifndef SERVER
        if (!col)
            createspark(a, c, 68, num, 255); // shell
#endif
    }
    if (weapon.num == guns[m249].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_m249_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
#ifndef SERVER
        if (!col)
            createspark(a, c, 72, num, 255); // shell
#endif
    }
    if (weapon.num == guns[ruger77].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_ruger77_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(recoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
#ifndef SERVER
        if (!col)
            createspark(a, c, 70, num, 255); // shell
#endif
    }
    if (weapon.num == guns[mp5].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_mp5_fire, spriteparts.pos[num]);
        a.x = skeleton.pos[15].x + 2 - 0.2 * b.x;
        a.y = skeleton.pos[15].y - 2 - 0.2 * b.y;
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
#ifndef SERVER
        if (!col)
            createspark(a, c, 67, num, 255); // shell
#endif
    }
    if (weapon.num == guns[spas12].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_spas12_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (position != pos_prone) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(shotgun, 1);

        // make sure firing interrupts reloading when prone
        if ((position == pos_prone) && (bodyanimation.id == reload.id))
            bodyanimation.currframe = bodyanimation.numframes;
    }
    if (weapon.num == guns[m79].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_m79_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (position != pos_prone) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
    }
    if (weapon.num == guns[eagle].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_deserteagle_fire, spriteparts.pos[num]);
        a.x = skeleton.pos[15].x + 3 - 0.17 * b.x;
        a.y = skeleton.pos[15].y - 2 - 0.15 * b.y;
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
#ifndef SERVER
        if (!col)
            createspark(a, c, 66, num, 255); // shell
        if (!col)
        {
            a.x = skeleton.pos[15].x - 3 - 0.25 * b.x;
            a.y = skeleton.pos[15].y - 3 - 0.3 * b.y;
            c.x =
                spriteparts.velocity[num].x + direction * aimdirection.y * (Random(0) * 0.5 + 0.8);
            c.y =
                spriteparts.velocity[num].y - direction * aimdirection.x * (Random(0) * 0.5 + 0.8);
            createspark(a, c, 66, num, 255); // shell
        }
#endif
    }
    if (weapon.num == guns[steyraug].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_steyraug_fire, spriteparts.pos[num]);
        if (!col)
            createspark(a, c, 69, num, 255); // shell
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
    }
    if (weapon.num == guns[barrett].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_barretm82_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (bodyanimation.id != getup.id) &&
            (bodyanimation.id != melee.id))
            bodyapplyanimation(barret, 1);
#ifndef SERVER
        if (!col)
            createspark(a, c, 71, num, 255); // shell
#endif
    }
    if (weapon.num == guns[minigun].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_minigun_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 2);
#ifndef SERVER
        if (!col)
            createspark(a, c, 73, num, 255); // shell
#endif
    }
    if (weapon.num == guns[colt].num)
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_colt1911_fire, spriteparts.pos[num]);
        a.x = skeleton.pos[15].x + 2 - 0.2 * b.x;
        a.y = skeleton.pos[15].y - 2 - 0.2 * b.y;
        if (!col)
            createspark(a, c, 65, num, 255); // shell
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
    }
    if ((weapon.num == guns[bow].num) || (weapon.num == guns[bow2].num))
    {
#ifndef SERVER
        if (bonusstyle != bonus_predator)
            playsound(sfx_bow_fire, spriteparts.pos[num]);
#endif
        if ((bodyanimation.id != throw_.id) && (position == pos_stand) &&
            (bodyanimation.id != getup.id) && (bodyanimation.id != melee.id))
            bodyapplyanimation(smallrecoil, 1);
        if (position == pos_crouch)
        {
            if (bodyanimation.id == handsupaim.id)
                bodyapplyanimation(handsuprecoil, 1);
            else
                bodyapplyanimation(aimrecoil, 1);
        }
    }
#ifndef SERVER
    if (weapon.num == guns[law].num)
    {
        if (bonusstyle != bonus_predator)
            playsound(sfx_law, spriteparts.pos[num]);
    }

    // smoke from muzzle
    muzzlesmokevector = b;
    vec2scale(muzzlesmokevector, muzzlesmokevector, 0.5);
    a = vec2add(a, muzzlesmokevector);
    vec2scale(muzzlesmokevector, muzzlesmokevector, 0.2);
    createspark(a, muzzlesmokevector, 35, num, 10);
#endif

    if (burstcount < 255)
        burstcount += 1;

        // TODO(skoskav): Make bink and self-bink sprite-specific so bots can also use it
#ifndef SERVER
    if (num == mysprite)
    {
        // Increase self-bink for next shot
        if (weapon.bink < 0)
        {
            if ((legsanimation.id == crouch.id) || (legsanimation.id == crouchrun.id) ||
                (legsanimation.id == crouchrunback.id) || (legsanimation.id == prone.id) ||
                (legsanimation.id == pronemove.id))
                hitspraycounter = calculatebink(hitspraycounter, round((float)(-weapon.bink) / 2));
            else
            {
                hitspraycounter = calculatebink(hitspraycounter, -weapon.bink);
            }
        }
    }

    // Screen shake
    if (((mysprite > 0) && (camerafollowsprite != 0)) and
        (!((num != mysprite) && !CVar::cl_screenshake)))
    {
        if (weapon.num != guns[chainsaw].num)
        {
            if (pointvisible(spriteparts.pos[num].x, spriteparts.pos[num].y, camerafollowsprite))
            {
                if ((weapon.num == guns[m249].num) || (weapon.num == guns[spas12].num) ||
                    (weapon.num == guns[barrett].num) || (weapon.num == guns[minigun].num))
                {
                    camerax = camerax - 3 + Random(7);
                    cameray = cameray - 3 + Random(7);
                }
                else
                {
                    camerax = camerax - 1 + Random(3);
                    cameray = cameray - 1 + Random(3);
                }
            }
        }
    }

    // Recoil!
    if (mysprite == num)
    {
        rc = (float)(burstcount) / 10.f;
        rc = rc * (float)weapon.recoil;

        // less recoil on crouch
        if (onground)
        {
            if ((legsanimation.id == crouch.id) && (legsanimation.currframe > 13))
                rc = (float)(rc) / 2;

            if ((legsanimation.id == prone.id) && (legsanimation.currframe > 23))
                rc = (float)(rc) / 3;
        }

        if (rc > 0)
        {
            rc = sin(degtorad(((float)(weapon.speed * weapon.fireinterval) / 364) * rc));

            calculaterecoil(gamewidthhalf - camerax + spriteparts.pos[num].x,
                            gameheighthalf - cameray + spriteparts.pos[num].y, mx, my,
                            -(pi / 1) * rc);
        }
    }
#endif
}

template <Config::Module M>
void Sprite<M>::throwflag()
{
    std::int32_t i, j;
    tvector2 b;
    float d = 0.0;
    tvector2 cursordirection;
    tvector2 boffset, bperp;
    tvector2 lookpoint1, lookpoint2, lookpoint3;
    tvector2 newposdiff;
    tvector2 futurepoint1, futurepoint2, futurepoint3, futurepoint4;

    if ((bodyanimation.id != roll.id) && (bodyanimation.id != rollback.id))
    {
        if (control.flagthrow)
        {
            if (holdedthing > 0)
            {
                for (i = 1; i <= max_things; i++)
                {
                    if (thing[i].holdingsprite == num)
                    {
                        if (thing[i].style < 4)
                        {
                            // Create start velocity vector
                            cursordirection = getcursoraimdirection();
                            vec2scale(cursordirection, cursordirection, flagthrow_power);

                            // FIXME: Offset it away from the player so it isn't instantly
                            // re-grabbed, it makes it look like lag though
                            vec2scale(boffset, cursordirection, 5);

                            // Add velocity
                            b = vec2add(cursordirection, spriteparts.velocity[num]);

                            // Don't throw if the flag would collide in the upcoming frame
                            newposdiff = vec2add(boffset, b);
                            lookpoint1 = vec2add(thing[i].skeleton.pos[1], newposdiff);

                            futurepoint1 = vec2add(lookpoint1, vector2(-10, -8));
                            futurepoint2 = vec2add(lookpoint1, vector2(10, -8));
                            futurepoint3 = vec2add(lookpoint1, vector2(-10, 8));
                            futurepoint4 = vec2add(lookpoint1, vector2(10, 8));

                            lookpoint1 = vec2add(thing[i].skeleton.pos[2], newposdiff);
                            lookpoint2 = vec2add(thing[i].skeleton.pos[3], newposdiff);
                            lookpoint3 = vec2add(thing[i].skeleton.pos[4], newposdiff);

                            if (!map.raycast(skeleton.pos[15], lookpoint1, d, 200, false, true,
                                             false) and
                                !map.raycast(skeleton.pos[15], lookpoint2, d, 200, false, true,
                                             false) and
                                !map.raycast(skeleton.pos[15], lookpoint3, d, 200, false, true,
                                             false) and
                                !map.collisiontest(futurepoint1, bperp, true) and
                                !map.collisiontest(futurepoint2, bperp, true) and
                                !map.collisiontest(futurepoint3, bperp, true) and
                                !map.collisiontest(futurepoint4, bperp, true))
                            {
                                for (j = 1; j <= 4; j++)
                                {
                                    // Apply offset from flagger
                                    thing[i].skeleton.pos[j] =
                                        vec2add(thing[i].skeleton.pos[j], boffset);

                                    // Apply velocities
                                    thing[i].skeleton.pos[j] = vec2add(thing[i].skeleton.pos[j], b);
                                    thing[i].skeleton.oldpos[j] =
                                        vec2subtract(thing[i].skeleton.pos[j], b);
                                }

                                // Add some spin for visual effect
                                bperp = vector2(-b.y, b.x);
                                vec2normalize(bperp, bperp);
                                vec2scale(bperp, bperp, direction);
                                thing[i].skeleton.pos[1] =
                                    vec2subtract(thing[i].skeleton.pos[1], bperp);
                                thing[i].skeleton.pos[2] = vec2add(thing[i].skeleton.pos[2], bperp);

                                // Release the flag
                                thing[i].holdingsprite = 0;
                                holdedthing = 0;
                                flaggrabcooldown = second / 4;

#ifdef SCRIPT
                                scrptdispatcher.onflagdrop(num, thing[i].style, true);
#endif

                                thing[i].bgstate.backgroundstatus = background_transition;
                                thing[i].bgstate.backgroundpoly = background_poly_unknown;

                                thing[i].statictype = false;
#ifdef SERVER
                                serverthingmustsnapshot(i);
#endif
                            }
                        }
                    }
                }
            }
        }
    }
}

template <Config::Module M>
void Sprite<M>::throwgrenade()
{
    tvector2 a, b, c, e;
    float f = 0.0;
    float grenadearcsize, grenadearcx, grenadearcy;
    tvector2 playervelocity;

    // Start throw animation
    if (!control.thrownade)
        grenadecanthrow = true;

    if (grenadecanthrow && control.thrownade && (bodyanimation.id != roll.id) &&
        (bodyanimation.id != rollback.id))
    {
        bodyapplyanimation(throw_, 1);
#ifndef SERVER
        setsoundpaused(reloadsoundchannel, true);
#endif
    }

#ifndef SERVER
    // Pull pin
    if ((bodyanimation.id == throw_.id) && (bodyanimation.currframe == 15) &&
        (tertiaryweapon.ammocount > 0) && (ceasefirecounter < 0))
    {
        b = gethandsaimdirection();
        vec2scale(b, b, (float)(bodyanimation.currframe) / guns[fraggrenade].speed);
        if (bodyanimation.currframe < 24)
            vec2scale(b, b, 0.65);
        b = vec2add(b, spriteparts.velocity[num]);
        a.x = skeleton.pos[15].x + b.x * 3;
        a.y = skeleton.pos[15].y - 2 + b.y * 3;
        if (!map.collisiontest(a, c))
        {
            b = gethandsaimdirection();
            b.x = b.x * 0.5;
            b.y = b.y + 0.4;
            createspark(a, b, 30, num, 255); // Pin
            playsound(sfx_grenade_pullout, a);
        }
    }
#endif

    if ((bodyanimation.id == throw_.id) && (!control.thrownade || (bodyanimation.currframe == 36)))
    {
        // Grenade throw
        if ((bodyanimation.currframe > 14) && (bodyanimation.currframe < 37) &&
            (tertiaryweapon.ammocount > 0) && (ceasefirecounter < 0))
        {
            b = getcursoraimdirection();

            // Add a few degrees of arc to the throw. The arc approaches zero as you aim up or down
            grenadearcsize = (float)(sign(b.x)) / 8 * (1 - fabs(b.y));
            grenadearcx = sin(b.y * pi / 2) * grenadearcsize;
            grenadearcy = sin(b.x * pi / 2) * grenadearcsize;
            b.x = b.x + grenadearcx;
            b.y = b.y - grenadearcy;
            vec2normalize(b, b);

            vec2scale(b, b, (float)(bodyanimation.currframe) / guns[fraggrenade].speed);
            if (bodyanimation.currframe < 24)
                vec2scale(b, b, 0.65);

            vec2scale(playervelocity, spriteparts.velocity[num],
                      guns[fraggrenade].inheritedvelocity);

            b = vec2add(b, playervelocity);
            a.x = skeleton.pos[15].x + b.x * 3;
            a.y = skeleton.pos[15].y - 2 + (b.y * 3);
            e.x = spriteparts.pos[num].x;
            e.y = spriteparts.pos[num].y - 12;
            if (!map.collisiontest(a, c) and
                !map.raycast(e, a, f, 50, false, false, true, false, player->team))
            {
                createbullet(a, b, tertiaryweapon.num, num, 255, guns[fraggrenade].hitmultiply,
                             true, false);
                if
#ifndef SERVER
                    (((player->controlmethod == human) && (num == mysprite)) ||
#else
                    (
#endif
                     (player->controlmethod == bot))
                    tertiaryweapon.ammocount -= 1;

#ifndef SERVER
                if ((num == mysprite) && (guns[fraggrenade].bink < 0))
                    hitspraycounter = calculatebink(hitspraycounter, -guns[fraggrenade].bink);

                playsound(sfx_grenade_throw, a);
#endif
            }
        }

        if (control.thrownade)
            grenadecanthrow = false;

        if (weapon.ammocount == 0)
        {
            if (weapon.reloadtimecount > weapon.clipouttime)
                bodyapplyanimation(clipout, 1);
            if (weapon.reloadtimecount < weapon.clipouttime)
                bodyapplyanimation(clipin, 1);
            if ((weapon.reloadtimecount < weapon.clipintime) && (weapon.reloadtimecount > 0))
                bodyapplyanimation(slideback, 1);
#ifndef SERVER
            setsoundpaused(reloadsoundchannel, false);
#endif
        }
    }
}

template <Config::Module M>
float Sprite<M>::getmoveacc()
{
    float moveacc;

    float result;
    result = 0;

    // No moveacc for bots on harder difficulties
    if ((player->controlmethod == bot)
#ifndef SERVER
    )
#else
        && (CVar::bots_difficulty < 50))
#endif
        moveacc = 0;
    else
        moveacc = weapon.movementacc;

    if (moveacc > 0)
    {
        if ((control.jetpack && (jetscount > 0)) || (legsanimation.id == jump.id) ||
            (legsanimation.id == jumpside.id) || (legsanimation.id == run.id) ||
            (legsanimation.id == runback.id) || (legsanimation.id == roll.id) ||
            (legsanimation.id == rollback.id))
        {
            result = moveacc * 7;
        }
        else if ((!ongroundpermanent && (legsanimation.id != prone.id) &&
                  (legsanimation.id != pronemove.id) && (legsanimation.id != crouch.id) &&
                  (legsanimation.id != crouchrun.id) && (legsanimation.id != crouchrunback.id)) ||
                 (legsanimation.id == getup.id) ||
                 ((legsanimation.id == prone.id) &&
                  (legsanimation.currframe < legsanimation.numframes)))
        {
            result = moveacc * 3;
        }
    }
    return result;
}

template <Config::Module M>
tvector2 Sprite<M>::getcursoraimdirection()
{
    tvector2 mouseaim;
    tvector2 aimdirection;

    tvector2 result;
    mouseaim.x = control.mouseaimx;
    mouseaim.y = control.mouseaimy;

    aimdirection = vec2subtract(mouseaim, skeleton.pos[15]);
    vec2normalize(aimdirection, aimdirection);

    result = aimdirection;
    return result;
}

template <Config::Module M>
tvector2 Sprite<M>::gethandsaimdirection()
{
    tvector2 aimdirection;

    tvector2 result;
    aimdirection = vec2subtract(skeleton.pos[15], skeleton.pos[16]);
    vec2normalize(aimdirection, aimdirection);

    result = aimdirection;
    return result;
}

template <Config::Module M>
bool Sprite<M>::issolo()
{
    bool result;
    result = player->team == team_none;
    return result;
}

template <Config::Module M>
bool Sprite<M>::isnotsolo()
{
    bool result;
    result = player->team != team_none;
    return result;
}

template <Config::Module M>
bool Sprite<M>::isinteam()
{
    bool result;
    switch (player->team)
    {
    case team_alpha:
    case team_bravo:
    case team_charlie:
    case team_delta:
        result = true;
        break;
    default:
        result = false;
    }
    return result;
}

template <Config::Module M>
bool Sprite<M>::isspectator()
{
    return player->team == team_spectator;
}

template <Config::Module M>
bool Sprite<M>::isnotspectator()
{
    return player->team != team_spectator;
}

template <Config::Module M>
bool Sprite<M>::isinsameteam(const Sprite &otherplayer)
{
    bool result;
    result = player->team == otherplayer.player->team;
    return result;
}

template <Config::Module M>
bool Sprite<M>::isnotinsameteam(const Sprite &otherplayer)
{
    bool result;
    result = player->team != otherplayer.player->team;
    return result;
}

template <Config::Module M>
bool Sprite<M>::canrespawn(bool deadmeatbeforerespawn)
{
    bool result;
    result = (CVar::sv_survivalmode == false) or (survivalendround) or (!deadmeatbeforerespawn);
    return result;
}

template <Config::Module M>
void Sprite<M>::SetHealth(float health)
{
    Health = health;
}

template <Config::Module M>
float Sprite<M>::GetHealth()
{
    // LogDebugG("Health {}", Health);
    return Health;
}

template class Sprite<Config::GetModule()>;
