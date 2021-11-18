// automatically converted

#include "Control.hpp"
#ifdef SERVER
#include "../../server/Server.hpp"
#include "../AI.hpp"
#include "../network/NetworkServerGame.hpp"
#include "../network/NetworkServerSprite.hpp"
#else
#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/Input.hpp"
#include "../../client/Sound.hpp"
#include "../network/NetworkClientGame.hpp"
#include "../network/NetworkClientMessages.hpp"
#include "../network/NetworkClientSprite.hpp"
#endif
#include "../Calc.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include <SDL2/SDL.h>

#ifndef SERVER
bool wasrunningleft;
bool wasjumping;
bool wasthrowinggrenade;
bool waschangingweapon;
bool wasthrowingweapon;
bool wasreloadingweapon;
bool freecampressed;
#endif

namespace
{

auto &run = InitGlobalVariable<tanimation, "run">();
auto &stand = InitGlobalVariable<tanimation, "stand">();
auto &runback = InitGlobalVariable<tanimation, "runback">();
auto &jump = InitGlobalVariable<tanimation, "jump">();
auto &jumpside = InitGlobalVariable<tanimation, "jumpside">();
auto &roll = InitGlobalVariable<tanimation, "roll">();
auto &rollback = InitGlobalVariable<tanimation, "rollback">();
auto &fall = InitGlobalVariable<tanimation, "fall">();
auto &crouch = InitGlobalVariable<tanimation, "crouch">();
auto &crouchrun = InitGlobalVariable<tanimation, "crouchrun">();
auto &crouchrunback = InitGlobalVariable<tanimation, "crouchrunback">();
auto &reload = InitGlobalVariable<tanimation, "reload">();
auto &throw_ = InitGlobalVariable<tanimation, "throw_">();
auto &recoil = InitGlobalVariable<tanimation, "recoil">();
auto &shotgun = InitGlobalVariable<tanimation, "shotgun">();
auto &barret = InitGlobalVariable<tanimation, "barret">();
auto &smallrecoil = InitGlobalVariable<tanimation, "smallrecoil">();
auto &aimrecoil = InitGlobalVariable<tanimation, "aimrecoil">();
auto &handsuprecoil = InitGlobalVariable<tanimation, "handsuprecoil">();
auto &clipin = InitGlobalVariable<tanimation, "clipin">();
auto &clipout = InitGlobalVariable<tanimation, "clipout">();
auto &slideback = InitGlobalVariable<tanimation, "slideback">();
auto &change = InitGlobalVariable<tanimation, "change">();
auto &throwweapon = InitGlobalVariable<tanimation, "throwweapon">();
auto &weaponnone = InitGlobalVariable<tanimation, "weaponnone">();
auto &punch = InitGlobalVariable<tanimation, "punch">();
auto &reloadbow = InitGlobalVariable<tanimation, "reloadbow">();
auto &melee = InitGlobalVariable<tanimation, "melee">();
auto &cigar = InitGlobalVariable<tanimation, "cigar">();
auto &match = InitGlobalVariable<tanimation, "match">();
auto &smoke = InitGlobalVariable<tanimation, "smoke">();
auto &wipe = InitGlobalVariable<tanimation, "wipe">();
auto &groin = InitGlobalVariable<tanimation, "groin">();
auto &takeoff = InitGlobalVariable<tanimation, "takeoff">();
auto &victory = InitGlobalVariable<tanimation, "victory">();
auto &piss = InitGlobalVariable<tanimation, "piss">();
auto &mercy = InitGlobalVariable<tanimation, "mercy">();
auto &mercy2 = InitGlobalVariable<tanimation, "mercy2">();
auto &own = InitGlobalVariable<tanimation, "own">();
auto &prone = InitGlobalVariable<tanimation, "prone">();
auto &getup = InitGlobalVariable<tanimation, "getup">();
auto &pronemove = InitGlobalVariable<tanimation, "pronemove">();
auto &aim = InitGlobalVariable<tanimation, "aim">();
auto &handsupaim = InitGlobalVariable<tanimation, "handsupaim">();

auto &spriteparts = InitGlobalVariable<particlesystem, "spriteparts">();

} // namespace

bool checkspritelineofsightvisibility(tsprite &looksprite, tsprite &spritetocheck)
{
    tvector2 startpoint, lookpoint;
    float d2 = 0.0;

    bool result;
    result = false;
    // Do we look in the right direction?
    startpoint.x = spritetocheck.skeleton.pos[7].x - looksprite.skeleton.pos[7].x;
    startpoint.y = spritetocheck.skeleton.pos[7].y - (looksprite.skeleton.pos[7].y - 2);
    lookpoint.x = looksprite.control.mouseaimx - looksprite.skeleton.pos[7].x;
    lookpoint.y = looksprite.control.mouseaimy - (looksprite.skeleton.pos[7].y - 2);
    vec2normalize(startpoint, startpoint);
    vec2normalize(lookpoint, lookpoint);
    if (vec2dot(startpoint, lookpoint) > 0.0) // 0.5 = 90 fov, 0.0 = 180 fov, -0.5 = 270 fov
    {
        lookpoint.x = looksprite.skeleton.pos[7].x;
        lookpoint.y = looksprite.skeleton.pos[7].y - 2;
        startpoint = spritetocheck.skeleton.pos[7];
        // Is it even possible to see the player
        if (!map.raycast(lookpoint, startpoint, d2, 1001, false, false, false))
            result = true;
    }

    return result;
}

bool areconflictingkeyspressed(tsprite &spritec)
{
    // True if more than one of the keys are pressed
    bool result;
    result =
        (std::int32_t)(spritec.control.thrownade) + (std::int32_t)(spritec.control.changeweapon) +
            (std::int32_t)(spritec.control.throwweapon) + (std::int32_t)(spritec.control.reload) >
        1;
    return result;
}

void controlsprite(tsprite &spritec)
{
    tvector2 a, b, lookpoint, startpoint;
    std::int32_t i, j;
#ifndef SERVER
    std::int32_t cameratarget;
#endif
    float d;
    tgun tempgun;
    tvector2 playervelocity;
    bool playerpressedleftright = false;
    bool unprone;

    switch (spritec.style)
    {
    case 1: // Gostek
    {
        // safety
        if ((spritec.weapon.ammocount > spritec.weapon.ammo) ||
            (spritec.weapon.fireintervalcount > spritec.weapon.fireinterval) ||
            (spritec.weapon.reloadtimecount > spritec.weapon.reloadtime))
        {
            spritec.applyweaponbynum(spritec.weapon.num, 1);
            spritec.weapon.ammocount = 0;
#ifndef SERVER
            if ((spritec.num == mysprite) && !spritec.deadmeat)
                clientspritesnapshot();
#endif
        }

        if (spritec.legsanimation.speed < 1)
            spritec.legsanimation.speed = 1;
        if (spritec.bodyanimation.speed < 1)
            spritec.bodyanimation.speed = 1;
#ifndef SERVER
        if ((spritec.num == mysprite) && !escmenu->active)
        {
            spritec.freecontrols();

            sprite[mysprite].control.mouseaimx = round(mx - gamewidthhalf + camerax);
            sprite[mysprite].control.mouseaimy = round(my - gameheighthalf + cameray);

            if (!teammenu->active && !limbomenu->active)
            {
                if (chattext == "")
                {
                    for (i = low(binds); i <= high(binds) - 1; i++)
                    {
                        if (keystatus[binds[i].keyid] and
                            ((binds[i].keymod == 0) ||
                             ((binds[i].keymod && SDL_GetModState()) != 0)))
                        {
                            if (binds[i].action == taction::left)
                                spritec.control.left = true;
                            if (binds[i].action == taction::right)
                                spritec.control.right = true;
                            if (binds[i].action == taction::taction_jump)
                                spritec.control.up = true;
                            if (binds[i].action == taction::taction_crouch)
                                spritec.control.down = true;
                            if (binds[i].action == taction::fire)
                                spritec.control.fire = true;
                            if (binds[i].action == taction::jet)
                                spritec.control.jetpack = true;
                            if (binds[i].action == taction::taction_reload)
                                spritec.control.reload = true;
                            if (binds[i].action == taction::changeweapon)
                                spritec.control.changeweapon = true;
                            if (binds[i].action == taction::throwgrenade)
                                spritec.control.thrownade = true;
                            if (binds[i].action == taction::dropweapon)
                                spritec.control.throwweapon = true;
                            if (binds[i].action == taction::taction_prone)
                                spritec.control.prone = true;
                            if (binds[i].action == taction::flagthrow)
                            {
                                spritec.control.flagthrow = true;
                            }
                            else
                            {
                                spritec.control.flagthrow =
                                    spritec.control.up && spritec.control.down;
                            }
                        }
                    }

                    // If both left and right directions are pressed, then decide which direction to
                    // go in
                    if (spritec.control.left && spritec.control.right)
                    {
                        // Remember that both directions were pressed, as it's useful for some moves
                        playerpressedleftright = true;

                        if (wasjumping)
                        {
                            // If jumping, keep going in the old direction
                            if (wasrunningleft)
                                spritec.control.right = false;
                            else
                                spritec.control.left = false;
                        }
                        else
                        {
                            // If not jumping, instead go in the new direction
                            if (wasrunningleft)
                                spritec.control.left = false;
                            else
                                spritec.control.right = false;
                        }
                    }
                    else
                    {
                        wasrunningleft = spritec.control.left;
                        wasjumping = spritec.control.up;
                    }

                    // Handle simultaneous key presses that would conflict
                    if (areconflictingkeyspressed(spritec))
                    {
                        // At least two buttons pressed, so deactivate any previous one
                        if (wasthrowinggrenade)
                            spritec.control.thrownade = false;
                        else if (waschangingweapon)
                            spritec.control.changeweapon = false;
                        else if (wasthrowingweapon)
                            spritec.control.throwweapon = false;
                        else if (wasreloadingweapon)
                            spritec.control.reload = false;

                        // If simultaneously pressing two or more new buttons, then deactivate them
                        // in order of least prefecence
                        while (areconflictingkeyspressed(spritec))
                        {
                            if (spritec.control.reload)
                                spritec.control.reload = false;
                            else if (spritec.control.changeweapon)
                                spritec.control.changeweapon = false;
                            else if (spritec.control.throwweapon)
                                spritec.control.throwweapon = false;
                            else if (spritec.control.thrownade)
                                spritec.control.thrownade = false;
                        }
                    }
                    else
                    {
                        // At most one of these will be true
                        wasthrowinggrenade = spritec.control.thrownade;
                        waschangingweapon = spritec.control.changeweapon;
                        wasthrowingweapon = spritec.control.throwweapon;
                        wasreloadingweapon = spritec.control.reload;
                    }
                }
            }

            if ((length(chattext) > 0) && keystatus[301])
            {
                SDL_StopTextInput();
                firechattext = chattext;
                chattext = "";
            }

            if (freecampressed and
                !(spritec.control.fire || spritec.control.jetpack || spritec.control.up))
                freecampressed = false;

            // change camera when dead
            if (menutimer < 1)
                if (!limbomenu->active)
                {
                    if (spritec.deadmeat)
                    {
                        if (spritec.control.fire or spritec.control.jetpack or spritec.control.up)
                        {
                            if (!demoplayer.active() or (freecam == 1))
                            {
                                if (playersnum < 1)
                                    return;

                                i = 0;
                                for (j = 1; j <= max_sprites; j++)
                                    if (sprite[j].active)
                                        if (sprite[j].isnotspectator() and
                                            (sprite[j].isinsameteam(sprite[mysprite]) or
                                             sprite[mysprite].isspectator()))
                                        {
                                            i = 1;
                                            break;
                                        }

                                cameratarget = iif(i == 1, getcameratarget(spritec.control.jetpack),
                                                   (std::uint8_t)0);

                                if ((cameratarget > 0) || !freecampressed)
                                {
                                    camerafollowsprite = cameratarget;

                                    mx = gamewidthhalf;
                                    my = gameheighthalf;
                                    mouseprev.x = mx;
                                    mouseprev.y = my;
                                    spritec.control.fire = false;
                                    spritec.control.jetpack = false;
                                    menutimer = 10;
                                    freecampressed = cameratarget == 0;
                                }
                            }
                        }
                    }
                }

            // Fog of War
            if (CVar::sv_realisticmode)
            {
                for (i = 1; i <= max_sprites; i++)
                    if (sprite[i].visible > 0)
                        sprite[i].visible -= 1;

                lookpoint.x = spritec.skeleton.pos[7].x;
                lookpoint.y = spritec.skeleton.pos[7].y - 2;
                spritec.visible = 45;

                for (i = 1; i <= max_sprites; i++)
                    if (sprite[i].active)
                    {
                        // Following sprites
                        if ((spritec.num != camerafollowsprite) and
                            spritec.isnotinsameteam(sprite[i]) && spritec.isnotspectator())
                        {
                            if (checkspritelineofsightvisibility(sprite[camerafollowsprite],
                                                                 sprite[i]))
                                sprite[i].visible = 45;
                        }
                        else
                        {
                            if (spritec.deadmeat or
                                ((isteamgame() && sprite[i].isinsameteam(spritec)) or
                                 ((!isteamgame()) && sprite[i].isnotinsameteam(spritec))))
                            {
                                sprite[i].visible = 45;
                            }
                            else
                            {
                                if (checkspritelineofsightvisibility(spritec, sprite[i]))
                                    sprite[i].visible = 45;
                            }
                        }
                    }
            }
        }
#endif
#ifdef SERVER
        controlbot(spritec);
#endif

        if (spritec.deadmeat)
            spritec.freecontrols();
        if (mapchangecounter > 0)
            spritec.freecontrols();

        spritec.fired = 0;

        spritec.control.mouseaimx =
            round(spritec.control.mouseaimx + spriteparts.velocity[spritec.num].x);
        spritec.control.mouseaimy =
            round(spritec.control.mouseaimy + spriteparts.velocity[spritec.num].y);

        // use weapons
        b.x = 0;
        b.y = 0;

        if (spritec.control.jetpack and
            (((spritec.legsanimation.id == jumpside.id) &&
              (((spritec.direction == -1) && spritec.control.right) or
               ((spritec.direction == 1) && spritec.control.left) or playerpressedleftright)) or
             ((spritec.legsanimation.id == rollback.id) && spritec.control.up)))
        {
            spritec.bodyapplyanimation(rollback, 1);
            spritec.legsapplyanimation(rollback, 1);
        }
        else if (spritec.control.jetpack && (spritec.jetscount > 0))
        {
            if (spritec.onground)
                spriteparts.forces[spritec.num].y = -2.5 * iif(grav > 0.05, jetspeed, grav * 2);

            if (!spritec.onground)
            {
                if (spritec.position != pos_prone)
                    spriteparts.forces[spritec.num].y =
                        spriteparts.forces[spritec.num].y - iif(grav > 0.05, jetspeed, grav * 2);
                else
                    spriteparts.forces[spritec.num].x =
                        spriteparts.forces[spritec.num].x +
                        ((float)(spritec.direction * iif(grav > 0.05, jetspeed, grav * 2)) / 2);
            }

            if ((spritec.legsanimation.id != getup.id) && (spritec.bodyanimation.id != roll.id) &&
                (spritec.bodyanimation.id != rollback.id))
                spritec.legsapplyanimation(fall, 1);
#ifndef SERVER
            a.x = spritec.skeleton.pos[1].x - 1;
            a.y = spritec.skeleton.pos[1].y + 3;

            b = vec2subtract(spritec.skeleton.pos[5], spritec.skeleton.pos[4]);
            vec2normalize(b, b);
            vec2scale(b, b, -0.5);

            // smoke
            if (Random(8) == 0)
                createspark(a, spriteparts.velocity[spritec.num], 1, spritec.num, 75);
            // sparks
            if (Random(7) == 0)
                createspark(a, b, 62, spritec.num, 40);

            a.x = spritec.skeleton.pos[2].x - 1;
            a.y = spritec.skeleton.pos[2].y + 3;
            b = vec2subtract(spritec.skeleton.pos[6], spritec.skeleton.pos[3]);
            vec2normalize(b, b);
            vec2scale(b, b, -0.5);

            // smoke
            if (Random(8) == 0)
                createspark(a, spriteparts.velocity[spritec.num], 1, spritec.num, 75);
            // sparks
            if (Random(7) == 0)
                createspark(a, b, 62, spritec.num, 40);

#endif
            spritec.jetscount -= 1;
#ifndef SERVER
            if ((spritec.jetscount == 1) && spritec.control.jetpack)
                spritec.jetscount = 0;

            // play rockets sound
            playsound(sfx_rocketz, spriteparts.pos[spritec.num], spritec.jetssoundchannel);
#endif
        }
        else
        {
#ifndef SERVER
            stopsound(spritec.jetssoundchannel);
#endif
        }
        // Jets

        // KOLBA
        if (spritec.stat == 0)
            if (spritec.position == pos_stand)
                if ((spritec.control.fire) && (spritec.ceasefirecounter < 0))
                    if ((spritec.weapon.num != guns[noweapon].num) &&
                        (spritec.weapon.num != guns[knife].num) &&
                        (spritec.weapon.num != guns[chainsaw].num))
                    {
                        for (i = 1; i <= max_sprites; i++)
                            if (sprite[i].active && !sprite[i].deadmeat and
                                (sprite[i].position == pos_stand) && (i != spritec.num) &&
                                sprite[i].isnotspectator())
                                if (distance(spriteparts.pos[spritec.num], spriteparts.pos[i]) <
                                    melee_dist)
                                    spritec.bodyapplyanimation(melee, 1);
                    }

        // FIRE!!!!
        // (not TargetMode or (SpriteC.Num <> MySprite))
        if (spritec.stat == 0)
        {
            if ((spritec.weapon.num == guns[chainsaw].num) ||
                ((spritec.bodyanimation.id != roll.id) &&
                 (spritec.bodyanimation.id != rollback.id) &&
                 (spritec.bodyanimation.id != melee.id) && (spritec.bodyanimation.id != change.id)))
            {
                if (((spritec.bodyanimation.id == handsupaim.id) &&
                     (spritec.bodyanimation.currframe == 11)) ||
                    (spritec.bodyanimation.id != handsupaim.id))
                {
                    if (spritec.control.fire && (spritec.ceasefirecounter < 0))
                    {
                        if ((spritec.weapon.num == guns[noweapon].num) ||
                            (spritec.weapon.num == guns[knife].num))
                            spritec.bodyapplyanimation(punch, 1);
                        else
                        {
                            if ((spritec.weapon.fireintervalcount == 0) &&
                                (spritec.weapon.ammocount > 0))
                            {
                                if (spritec.weapon.startuptime > 0)
                                {
#ifndef SERVER
                                    stopsound(spritec.gattlingsoundchannel2);
#endif

                                    if (spritec.weapon.startuptimecount > 0)
                                    {
#ifndef SERVER
                                        if (spritec.weapon.startuptimecount ==
                                            spritec.weapon.startuptime)
                                        {
                                            // Barrett wind up
                                            if (spritec.weapon.num == guns[barrett].num)
                                                playsound(sfx_law_start,
                                                          spriteparts.pos[spritec.num],
                                                          spritec.gattlingsoundchannel);

                                            // Minigun wind up
                                            else if (spritec.weapon.num == guns[minigun].num)
                                                playsound(sfx_minigun_start,
                                                          spriteparts.pos[spritec.num],
                                                          spritec.gattlingsoundchannel);

                                            // LAW wind up
                                            else if (spritec.weapon.num == guns[law].num)
                                            {
                                                if (spritec.onground and
                                                    (((spritec.legsanimation.id == crouch.id) &&
                                                      (spritec.legsanimation.currframe > 13)) ||
                                                     (spritec.legsanimation.id == crouchrun.id) ||
                                                     (spritec.legsanimation.id ==
                                                      crouchrunback.id) ||
                                                     ((spritec.legsanimation.id == prone.id) &&
                                                      (spritec.legsanimation.currframe > 23))))
                                                    playsound(sfx_law_start,
                                                              spriteparts.pos[spritec.num],
                                                              spritec.gattlingsoundchannel);
                                            }
                                        }
#endif
                                        if ((spritec.weapon.num != guns[law].num) ||
                                            ((spritec.onground or spritec.ongroundpermanent) and
                                             (((spritec.legsanimation.id == crouch.id) &&
                                               (spritec.legsanimation.currframe > 13)) ||
                                              (spritec.legsanimation.id == crouchrun.id) ||
                                              (spritec.legsanimation.id == crouchrunback.id) ||
                                              ((spritec.legsanimation.id == prone.id) &&
                                               (spritec.legsanimation.currframe > 23)))))
                                            spritec.weapon.startuptimecount -= 1;
                                    }
                                    else
                                        spritec.fire();
                                }
                                else
                                    spritec.fire();
                            }
                        }
                    }
                    else
                    {
#ifndef SERVER
                        stopsound(spritec.gattlingsoundchannel);

                        if (spritec.weapon.startuptimecount < spritec.weapon.startuptime)
                        {
                            if (spritec.weapon.num == guns[minigun].num)
                                // gattling end sound
                                playsound(sfx_minigun_end, spriteparts.pos[spritec.num],
                                          spritec.gattlingsoundchannel2);
                            if (spritec.weapon.num == guns[law].num)
                                if ((spritec.onground) and
                                    (((spritec.legsanimation.id == crouch.id) &&
                                      (spritec.legsanimation.currframe > 13)) ||
                                     (spritec.legsanimation.id == crouchrun.id) ||
                                     (spritec.legsanimation.id == crouchrunback.id) ||
                                     ((spritec.legsanimation.id == prone.id) &&
                                      (spritec.legsanimation.currframe > 23))))
                                    // LAW wind down sound
                                    playsound(sfx_law_end, spriteparts.pos[spritec.num],
                                              spritec.gattlingsoundchannel2);
                        }

                        spritec.weapon.startuptimecount = spritec.weapon.startuptime;
#endif
                    }
                }
            }
            else
            {
                if (spritec.weapon.startuptimecount < spritec.weapon.startuptime)
                {
                    spritec.weapon.startuptimecount = spritec.weapon.startuptime;
                }

                spritec.burstcount = 0;
            }
        }

        if (spritec.player->controlmethod == human)
            if (!spritec.control.fire)
                spritec.burstcount = 0;

        // Fire Mode styles
        switch (spritec.weapon.firemode)
        {
        case 2: // Single shot
        {

            if (spritec.player->controlmethod == human)
                if (spritec.control.fire)
                {
                    if (((spritec.burstcount > 0) or spritec.control.reload) and
                        (spritec.weapon.fireintervalcount < 2))
                        spritec.weapon.fireintervalcount += 1;
                }
        }
        break;
        }

        // TARGET MODE
#ifndef SERVER
        if ((spritec.num == mysprite) && targetmode && spritec.control.fire)
        {
            clientfreecamtarget();
            targetmode = false;
            camerafollowsprite = mysprite;
            spritec.control.fire = false;
            spritec.weapon.fireintervalprev = spritec.weapon.fireinterval;
            spritec.weapon.fireintervalcount = spritec.weapon.fireinterval;
        }
#else
        spritec.throwflag();
#endif

        spritec.throwgrenade();

        // change weapon animation
        if ((spritec.bodyanimation.id != roll.id) && (spritec.bodyanimation.id != rollback.id) &&
            (spritec.bonusstyle != bonus_flamegod))
            if (spritec.control.changeweapon)
            {
                spritec.bodyapplyanimation(change, 1);
#ifndef SERVER
                setsoundpaused(spritec.reloadsoundchannel, true);
#endif
            }

            // clear dont drop flag if needed
#ifndef SERVER
        if (spritec.dontdrop)
            if (!spritec.control.throwweapon or (spritec.weapon.num == guns[knife].num))
                spritec.dontdrop = false;
#endif
        // throw weapon animation
        if (spritec.control.throwweapon && !spritec.control.thrownade && !spritec.dontdrop and
            (spritec.bodyanimation.id != roll.id) && (spritec.bodyanimation.id != rollback.id) &&
            ((spritec.bodyanimation.id != change.id) || (spritec.bodyanimation.currframe > 25)) &&
            (spritec.bonusstyle != bonus_flamegod) && (spritec.weapon.num != guns[bow].num) &&
            (spritec.weapon.num != guns[bow2].num) && (spritec.weapon.num != guns[noweapon].num))
        {
            spritec.bodyapplyanimation(throwweapon, 1);

            if (spritec.weapon.num == guns[knife].num)
                spritec.bodyanimation.speed = 2;

#ifndef SERVER
            stopsound(spritec.reloadsoundchannel);
#endif
        }

        // reload
        if ((spritec.weapon.num == guns[chainsaw].num) ||
            ((spritec.bodyanimation.id != roll.id) && (spritec.bodyanimation.id != rollback.id) &&
             (spritec.bodyanimation.id != change.id)))
        {
            if (spritec.control.reload)
            {
                if (spritec.weapon.ammocount != spritec.weapon.ammo)
                {
                    if (spritec.weapon.num == guns[spas12].num)
                    {
                        if (spritec.weapon.ammocount < spritec.weapon.ammo)
                        {
                            if (spritec.weapon.fireintervalcount == 0)
                                spritec.bodyapplyanimation(reload, 1);
                            else
                                spritec.autoreloadwhencanfire = true;
                        }
                    }
                    else
                    {
                        spritec.weapon.ammocount = 0;
                        spritec.weapon.fireintervalprev = spritec.weapon.fireinterval;
                        spritec.weapon.fireintervalcount = spritec.weapon.fireinterval;
                    }
                    spritec.burstcount = 0;
                }
            }
        }

        // reload shotgun / reload spas
        if ((spritec.bodyanimation.id == reload.id) && (spritec.bodyanimation.currframe == 7))
        {
#ifndef SERVER
            playsound(sfx_spas12_reload, spriteparts.pos[spritec.num], spritec.reloadsoundchannel);
#endif
            spritec.bodyanimation.currframe += 1;
        }

        if (!spritec.control.fire or (spritec.weapon.ammocount == 0))
            if ((spritec.bodyanimation.id == reload.id) && (spritec.bodyanimation.currframe == 14))
            {
                spritec.weapon.ammocount = spritec.weapon.ammocount + 1;
                if (spritec.weapon.ammocount < spritec.weapon.ammo)
                    spritec.bodyanimation.currframe = 1;
            }

        // Change Weapon
        // sound
        if ((spritec.bodyanimation.id == change.id) && (spritec.bodyanimation.currframe == 2))
        {
#ifndef SERVER
            if (spritec.secondaryweapon.num == guns[colt].num)
                playsound(sfx_changespin, spriteparts.pos[spritec.num]);
            else if (spritec.secondaryweapon.num == guns[knife].num)
                playsound(sfx_knife, spriteparts.pos[spritec.num]);
            else if (spritec.secondaryweapon.num == guns[chainsaw].num)
                playsound(sfx_chainsaw_d, spriteparts.pos[spritec.num]);
            else
                playsound(sfx_changeweapon, spriteparts.pos[spritec.num]);
#endif
            spritec.bodyanimation.currframe += 1;
        }

        if ((spritec.bodyanimation.id == change.id) && (spritec.bodyanimation.currframe == 25) &&
            (spritec.bonusstyle != bonus_flamegod))
        {
            if (
#ifndef SERVER
                (spritec.num == mysprite) ||
#endif
                (spritec.player->controlmethod == bot))
            {
                tempgun = spritec.weapon;
                spritec.weapon = spritec.secondaryweapon;
                spritec.secondaryweapon = tempgun;

                spritec.lastweaponhm = spritec.weapon.hitmultiply;
                spritec.lastweaponstyle = spritec.weapon.bulletstyle;
                spritec.lastweaponspeed = spritec.weapon.speed;
                spritec.lastweaponfire = spritec.weapon.fireinterval;
                spritec.lastweaponreload = spritec.weapon.reloadtime;

                spritec.weapon.startuptimecount = spritec.weapon.startuptime;
                spritec.weapon.reloadtimeprev = spritec.weapon.reloadtimecount;

                spritec.burstcount = 0;
            }
        }

        if (((spritec.bodyanimation.id == change.id) &&
             (spritec.bodyanimation.currframe == change.numframes)) &&
            (spritec.bonusstyle != bonus_flamegod) && (spritec.weapon.ammocount == 0))
        {
            spritec.bodyapplyanimation(stand, 1);
#ifndef SERVER
            setsoundpaused(spritec.reloadsoundchannel, false);
#endif
        }

        // Throw away weapon
#ifndef SERVER
        if ((spritec.bodyanimation.id == throwweapon.id) && (spritec.bodyanimation.currframe == 2))
            playsound(sfx_throwgun, spriteparts.pos[spritec.num]);
#endif
        if (spritec.weapon.num != guns[knife].num)
            if ((spritec.bodyanimation.id == throwweapon.id) &&
                (spritec.bodyanimation.currframe == 19) &&
                (spritec.weapon.num != guns[noweapon].num))
            {
                spritec.dropweapon();
                spritec.bodyapplyanimation(stand, 1);
            }

        // Throw knife
        if ((spritec.weapon.num == guns[knife].num) &&
            (spritec.bodyanimation.id == throwweapon.id) &&
            (!spritec.control.throwweapon or (spritec.bodyanimation.currframe == 16)))
        {
            if ((spritec.player->controlmethod == bot)
#ifndef SERVER
                || (spritec.num == mysprite))
#else
            )
#endif
            {
                // Set the dont drop flag so ThrowWeapon will not be sent to the server after
                // knife is thrown
#ifndef SERVER
                spritec.dontdrop = true;
                // Force a snapshot to be sent
                forceclientspritesnapshotmov = true;
                lastforceclientspritesnapshotmovtick = maintickcounter;
#endif
                b = spritec.getcursoraimdirection();
                vec2scale(playervelocity, spriteparts.velocity[spritec.num],
                          guns[thrownknife].inheritedvelocity);

                d = (float)(min(max(spritec.bodyanimation.currframe, 8), 16)) / 16;
                vec2scale(b, b, guns[thrownknife].speed * 1.5 * d);
                b = vec2add(b, playervelocity);
                a = spritec.skeleton.pos[16];
                createbullet(a, b, guns[thrownknife].num, spritec.num, 255,
                             guns[thrownknife].hitmultiply, true, false);
                spritec.applyweaponbynum(guns[noweapon].num, 1);
                spritec.bodyapplyanimation(stand, 1);
#ifndef SERVER
                if ((spritec.num == mysprite) && !spritec.deadmeat)
                    clientspritesnapshot();
#endif
            }
        }

        // Punch!
        if (!spritec.deadmeat)
            if ((spritec.bodyanimation.id == punch.id) && (spritec.bodyanimation.currframe == 11) &&
                (spritec.weapon.num != guns[law].num) && (spritec.weapon.num != guns[m79].num))
            {
                a.x = spritec.skeleton.pos[16].x + 2 * spritec.direction;
                a.y = spritec.skeleton.pos[16].y + 3;
                b.x = spritec.direction * 0.1;
                b.y = 0;
                createbullet(a, b, spritec.weapon.num, spritec.num, 255, spritec.weapon.hitmultiply,
                             true, false);

#ifndef SERVER
                if (spritec.weapon.num == guns[knife].num)
                    playsound(sfx_slash, spriteparts.pos[spritec.num]);
#endif

                spritec.bodyanimation.currframe += 1;
            }

        // Buttstock!
        if (!spritec.deadmeat)
            if ((spritec.bodyanimation.id == melee.id) && (spritec.bodyanimation.currframe == 12))
            {
                a.x = spritec.skeleton.pos[16].x + 2 * spritec.direction;
                a.y = spritec.skeleton.pos[16].y + 3;
                b.x = spritec.direction * 0.1;
                b.y = 0;
                createbullet(a, b, guns[noweapon].num, spritec.num, 255, guns[noweapon].hitmultiply,
                             true, true);

#ifndef SERVER
                playsound(sfx_slash, spriteparts.pos[spritec.num]);
#endif
            }

        if (spritec.bodyanimation.id == melee.id)
            if (spritec.bodyanimation.currframe > 20)
                spritec.bodyapplyanimation(stand, 1);

        // Shotgun luska
        if ((spritec.bodyanimation.id == shotgun.id) && (spritec.bodyanimation.currframe == 24))
        {
#ifndef SERVER
            b = spritec.gethandsaimdirection();
            vec2scale(b, b, spritec.weapon.speed);
            b.x = spritec.direction * 0.025 * b.y + spriteparts.velocity[spritec.num].x;
            b.y = -spritec.direction * 0.025 * b.x + spriteparts.velocity[spritec.num].y;
            a.x = spritec.skeleton.pos[15].x + 2 - spritec.direction * 0.015 * b.x;
            a.y = spritec.skeleton.pos[15].y - 2 - spritec.direction * 0.015 * b.y;

            createspark(a, b, 51, spritec.num, 255); // czerwona luska
#endif
            spritec.bodyanimation.currframe += 1;
        }

        // M79 luska
        if ((spritec.weapon.num == guns[m79].num) &&
            (spritec.weapon.reloadtimecount == spritec.weapon.clipouttime))
        {
#ifndef SERVER
            b = spritec.gethandsaimdirection();
            vec2scale(b, b, spritec.weapon.speed);
            b.x = spritec.direction * 0.08 * b.y + spriteparts.velocity[spritec.num].x;
            b.y = -spritec.direction * 0.08 * b.x + spriteparts.velocity[spritec.num].y;
            a.x = spritec.skeleton.pos[15].x + 2 - spritec.direction * 0.015 * b.x;
            a.y = spritec.skeleton.pos[15].y - 2 - spritec.direction * 0.015 * b.y;

            createspark(a, b, 52, spritec.num, 255); /*m79 luska*/
#endif
            if (spritec.weapon.reloadtimecount > 0)
                spritec.weapon.reloadtimecount -= 1;
        }

        // Prone
        if (spritec.control.prone)
        {
            if ((spritec.legsanimation.id != getup.id) && (spritec.legsanimation.id != prone.id) &&
                (spritec.legsanimation.id != pronemove.id))
            {
#ifndef SERVER
                playsound(sfx_goprone, spriteparts.pos[spritec.num]);
#endif

                spritec.legsapplyanimation(prone, 1);
                if ((spritec.bodyanimation.id != reload.id) &&
                    (spritec.bodyanimation.id != change.id) &&
                    (spritec.bodyanimation.id != throwweapon.id))
                    spritec.bodyapplyanimation(prone, 1);

                spritec.olddirection = spritec.direction;
                spritec.control.prone = false;
            }
        }

        // Get up
        if (spritec.position == pos_prone)
            if (spritec.control.prone or (spritec.direction != spritec.olddirection))
                if (((spritec.legsanimation.id == prone.id) &&
                     (spritec.legsanimation.currframe > 23)) ||
                    (spritec.legsanimation.id == pronemove.id))
                {
                    if (spritec.legsanimation.id != getup.id)
                    {
                        spritec.legsanimation = getup;
                        spritec.legsanimation.currframe = 9;
                        spritec.control.prone = false;
#ifndef SERVER
                        playsound(sfx_standup, spriteparts.pos[spritec.num]);
#endif
                    }
                    if ((spritec.bodyanimation.id != reload.id) &&
                        (spritec.bodyanimation.id != change.id) &&
                        (spritec.bodyanimation.id != throwweapon.id))
                        spritec.bodyapplyanimation(getup, 9);
                }

        unprone = false;
        // Immediately switch from unprone to jump/sidejump, because the end of the unprone
        // animation can be seen as the "wind up" for the jump
        if ((spritec.legsanimation.id == getup.id) &&
            (spritec.legsanimation.currframe > 23 - (4 - 1)) && // Possible during the last 4 frames
            spritec.onground && spritec.control.up and
            (spritec.control.right or spritec.control.left))
        {
            // Set sidejump frame 1 to 4 depending on which unprone frame we're in
            spritec.legsapplyanimation(jumpside, spritec.legsanimation.currframe - (23 - (4 - 1)));
            unprone = true;
        }
        else if ((spritec.legsanimation.id == getup.id) &&
                 (spritec.legsanimation.currframe >
                  23 - (4 - 1)) && // Possible during the last 4 frames
                 spritec.onground and
                 spritec.control.up && !(spritec.control.right or spritec.control.left))
        {
            // Set jump frame 6 to 9 depending on which unprone frame we're in
            spritec.legsapplyanimation(jump, spritec.legsanimation.currframe - (23 - (9 - 1)));
            unprone = true;
        }
        else if ((spritec.legsanimation.id == getup.id) && (spritec.legsanimation.currframe > 23))
        {
            if (spritec.control.right or spritec.control.left)
            {
                // Run back or forward depending on facing direction and direction key pressed
                if ((spritec.direction == 1) ^ spritec.control.left)
                    spritec.legsapplyanimation(run, 1);
                else
                    spritec.legsapplyanimation(runback, 1);
            }
            else if (!spritec.onground && spritec.control.up)
                spritec.legsapplyanimation(run, 1);
            else
                spritec.legsapplyanimation(stand, 1);

            unprone = true;
        }

        if (unprone)
        {
            spritec.position = pos_stand;

            if ((spritec.bodyanimation.id != reload.id) &&
                (spritec.bodyanimation.id != change.id) &&
                (spritec.bodyanimation.id != throwweapon.id))
                spritec.bodyapplyanimation(stand, 1);
        }

        // Stat overheat less
        if (!spritec.control.fire)
        {
            if (spritec.usetime > m2gun_overheat + 1)
                spritec.usetime = 0;
            if (spritec.usetime > 0)
                if (maintickcounter % 8 == 0)
                    spritec.usetime -= 1;
        }

        // Fondle Barrett?!
        if ((spritec.weapon.num == guns[barrett].num) && (spritec.weapon.fireintervalcount > 0))
            if ((spritec.bodyanimation.id == stand.id) || (spritec.bodyanimation.id == crouch.id) ||
                (spritec.bodyanimation.id == prone.id))
                spritec.bodyapplyanimation(barret, 1);

        // IDLE
        if (spritec.stat == 0)
        {
            if (((spritec.bodyanimation.id == stand.id) &&
                 (spritec.legsanimation.id == stand.id) and !spritec.deadmeat and
                 (spritec.idletime > 0)) ||
                (spritec.idletime > default_idletime))
            {
#ifndef SERVER
                if (spritec.idlerandom >= 0)
#endif
                {
                    spritec.idletime -= 1;
                }
            }
            else
            {
                spritec.idletime = default_idletime;
            }
#ifdef SERVER
            if ((spritec.idletime == 1) && (spritec.idlerandom < 0))
            {
                spritec.idletime = 0;
                spritec.idlerandom = Random(4);
            }
#endif
        }

        if (spritec.idlerandom == 0) // STUFF
        {
            if (spritec.idletime == 0)
            {
                spritec.bodyapplyanimation(smoke, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
            }

            if ((spritec.bodyanimation.id == smoke.id) && (spritec.bodyanimation.currframe == 17))
            {
#ifndef SERVER
                playsound(sfx_stuff, spriteparts.pos[spritec.num]);
#endif
                spritec.bodyanimation.currframe += 1;
            }

            if (!spritec.deadmeat)
            {
                if ((spritec.idletime == 1) && (spritec.bodyanimation.id != smoke.id) &&
                    (spritec.legsanimation.id == stand.id))
                {
#ifndef SERVER
                    a = spritec.skeleton.pos[12];
                    b = spritec.gethandsaimdirection();
                    vec2scale(b, b, 2);
                    createspark(a, b, 32, spritec.num, 245);
                    playsound(sfx_spit, spriteparts.pos[spritec.num]);
#endif
                    spritec.idletime = default_idletime;
                    spritec.idlerandom = -1;
                }
            }
        }
        else if (spritec.idlerandom == 1) // CIGAR
        {
            if (!spritec.deadmeat)
            {
                if (spritec.idletime == 0)
                {
                    if (spritec.hascigar == 0)
                    {
                        if (spritec.bodyanimation.id == stand.id)
                        {
                            // Step 1/8
                            spritec.bodyapplyanimation(cigar, 1);
#ifdef SERVER
                            serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                            spritec.idletime = default_idletime;
                        }
                    }
                    else if (spritec.hascigar == 5)
                    {
                        if ((spritec.bodyanimation.id != smoke.id) &&
                            (spritec.bodyanimation.id != cigar.id))
                        {
                            // Step 4.5/8 (only occurrs if interrupted between step 2 and 5, so redo
                            // step 1)
                            spritec.hascigar = 0;
                            spritec.bodyapplyanimation(cigar, 1);
                            spritec.idletime = default_idletime;
                        }
                    }
                    else if (spritec.hascigar == 10)
                    {
                        if (spritec.bodyanimation.id != smoke.id)
                        {
                            // Step 6/8
                            spritec.bodyapplyanimation(smoke, 1);
                            spritec.idletime = default_idletime;
                        }
                    }
                }

                if ((spritec.bodyanimation.id == cigar.id) &&
                    (spritec.bodyanimation.currframe == 37))
                    if (spritec.hascigar == 5)
                    {
                        // Step 3/8
                        spritec.bodyapplyanimation(stand, 1);
                        spritec.bodyapplyanimation(cigar, 1);
                    }

                if ((spritec.bodyanimation.id == cigar.id) &&
                    (spritec.bodyanimation.currframe == 9))
                    if (spritec.hascigar == 5)
                    {
                        // Step 4/8
#ifndef SERVER
                        playsound(sfx_match, spriteparts.pos[spritec.num]);
#endif
                        spritec.bodyanimation.currframe += 1;
                    }

                if ((spritec.bodyanimation.id == cigar.id) &&
                    (spritec.bodyanimation.currframe == 26))
                {
                    if (spritec.hascigar == 5)
                    {
                        // Step 5/8
                        spritec.hascigar = 10;
#ifndef SERVER
                        a = spritec.skeleton.pos[12];
                        a.x = a.x + spritec.direction * 4;
                        b.x = 0;
                        b.y = -0.7;
                        createspark(a, b, 31, spritec.num, 65);
                        playsound(sfx_smoke, spriteparts.pos[spritec.num]);

                        a = spritec.skeleton.pos[15];
                        b.x = (float)(spritec.direction) / 2;
                        b.y = 0.15;
                        createspark(a, b, 33, spritec.num, 245);
#endif
                        spritec.bodyanimation.currframe += 1;
                        spritec.idletime = longer_idletime;
                    }
                    else if (spritec.hascigar == 0)
                    {
                        // Step 2/8
                        spritec.hascigar = 5;
                        spritec.bodyanimation.currframe += 1;
                    }
                }

                if ((spritec.bodyanimation.id == smoke.id) &&
                    ((spritec.bodyanimation.currframe == 17) ||
                     (spritec.bodyanimation.currframe == 37)))
                {
                    // Step 7/8
#ifndef SERVER
                    a = spritec.skeleton.pos[12];
                    a.x = a.x + spritec.direction * 4;
                    b.x = 0;
                    b.y = -0.7;
                    createspark(a, b, 31, spritec.num, 65);
                    playsound(sfx_smoke, spriteparts.pos[spritec.num]);
#endif
                    spritec.bodyanimation.currframe += 1;
                }

                if ((spritec.bodyanimation.id == smoke.id) &&
                    (spritec.bodyanimation.currframe == 38))
                {
                    // Step 8/8
                    spritec.hascigar = 0;
#ifndef SERVER
                    a = spritec.skeleton.pos[15];
                    b.x = spritec.direction / 1.5;
                    b.y = 0.1;
                    createspark(a, b, 34, spritec.num, 245);
#endif
                    spritec.bodyanimation.currframe += 1;
                    spritec.idletime = default_idletime;
                    spritec.idlerandom = -1;
                }
            }
        }
        else if (spritec.idlerandom == 2) // WIPE
        {
            if (spritec.idletime == 0)
            {
                spritec.bodyapplyanimation(wipe, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
                spritec.idlerandom = -1;
            }
        }
        else if (spritec.idlerandom == 3) // EGGS
        {
            if (spritec.idletime == 0)
            {
                spritec.bodyapplyanimation(groin, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
                spritec.idlerandom = -1;
            }
        }
        else if (spritec.idlerandom == 4) // TAKE OFF HELMET
        {
            if ((spritec.weapon.num != guns[bow].num) && (spritec.weapon.num != guns[bow2].num))
            {
                if (spritec.idletime == 0)
                {
                    if (spritec.wearhelmet == 1)
                        spritec.bodyapplyanimation(takeoff, 1);
                    if (spritec.wearhelmet == 2)
                        spritec.bodyapplyanimation(takeoff, 10);
#ifdef SERVER
                    serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                    spritec.idletime = default_idletime;
                }

                if (spritec.wearhelmet == 1)
                {
                    if ((spritec.bodyanimation.id == takeoff.id) &&
                        (spritec.bodyanimation.currframe == 15))
                    {
                        spritec.wearhelmet = 2;
                        spritec.bodyanimation.currframe += 1;
                    }
                }
                else if (spritec.wearhelmet == 2)
                {
                    if ((spritec.bodyanimation.id == takeoff.id) &&
                        (spritec.bodyanimation.currframe == 22))
                    {
                        spritec.bodyapplyanimation(stand, 1);
                        spritec.idlerandom = -1;
                    }

                    if ((spritec.bodyanimation.id == takeoff.id) &&
                        (spritec.bodyanimation.currframe == 15))
                    {
                        spritec.wearhelmet = 1;
                        spritec.bodyanimation.currframe += 1;
                    }
                }
            }
        }
        else if (spritec.idlerandom == 5) // VICTORY
        {
            if (spritec.idletime == 0)
            {
                spritec.bodyapplyanimation(victory, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
                spritec.idlerandom = -1;

#ifndef SERVER
                playsound(sfx_roar, spriteparts.pos[spritec.num]);
#endif
            }
        }
        else if (spritec.idlerandom == 6) // PISS...
        {
            if (spritec.idletime == 0)
            {
                spritec.bodyapplyanimation(piss, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;

#ifndef SERVER
                playsound(sfx_piss, spriteparts.pos[spritec.num]);
#endif
            }

            if (spritec.bodyanimation.id == piss.id)
            {
                if ((spritec.bodyanimation.currframe > 8) && (spritec.bodyanimation.currframe < 22))
                {
                    if (Random(2) == 0)
                    {
#ifndef SERVER
                        a = spritec.skeleton.pos[20];
                        lookpoint.x = spritec.control.mouseaimx;
                        lookpoint.y = spritec.control.mouseaimy;
                        b = vec2subtract(spritec.skeleton.pos[20], lookpoint);
                        vec2normalize(b, b);
                        vec2scale(b, b, -1.3);
                        createspark(a, b, 57, spritec.num, 165);
#endif
                    }
                }
                else if ((spritec.bodyanimation.currframe > 21) &&
                         (spritec.bodyanimation.currframe < 34))
                {
                    if (Random(3) == 0)
                    {
#ifndef SERVER
                        a = spritec.skeleton.pos[20];
                        lookpoint.x = spritec.control.mouseaimx;
                        lookpoint.y = spritec.control.mouseaimy;
                        b = vec2subtract(spritec.skeleton.pos[20], lookpoint);
                        vec2normalize(b, b);
                        vec2scale(b, b, -1.9);
                        createspark(a, b, 57, spritec.num, 120);
#endif
                    }
                }
                else if ((spritec.bodyanimation.currframe > 33) &&
                         (spritec.bodyanimation.currframe < 35))
                {
                    if (Random(4) == 0)
                    {
#ifndef SERVER
                        a = spritec.skeleton.pos[20];
                        lookpoint.x = spritec.control.mouseaimx;
                        lookpoint.y = spritec.control.mouseaimy;
                        b = vec2subtract(spritec.skeleton.pos[20], lookpoint);
                        vec2normalize(b, b);
                        vec2scale(b, b, -1.3);

                        createspark(a, b, 57, spritec.num, 120);
#endif
                    }
                }
                else if (spritec.bodyanimation.currframe == 37)
                {
                    spritec.idlerandom = -1;
                }
            }
        }
        else if (spritec.idlerandom == 7) // SELFKILL
        {
            if (spritec.idletime == 0)
            {
                if (spritec.canmercy)
                {
                    if ((spritec.weapon.num == guns[m79].num) ||
                        (spritec.weapon.num == guns[m249].num) ||
                        (spritec.weapon.num == guns[spas12].num) ||
                        (spritec.weapon.num == guns[law].num) ||
                        (spritec.weapon.num == guns[chainsaw].num) ||
                        (spritec.weapon.num == guns[barrett].num) ||
                        (spritec.weapon.num == guns[minigun].num))
                    {
                        spritec.bodyapplyanimation(mercy2, 1);
                        spritec.legsapplyanimation(mercy2, 1);
                    }
                    else if (spritec.weapon.num != guns[minigun].num)
                    {
                        spritec.bodyapplyanimation(mercy, 1);
                        spritec.legsapplyanimation(mercy, 1);
                    }

#ifndef SERVER
                    playsound(sfx_mercy, spriteparts.pos[spritec.num]);
                    if (spritec.weapon.num == guns[minigun].num)
                        playsound(sfx_minigun_start, spriteparts.pos[spritec.num]);
#else
                    serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                    spritec.idletime = default_idletime;

                    spritec.canmercy = false;
                }
                else
                {
                    spritec.idlerandom = -1;
                    spritec.canmercy = true;
                }
            }

            if ((spritec.bodyanimation.id == mercy.id) || (spritec.bodyanimation.id == mercy2.id))
            {
                if (spritec.bodyanimation.currframe == 20)
                {
                    spritec.fire();
#ifndef SERVER
                    if (spritec.weapon.num == guns[knife].num)
                        playsound(sfx_slash, spriteparts.pos[spritec.num],
                                  sprite[spritec.num].gattlingsoundchannel);
                    if (spritec.weapon.num == guns[chainsaw].num)
                        playsound(sfx_chainsaw_r, spriteparts.pos[spritec.num],
                                  sprite[spritec.num].gattlingsoundchannel);
                    if (spritec.weapon.num == guns[noweapon].num)
                        playsound(sfx_dead_hit, spriteparts.pos[spritec.num],
                                  sprite[spritec.num].gattlingsoundchannel);

                    if (spritec.num == mysprite)
                        clientsendstringmessage("/KILL", spritec.num);
#endif
                    spritec.bodyanimation.currframe += 1;

                    spritec.idlerandom = -1;
                }
            }
        }
        else if (spritec.idlerandom == 8) // PWN!
        {
            if (spritec.idletime == 0)
            {
                spritec.bodyapplyanimation(own, 1);
                spritec.legsapplyanimation(own, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
                spritec.idlerandom = -1;
            }
        }

        // *CHEAT*
        if (spritec.legsanimation.speed > 1)
        {
            if ((spritec.legsanimation.id == jump.id) ||
                (spritec.legsanimation.id == jumpside.id) ||
                (spritec.legsanimation.id == roll.id) ||
                (spritec.legsanimation.id == rollback.id) ||
                (spritec.legsanimation.id == prone.id) || (spritec.legsanimation.id == run.id) ||
                (spritec.legsanimation.id == runback.id))
            {
                spriteparts.velocity[spritec.num].x =
                    ((float)(spriteparts.velocity[spritec.num].x) / spritec.legsanimation.speed);
                spriteparts.velocity[spritec.num].y =
                    ((float)(spriteparts.velocity[spritec.num].y) / spritec.legsanimation.speed);
            }

            if (spritec.legsanimation.speed > 2)
                if ((spritec.legsanimation.id == pronemove.id) ||
                    (spritec.legsanimation.id == crouchrun.id))
                {
                    spriteparts.velocity[spritec.num].x =
                        ((float)(spriteparts.velocity[spritec.num].x) /
                         spritec.legsanimation.speed);
                    spriteparts.velocity[spritec.num].y =
                        ((float)(spriteparts.velocity[spritec.num].y) /
                         spritec.legsanimation.speed);
                }
        }

        // stat gun deactivate if needed
        if (spritec.control.up or spritec.control.jetpack)
            if (spritec.stat > 0)
            {
                thing[spritec.stat].statictype = false;
                spritec.stat = 0;
            }

        // AimDistCoef (Sniper view)
        if (spritec.weapon.num == guns[barrett].num)
        {
            if ((spritec.weapon.fireintervalcount == 0) &&
                ((spritec.bodyanimation.id == prone.id) || (spritec.bodyanimation.id == aim.id)))
            {
                if ((fabs(spritec.control.mouseaimx - spriteparts.pos[spritec.num].x) >=
                     640 / 1.035) ||
                    (fabs(spritec.control.mouseaimy - spriteparts.pos[spritec.num].y) >=
                     480 / 1.035))
                {
                    if (spritec.aimdistcoef == defaultaimdist)
                    {
#ifndef SERVER
                        playsound(sfx_scope, spriteparts.pos[spritec.num]);
#else
                        serverspritedeltasmouse(spritec.num);
#endif
                    }

                    if (spritec.bodyanimation.id == prone.id)
                        if (spritec.aimdistcoef > sniperaimdist)
                        {
                            spritec.aimdistcoef = spritec.aimdistcoef - aimdistincr;
                            if (maintickcounter % 27 == 0)
                            {
#ifndef SERVER
                                playsound(sfx_scoperun, spriteparts.pos[spritec.num]);
#else
                                serverspritedeltasmouse(spritec.num);
#endif
                            }
                        }

                    if (spritec.bodyanimation.id == aim.id)
                        if (spritec.aimdistcoef > crouchaimdist)
                        {
                            spritec.aimdistcoef = spritec.aimdistcoef - 2 * aimdistincr;
                            if (maintickcounter % 27 == 0)
                            {
#ifndef SERVER
                                playsound(sfx_scoperun, spriteparts.pos[spritec.num]);
#else
                                serverspritedeltasmouse(spritec.num);
#endif
                            }
                        }
                }

                if ((fabs(spritec.control.mouseaimx - spriteparts.pos[spritec.num].x) <
                     640 / 1.5) &&
                    (fabs(spritec.control.mouseaimy - spriteparts.pos[spritec.num].y) < 480 / 1.5))
                {
                    if (spritec.aimdistcoef < defaultaimdist)
                    {
                        spritec.aimdistcoef = spritec.aimdistcoef + aimdistincr;
#ifndef SERVER
                        if (spritec.aimdistcoef == defaultaimdist)
                            playsound(sfx_scope, spriteparts.pos[spritec.num]);
#endif
                        if (maintickcounter % 27 == 0)
                        {
#ifndef SERVER
                            playsound(sfx_scoperun, spriteparts.pos[spritec.num]);
#else
                            serverspritedeltasmouse(spritec.num);
#endif
                        }
                    }
                }
            }
            else
            {
                if (spritec.aimdistcoef != defaultaimdist)
                {
#ifndef SERVER
                    playsound(sfx_scopeback, spriteparts.pos[spritec.num]);
#else
                    serverspritedeltasmouse(spritec.num);
#endif
                }

                spritec.aimdistcoef = defaultaimdist;
                spritec.control.mousedist = 150;
            }
        }
        else
        {
            spritec.aimdistcoef = defaultaimdist;
            spritec.control.mousedist = 150;
        }

        // Check if near collider
        if (maintickcounter % 10 == 0)
        {
            spritec.colliderdistance = 255; // not near

            for (j = 1; j <= 128; j++)
                if (map.collider[j].active)
                {
                    a.x = map.collider[j].x;
                    a.y = map.collider[j].y;

                    b = vec2subtract(spritec.skeleton.pos[15], spritec.skeleton.pos[16]);
                    vec2normalize(b, b);
                    vec2scale(b, b, 8);
                    startpoint.x = spritec.skeleton.pos[12].x;
                    startpoint.y = spritec.skeleton.pos[12].y - 5;
                    lookpoint = vec2add(startpoint, b);

                    b = vec2subtract(lookpoint, a);
                    d = vec2length(b);

                    if (d < map.collider[j].radius)
                    {
                        spritec.colliderdistance = 1;

                        if (spritec.colliderdistance == 1)
                        {
                            if (d > 253)
                                d = 253;
                            spritec.colliderdistance = round(d);
                        }

                        break;
                    }
                }

            // raise weapon above teammate when crouching
            for (j = 1; j <= max_sprites; j++)
                if (isteamgame())
                    if (sprite[j].active && sprite[j].isinsameteam(spritec) and
                        (sprite[j].position == pos_crouch) && (j != spritec.num) &&
                        spritec.isnotspectator())
                    {
                        a = spriteparts.pos[j];

                        b = vec2subtract(spritec.skeleton.pos[15], spritec.skeleton.pos[16]);
                        vec2normalize(b, b);
                        vec2scale(b, b, 8);
                        startpoint.x = spritec.skeleton.pos[12].x;
                        startpoint.y = spritec.skeleton.pos[12].y - 5;
                        lookpoint = vec2add(startpoint, b);

                        b = vec2subtract(lookpoint, a);
                        d = vec2length(b);

                        if (d < sprite_radius)
                        {
                            spritec.colliderdistance = 1;

                            if (spritec.colliderdistance == 1)
                            {
                                if (d > 253)
                                    d = 253;
                                spritec.colliderdistance = round(d);
                            }

                            break;
                        }
                    }
        }
#ifndef SERVER
        if (targetmode && (spritec.num == mysprite))
        {
            spritec.freecontrols();
        }
#endif
        // End any ongoing idle animations if a key is pressed
        if ((spritec.bodyanimation.id == cigar.id) || (spritec.bodyanimation.id == match.id) ||
            (spritec.bodyanimation.id == smoke.id) || (spritec.bodyanimation.id == wipe.id) ||
            (spritec.bodyanimation.id == groin.id))
        {
            if (spritec.control.left or spritec.control.right or spritec.control.up or
                spritec.control.down or spritec.control.fire or spritec.control.jetpack or
                spritec.control.thrownade or spritec.control.changeweapon or
                spritec.control.throwweapon or spritec.control.reload or spritec.control.prone)
            {
                spritec.bodyanimation.currframe = spritec.bodyanimation.numframes;
            }
        }

        // make anims out of controls
        // rolling
        if ((spritec.bodyanimation.id != takeoff.id) && (spritec.bodyanimation.id != piss.id) &&
            (spritec.bodyanimation.id != mercy.id) && (spritec.bodyanimation.id != mercy2.id) &&
            (spritec.bodyanimation.id != victory.id) && (spritec.bodyanimation.id != own.id))
        {
            if ((spritec.bodyanimation.id == roll.id) || (spritec.bodyanimation.id == rollback.id))
            {
                if (spritec.legsanimation.id == roll.id)
                {
                    if (spritec.onground) // if staying on ground
                        spriteparts.forces[spritec.num].x = spritec.direction * rollspeed;
                    else
                        spriteparts.forces[spritec.num].x = spritec.direction * 2 * flyspeed;
                }
                else if (spritec.legsanimation.id == rollback.id)
                {
                    if (spritec.onground) // if staying on ground
                        spriteparts.forces[spritec.num].x = -spritec.direction * rollspeed;
                    else
                        spriteparts.forces[spritec.num].x = -spritec.direction * 2 * flyspeed;

                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 1) &&
                        (spritec.legsanimation.currframe < 8))
                    {
                        if (spritec.control.up)
                        {
                            spriteparts.forces[spritec.num].y =
                                spriteparts.forces[spritec.num].y - jumpdirspeed * 1.5;
                            spriteparts.forces[spritec.num].x =
                                spriteparts.forces[spritec.num].x * 0.5;
                            spriteparts.velocity[spritec.num].x =
                                spriteparts.velocity[spritec.num].x * 0.8;
                        }
                    }
                }
            }
            // downright
            else if (spritec.control.right && spritec.control.down)
            {
                if (spritec.onground) // if staying on ground
                {
                    // roll to the side
                    if ((spritec.legsanimation.id == run.id) ||
                        (spritec.legsanimation.id == runback.id) ||
                        (spritec.legsanimation.id == fall.id) ||
                        (spritec.legsanimation.id == pronemove.id) ||
                        ((spritec.legsanimation.id == prone.id) &&
                         (spritec.legsanimation.currframe >= 24)))
                    {
                        if ((spritec.legsanimation.id == pronemove.id) ||
                            ((spritec.legsanimation.id == prone.id) &&
                             (spritec.legsanimation.currframe == spritec.legsanimation.numframes)))
                        {
                            spritec.control.prone = false;
                            spritec.position = pos_stand;
                        }
#ifndef SERVER
                        if ((spritec.legsanimation.id != rollback.id) &&
                            (spritec.legsanimation.id != roll.id))
                            playsound(sfx_roll, spriteparts.pos[spritec.num]);

                        setsoundpaused(spritec.reloadsoundchannel, true);
#endif

                        if (spritec.direction == 1)
                        {
                            spritec.bodyapplyanimation(roll, 1);
                            spritec.legsanimation = roll;
                            spritec.legsanimation.currframe = 1;
                        }
                        else
                        {
                            spritec.bodyapplyanimation(rollback, 1);
                            spritec.legsanimation = rollback;
                            spritec.legsanimation.currframe = 1;
                        }
                    }
                    else
                    {
                        if (spritec.direction == 1)
                            spritec.legsapplyanimation(crouchrun, 1);
                        else
                            spritec.legsapplyanimation(crouchrunback, 1);
                    }

                    if ((spritec.legsanimation.id == crouchrun.id) ||
                        (spritec.legsanimation.id == crouchrunback.id))
                        spriteparts.forces[spritec.num].x = crouchrunspeed;
                    else if ((spritec.legsanimation.id == roll.id) ||
                             (spritec.legsanimation.id == rollback.id))
                        spriteparts.forces[spritec.num].x = 2 * crouchrunspeed;
                }
            }
            // downleft
            else if (spritec.control.left && spritec.control.down)
            {
                if (spritec.onground) // if staying on ground
                {
                    // roll to the side
                    if ((spritec.legsanimation.id == run.id) ||
                        (spritec.legsanimation.id == runback.id) ||
                        (spritec.legsanimation.id == fall.id) ||
                        (spritec.legsanimation.id == pronemove.id) ||
                        ((spritec.legsanimation.id == prone.id) &&
                         (spritec.legsanimation.currframe >= 24)))
                    {
                        if ((spritec.legsanimation.id == pronemove.id) ||
                            ((spritec.legsanimation.id == prone.id) &&
                             (spritec.legsanimation.currframe == spritec.legsanimation.numframes)))
                        {
                            spritec.control.prone = false;
                            spritec.position = pos_stand;
                        }
#ifndef SERVER
                        if ((spritec.legsanimation.id != rollback.id) &&
                            (spritec.legsanimation.id != roll.id))
                            playsound(sfx_roll, spriteparts.pos[spritec.num]);

                        setsoundpaused(spritec.reloadsoundchannel, true);
#endif

                        if (spritec.direction == 1)
                        {
                            spritec.bodyapplyanimation(rollback, 1);
                            spritec.legsanimation = rollback;
                            spritec.legsanimation.currframe = 1;
                        }
                        else
                        {
                            spritec.bodyapplyanimation(roll, 1);
                            spritec.legsanimation = roll;
                            spritec.legsanimation.currframe = 1;
                        }
                    }
                    else
                    {
                        if (spritec.direction == 1)
                            spritec.legsapplyanimation(crouchrunback, 1);
                        else
                            spritec.legsapplyanimation(crouchrun, 1);
                    }

                    if ((spritec.legsanimation.id == crouchrun.id) ||
                        (spritec.legsanimation.id == crouchrunback.id))
                        spriteparts.forces[spritec.num].x = -crouchrunspeed;
                }
            }
            // Proning
            // FIXME(skoskav): The "and Body <> Throw|Punch" check is to keep the grenade tap and
            // punch/stab prone cancel bugs
            else if ((spritec.legsanimation.id == prone.id) ||
                     (spritec.legsanimation.id == pronemove.id) ||
                     ((spritec.legsanimation.id == getup.id) &&
                      (spritec.bodyanimation.id != throw_.id) &&
                      (spritec.bodyanimation.id != punch.id)))
            {
                if (spritec.onground)
                {
                    if (((spritec.legsanimation.id == prone.id) &&
                         (spritec.legsanimation.currframe > 25)) ||
                        (spritec.legsanimation.id == pronemove.id))
                    {
                        if (spritec.control.left or spritec.control.right)
                        {
                            if ((spritec.legsanimation.currframe < 4) ||
                                (spritec.legsanimation.currframe > 14))
                                spriteparts.forces[spritec.num].x =
                                    iif(spritec.control.left, -pronespeed, pronespeed);

                            spritec.legsapplyanimation(pronemove, 1);
                            if ((spritec.bodyanimation.id != clipin.id) &&
                                (spritec.bodyanimation.id != clipout.id) &&
                                (spritec.bodyanimation.id != slideback.id) &&
                                (spritec.bodyanimation.id != reload.id) &&
                                (spritec.bodyanimation.id != change.id) &&
                                (spritec.bodyanimation.id != throw_.id) &&
                                (spritec.bodyanimation.id != throwweapon.id))
                                spritec.bodyapplyanimation(pronemove, 1);

                            if (spritec.legsanimation.id != pronemove.id)
                                spritec.legsanimation = pronemove;
                        }
                        else
                        {
                            if (spritec.legsanimation.id != prone.id)
                                spritec.legsanimation = prone;
                            spritec.legsanimation.currframe = 26;
                        }
                    }
                }
            }
            // upright
            else if (spritec.control.right && spritec.control.up)
            {
                if (spritec.onground) // if staying on ground
                {
                    // jump to the side
                    if ((spritec.legsanimation.id == run.id) ||
                        (spritec.legsanimation.id == runback.id) ||
                        (spritec.legsanimation.id == stand.id) ||
                        (spritec.legsanimation.id == crouch.id) ||
                        (spritec.legsanimation.id == crouchrun.id) ||
                        (spritec.legsanimation.id == crouchrunback.id))
                    {
                        spritec.legsapplyanimation(jumpside, 1);
#ifndef SERVER
                        playsound(sfx_jump, spriteparts.pos[spritec.num]);
#endif
                    }

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(run, 1);
                }
                else if ((spritec.legsanimation.id == roll.id) ||
                         (spritec.legsanimation.id == rollback.id))
                {
                    if (spritec.direction == 1)
                        spritec.legsapplyanimation(run, 1);
                    else
                        spritec.legsapplyanimation(runback, 1);
                }

                if (spritec.legsanimation.id == jump.id)
                {
                    if (spritec.legsanimation.currframe < 10)
                    {
                        spritec.legsapplyanimation(jumpside, 1);
                    }
                }

                if (spritec.legsanimation.id == jumpside.id)
                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 3) &&
                        (spritec.legsanimation.currframe < 11))
                    {
                        spriteparts.forces[spritec.num].x = jumpdirspeed;
                        spriteparts.forces[spritec.num].y = -jumpdirspeed / 1.2;
                    }
            }
            // upleft
            else if (spritec.control.left && spritec.control.up)
            {
                if (spritec.onground) // if staying on ground
                {
                    // jump to the side
                    if ((spritec.legsanimation.id == run.id) ||
                        (spritec.legsanimation.id == runback.id) ||
                        (spritec.legsanimation.id == stand.id) ||
                        (spritec.legsanimation.id == crouch.id) ||
                        (spritec.legsanimation.id == crouchrun.id) ||
                        (spritec.legsanimation.id == crouchrunback.id))
                    {
                        spritec.legsapplyanimation(jumpside, 1);
#ifndef SERVER
                        playsound(sfx_jump, spriteparts.pos[spritec.num]);
#endif
                    }

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(run, 1);
                }
                else if ((spritec.legsanimation.id == roll.id) ||
                         (spritec.legsanimation.id == rollback.id))
                {
                    if (spritec.direction == -1)
                        spritec.legsapplyanimation(run, 1);
                    else
                        spritec.legsapplyanimation(runback, 1);
                }

                if (spritec.legsanimation.id == jump.id)
                {
                    if (spritec.legsanimation.currframe < 10)
                    {
                        spritec.legsapplyanimation(jumpside, 1);
                    }
                }

                if (spritec.legsanimation.id == jumpside.id)
                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 3) &&
                        (spritec.legsanimation.currframe < 11))
                    {
                        spriteparts.forces[spritec.num].x = -jumpdirspeed;
                        spriteparts.forces[spritec.num].y = -jumpdirspeed / 1.2;
                    }
            }
            // up
            else if (spritec.control.up)
            {
                if (spritec.onground) // if staying on ground
                {
                    if (spritec.legsanimation.id != jump.id)
                    {
                        spritec.legsapplyanimation(jump, 1);
#ifndef SERVER
                        playsound(sfx_jump, spriteparts.pos[spritec.num]);
#endif
                    }

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(stand, 1);
                }

                if (spritec.legsanimation.id == jump.id)
                {
                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 8) &&
                        (spritec.legsanimation.currframe < 15))
                        spriteparts.forces[spritec.num].y = -jumpspeed;

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(fall, 1);
                }
            }
            // down
            else if (spritec.control.down)
            {
                if (spritec.onground) // if staying on ground
                {
#ifndef SERVER
                    if ((spritec.legsanimation.id != crouchrun.id) &&
                        (spritec.legsanimation.id != crouchrunback.id) &&
                        (spritec.legsanimation.id != crouch.id))
                        playsound(sfx_crouch, spriteparts.pos[spritec.num]);
#endif

                    spritec.legsapplyanimation(crouch, 1);
                }
            }
            // right
            else if (spritec.control.right)
            {
                if (spritec.para == 0)
                {
                    if (spritec.direction == 1)
                        spritec.legsapplyanimation(run, 1);
                    else
                        spritec.legsapplyanimation(runback, 1);
                }
                else if (spritec.holdedthing != 0)
                {
                    // parachute bend
                    thing[spritec.holdedthing].skeleton.forces[3].y =
                        thing[spritec.holdedthing].skeleton.forces[3].y - 0.5;
                    thing[spritec.holdedthing].skeleton.forces[2].y =
                        thing[spritec.holdedthing].skeleton.forces[2].y + 0.5;
                }

                if (spritec.onground) // if staying on ground
                {
                    spriteparts.forces[spritec.num].x = runspeed;
                    spriteparts.forces[spritec.num].y = -runspeedup;
                }
                else
                    spriteparts.forces[spritec.num].x = flyspeed;
            }
            // left
            else if (spritec.control.left)
            {
                if (spritec.para == 0)
                {
                    if (spritec.direction == -1)
                        spritec.legsapplyanimation(run, 1);
                    else
                        spritec.legsapplyanimation(runback, 1);
                }
                else if (spritec.holdedthing != 0)
                {
                    // parachute bend
                    thing[spritec.holdedthing].skeleton.forces[2].y =
                        thing[spritec.holdedthing].skeleton.forces[2].y - 0.5;
                    thing[spritec.holdedthing].skeleton.forces[3].y =
                        thing[spritec.holdedthing].skeleton.forces[3].y + 0.5;
                }

                if (spritec.onground) // if staying on ground
                {
                    spriteparts.forces[spritec.num].x = -runspeed;
                    spriteparts.forces[spritec.num].y = -runspeedup;
                }
                else
                    spriteparts.forces[spritec.num].x = -flyspeed;
            }
            // else all keys not pressed
            else
            {
                if (spritec.onground) // if staying on ground
                {
#ifndef SERVER
                    if (!spritec.deadmeat)
                        if (spritec.legsanimation.id != stand.id)
                            playsound(sfx_stop, spriteparts.pos[spritec.num]);
#endif
                    spritec.legsapplyanimation(stand, 1);
                }
                else
                    spritec.legsapplyanimation(fall, 1);
            }
        }

        // Body animations

        // reloading
        if ((spritec.weapon.reloadtimecount == spritec.weapon.clipouttime) &&
            (spritec.bodyanimation.id != reload.id) && (spritec.bodyanimation.id != reloadbow.id) &&
            (spritec.bodyanimation.id != roll.id) && (spritec.bodyanimation.id != rollback.id))
            spritec.bodyapplyanimation(clipin, 1);
        if (spritec.weapon.reloadtimecount == spritec.weapon.clipintime)
            spritec.bodyapplyanimation(slideback, 1);

        // this piece of code fixes the infamous crouch bug
        // how you ask? well once upon time soldat's code decided that
        // randomly the animation for roll for the body and legs will magically
        // go out of sync, which is causing the crouch bug. so this piece of
        // awesome code simply syncs them when they go out of sync <3
        if ((spritec.legsanimation.id == roll.id) && (spritec.bodyanimation.id != roll.id))
            spritec.bodyapplyanimation(roll, 1);
        if ((spritec.bodyanimation.id == roll.id) && (spritec.legsanimation.id != roll.id))
            spritec.legsapplyanimation(roll, 1);
        if ((spritec.legsanimation.id == rollback.id) && (spritec.bodyanimation.id != rollback.id))
            spritec.bodyapplyanimation(rollback, 1);
        if ((spritec.bodyanimation.id == rollback.id) && (spritec.legsanimation.id != rollback.id))
            spritec.legsapplyanimation(rollback, 1);

        if ((spritec.bodyanimation.id == roll.id) || (spritec.bodyanimation.id == rollback.id))
        {
            if (spritec.legsanimation.currframe != spritec.bodyanimation.currframe)
            {
                if (spritec.legsanimation.currframe > spritec.bodyanimation.currframe)
                    spritec.bodyanimation.currframe = spritec.legsanimation.currframe;
                else
                    spritec.legsanimation.currframe = spritec.bodyanimation.currframe;
            }
        }

        // Gracefully end a roll animation
        if (((spritec.bodyanimation.id == roll.id) || (spritec.bodyanimation.id == rollback.id)) &&
            (spritec.bodyanimation.currframe == spritec.bodyanimation.numframes))
        {
            // Was probably a roll
            if (spritec.onground)
            {
                if (spritec.control.down)
                {
                    if (spritec.control.left or spritec.control.right)
                    {
                        if (spritec.bodyanimation.id == roll.id)
                            spritec.legsapplyanimation(crouchrun, 1);
                        else
                            spritec.legsapplyanimation(crouchrunback, 1);
                    }
                    else
                        spritec.legsapplyanimation(crouch, 15);
                }
            }
            // Was probably a backflip
            else if ((spritec.bodyanimation.id == rollback.id) && spritec.control.up)
            {
                if (spritec.control.left or spritec.control.right)
                {
                    // Run back or forward depending on facing direction and direction key pressed
                    if ((spritec.direction == 1) ^ spritec.control.left)
                        spritec.legsapplyanimation(run, 1);
                    else
                        spritec.legsapplyanimation(runback, 1);
                }
                else
                    spritec.legsapplyanimation(fall, 1);
            }
            // Was probably a roll (that ended mid-air)
            else if (spritec.control.down)
            {
                if (spritec.control.left or spritec.control.right)
                {
                    if (spritec.bodyanimation.id == roll.id)
                        spritec.legsapplyanimation(crouchrun, 1);
                    else
                        spritec.legsapplyanimation(crouchrunback, 1);
                }
                else
                    spritec.legsapplyanimation(crouch, 15);
            }

            spritec.bodyapplyanimation(stand, 1);
        }

        if (spritec.weapon.ammocount > 0)
            if ((!spritec.control.thrownade && (spritec.bodyanimation.id != recoil.id) &&
                 (spritec.bodyanimation.id != smallrecoil.id) &&
                 (spritec.bodyanimation.id != aimrecoil.id) &&
                 (spritec.bodyanimation.id != handsuprecoil.id) &&
                 (spritec.bodyanimation.id != shotgun.id) &&
                 (spritec.bodyanimation.id != barret.id) &&
                 (spritec.bodyanimation.id != change.id) &&
                 (spritec.bodyanimation.id != throwweapon.id) &&
                 (spritec.bodyanimation.id != weaponnone.id) &&
                 (spritec.bodyanimation.id != punch.id) && (spritec.bodyanimation.id != roll.id) &&
                 (spritec.bodyanimation.id != rollback.id) &&
                 (spritec.bodyanimation.id != reloadbow.id) &&
                 (spritec.bodyanimation.id != cigar.id) && (spritec.bodyanimation.id != match.id) &&
                 (spritec.bodyanimation.id != smoke.id) && (spritec.bodyanimation.id != wipe.id) &&
                 (spritec.bodyanimation.id != takeoff.id) &&
                 (spritec.bodyanimation.id != groin.id) && (spritec.bodyanimation.id != piss.id) &&
                 (spritec.bodyanimation.id != mercy.id) &&
                 (spritec.bodyanimation.id != mercy2.id) &&
                 (spritec.bodyanimation.id != victory.id) && (spritec.bodyanimation.id != own.id) &&
                 (spritec.bodyanimation.id != reload.id) &&
                 (spritec.bodyanimation.id != prone.id) && (spritec.bodyanimation.id != getup.id) &&
                 (spritec.bodyanimation.id != pronemove.id) &&
                 (spritec.bodyanimation.id != melee.id)) ||
                ((spritec.bodyanimation.currframe == spritec.bodyanimation.numframes) &&
                 (spritec.bodyanimation.id != prone.id)) ||
                ((spritec.weapon.fireintervalcount == 0) &&
                 (spritec.bodyanimation.id == barret.id)))
            {
                if (spritec.position != pos_prone)
                {
                    if (spritec.position == pos_stand)
                        spritec.bodyapplyanimation(stand, 1);

                    if (spritec.position == pos_crouch)
                    {
                        if (spritec.colliderdistance < 255)
                        {
                            if (spritec.bodyanimation.id == handsuprecoil.id)
                                spritec.bodyapplyanimation(handsupaim, 11);
                            else
                                spritec.bodyapplyanimation(handsupaim, 1);
                        }
                        else
                        {
                            if (spritec.bodyanimation.id == aimrecoil.id)
                                spritec.bodyapplyanimation(aim, 6);
                            else
                                spritec.bodyapplyanimation(aim, 1);
                        }
                    }
                }
                else
                    spritec.bodyapplyanimation(prone, 26);
            }

        if ((spritec.legsanimation.id == crouch.id) || (spritec.legsanimation.id == crouchrun.id) ||
            (spritec.legsanimation.id == crouchrunback.id))
            spritec.position = pos_crouch;
        else
            spritec.position = pos_stand;

        if ((spritec.legsanimation.id == prone.id) || (spritec.legsanimation.id == pronemove.id))
            spritec.position = pos_prone;

#ifndef SERVER
        if (clientstopmovingcounter < 1)
            spritec.freecontrols();
#endif
    }
    break;
    }
}
