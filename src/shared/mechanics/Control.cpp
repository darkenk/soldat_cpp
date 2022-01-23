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
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "common/Calc.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <SDL2/SDL.h>
#include <Tracy.hpp>

// clang-format on
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format off

#ifndef SERVER
bool wasrunningleft;
bool wasjumping;
bool wasthrowinggrenade;
bool waschangingweapon;
bool wasthrowingweapon;
bool wasreloadingweapon;
bool freecampressed;
#endif

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
    auto &map = GS::GetGame().GetMap();
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
    ZoneScopedN("ControlSprite");
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

    auto &map = GS::GetGame().GetMap();
    auto& spriteSystem = SpriteSystem::Get();
    const auto &spritePartsPos = spriteSystem.GetSpritePartsPos(spritec.num);
    auto &spriteVelocity = spriteSystem.GetVelocity(spritec.num);
    auto &spriteForces = spriteSystem.GetForces(spritec.num);


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

            SpriteSystem::Get().GetSprite(mysprite).control.mouseaimx = round(mx - gamewidthhalf + camerax);
            SpriteSystem::Get().GetSprite(mysprite).control.mouseaimy = round(my - gameheighthalf + cameray);

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
                                for (auto& sprite : SpriteSystem::Get().GetActiveSprites())
                                {
                                    if (sprite.isnotspectator() and
                                        (sprite.isinsameteam(SpriteSystem::Get().GetSprite(mysprite)) or
                                         SpriteSystem::Get().GetSprite(mysprite).isspectator()))
                                    {
                                        i = 1;
                                        break;
                                    }
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
                    if (SpriteSystem::Get().GetSprite(i).visible > 0)
                        SpriteSystem::Get().GetSprite(i).visible -= 1;

                lookpoint.x = spritec.skeleton.pos[7].x;
                lookpoint.y = spritec.skeleton.pos[7].y - 2;
                spritec.visible = 45;

                for (auto& sprite : SpriteSystem::Get().GetActiveSprites())
                {
                    // Following sprites
                    if ((spritec.num != camerafollowsprite) and
                        spritec.isnotinsameteam(sprite) && spritec.isnotspectator())
                    {
                        if (checkspritelineofsightvisibility(SpriteSystem::Get().GetSprite(camerafollowsprite),
                                                             sprite))
                            sprite.visible = 45;
                    }
                    else
                    {
                        if (spritec.deadmeat or
                            ((GS::GetGame().isteamgame() && sprite.isinsameteam(spritec)) or
                             ((!GS::GetGame().isteamgame()) && sprite.isnotinsameteam(spritec))))
                        {
                            sprite.visible = 45;
                        }
                        else
                        {
                            if (checkspritelineofsightvisibility(spritec, sprite))
                            {
                                sprite.visible = 45;
                            }
                        }
                    }
                }
            }
        }
#endif
#ifdef SERVER
        controlbot(spritec, GS::GetGame().GetBotPath());
#endif

        if (spritec.deadmeat)
            spritec.freecontrols();
        if (GS::GetGame().GetMapchangecounter() > 0)
        {
            spritec.freecontrols();
        }

        spritec.fired = 0;

        spritec.control.mouseaimx =
            round(spritec.control.mouseaimx + spriteVelocity.x);
        spritec.control.mouseaimy =
            round(spritec.control.mouseaimy + spriteVelocity.y);

        // use weapons
        b.x = 0;
        b.y = 0;

        if (spritec.control.jetpack and
            (((spritec.legsanimation.id == AnimationType::JumpSide) &&
              (((spritec.direction == -1) && spritec.control.right) or
               ((spritec.direction == 1) && spritec.control.left) or playerpressedleftright)) or
             ((spritec.legsanimation.id == AnimationType::RollBack) && spritec.control.up)))
        {
            spritec.bodyapplyanimation(AnimationType::RollBack, 1);
            spritec.legsapplyanimation(AnimationType::RollBack, 1);
        }
        else if (spritec.control.jetpack && (spritec.jetscount > 0))
        {
            if (spritec.onground)
            {
                spriteForces.y = -2.5 * iif(grav > 0.05, jetspeed, grav * 2);
            }
            else
            {
                if (spritec.position != pos_prone)
                    spriteForces.y =
                        spriteForces.y - iif(grav > 0.05, jetspeed, grav * 2);
                else
                    spriteForces.x =
                        spriteForces.x +
                        ((float)(spritec.direction * iif(grav > 0.05, jetspeed, grav * 2)) / 2);
            }

            if ((spritec.legsanimation.id != AnimationType::GetUp) && (spritec.bodyanimation.id != AnimationType::Roll) &&
                (spritec.bodyanimation.id != AnimationType::RollBack))
                spritec.legsapplyanimation(AnimationType::Fall, 1);
#ifndef SERVER
            a.x = spritec.skeleton.pos[1].x - 1;
            a.y = spritec.skeleton.pos[1].y + 3;

            b = vec2subtract(spritec.skeleton.pos[5], spritec.skeleton.pos[4]);
            vec2normalize(b, b);
            vec2scale(b, b, -0.5);

            // smoke
            if (Random(8) == 0)
                createspark(a, spriteVelocity, 1, spritec.num, 75);
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
                createspark(a, spriteVelocity, 1, spritec.num, 75);
            // sparks
            if (Random(7) == 0)
                createspark(a, b, 62, spritec.num, 40);

#endif
            spritec.jetscount -= 1;
#ifndef SERVER
            if ((spritec.jetscount == 1) && spritec.control.jetpack)
                spritec.jetscount = 0;

            // play rockets sound
            playsound(SfxEffect::rocketz, spritePartsPos, spritec.jetssoundchannel);
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
                    if ((spritec.weapon.num != noweapon_num) &&
                        (spritec.weapon.num != knife_num) &&
                        (spritec.weapon.num != chainsaw_num))
                    {
                        for (auto& sprite : SpriteSystem::Get().GetActiveSprites())
                        {
                            if (!sprite.deadmeat and
                                (sprite.position == pos_stand) && (sprite.num != spritec.num) &&
                                sprite.isnotspectator())
                            {
                                auto &secSpritePartsPos = SpriteSystem::Get().GetSpritePartsPos(sprite.num);
                                if (distance(spritePartsPos, secSpritePartsPos) <
                                    melee_dist)
                                    spritec.bodyapplyanimation(AnimationType::Melee, 1);
                            }
                        }
                    }

        // FIRE!!!!
        // (not TargetMode or (SpriteC.Num <> MySprite))
        if (spritec.stat == 0)
        {
            if ((spritec.weapon.num == chainsaw_num) ||
                ((spritec.bodyanimation.id != AnimationType::Roll) &&
                 (spritec.bodyanimation.id != AnimationType::RollBack) &&
                 (spritec.bodyanimation.id != AnimationType::Melee) && (spritec.bodyanimation.id != AnimationType::Change)))
            {
                if (((spritec.bodyanimation.id == AnimationType::HandSupAim) &&
                     (spritec.bodyanimation.currframe == 11)) ||
                    (spritec.bodyanimation.id != AnimationType::HandSupAim))
                {
                    if (spritec.control.fire && (spritec.ceasefirecounter < 0))
                    {
                        if ((spritec.weapon.num == noweapon_num) ||
                            (spritec.weapon.num == knife_num))
                            spritec.bodyapplyanimation(AnimationType::Punch, 1);
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
                                            if (spritec.weapon.num == barrett_num)
                                                playsound(SfxEffect::law_start,
                                                          spritePartsPos,
                                                          spritec.gattlingsoundchannel);

                                            // Minigun wind up
                                            else if (spritec.weapon.num == minigun_num)
                                                playsound(SfxEffect::minigun_start,
                                                          spritePartsPos,
                                                          spritec.gattlingsoundchannel);

                                            // LAW wind up
                                            else if (spritec.weapon.num == law_num)
                                            {
                                                if (spritec.onground and
                                                    (((spritec.legsanimation.id == AnimationType::Crouch) &&
                                                      (spritec.legsanimation.currframe > 13)) ||
                                                     (spritec.legsanimation.id == AnimationType::CrouchRun) ||
                                                     (spritec.legsanimation.id ==
                                                      AnimationType::CrouchRunBack) ||
                                                     ((spritec.legsanimation.id == AnimationType::Prone) &&
                                                      (spritec.legsanimation.currframe > 23))))
                                                    playsound(SfxEffect::law_start,
                                                              spritePartsPos,
                                                              spritec.gattlingsoundchannel);
                                            }
                                        }
#endif
                                        if ((spritec.weapon.num != law_num) ||
                                            ((spritec.onground or spritec.ongroundpermanent) and
                                             (((spritec.legsanimation.id == AnimationType::Crouch) &&
                                               (spritec.legsanimation.currframe > 13)) ||
                                              (spritec.legsanimation.id == AnimationType::CrouchRun) ||
                                              (spritec.legsanimation.id == AnimationType::CrouchRunBack) ||
                                              ((spritec.legsanimation.id == AnimationType::Prone) &&
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
                            if (spritec.weapon.num == minigun_num)
                                // gattling end sound
                                playsound(SfxEffect::minigun_end, spritePartsPos,
                                          spritec.gattlingsoundchannel2);
                            if (spritec.weapon.num == law_num)
                                if ((spritec.onground) and
                                    (((spritec.legsanimation.id == AnimationType::Crouch) &&
                                      (spritec.legsanimation.currframe > 13)) ||
                                     (spritec.legsanimation.id == AnimationType::CrouchRun) ||
                                     (spritec.legsanimation.id == AnimationType::CrouchRunBack) ||
                                     ((spritec.legsanimation.id == AnimationType::Prone) &&
                                      (spritec.legsanimation.currframe > 23))))
                                    // LAW wind down sound
                                    playsound(SfxEffect::law_end, spritePartsPos,
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
        if ((spritec.bodyanimation.id != AnimationType::Roll) && (spritec.bodyanimation.id != AnimationType::RollBack) &&
            (spritec.bonusstyle != bonus_flamegod))
            if (spritec.control.changeweapon)
            {
                spritec.bodyapplyanimation(AnimationType::Change, 1);
#ifndef SERVER
                setsoundpaused(spritec.reloadsoundchannel, true);
#endif
            }

            // clear dont drop flag if needed
#ifndef SERVER
        if (spritec.dontdrop)
            if (!spritec.control.throwweapon or (spritec.weapon.num == knife_num))
                spritec.dontdrop = false;
#endif
        // throw weapon animation
        if (spritec.control.throwweapon && !spritec.control.thrownade && !spritec.dontdrop and
            (spritec.bodyanimation.id != AnimationType::Roll) && (spritec.bodyanimation.id != AnimationType::RollBack) &&
            ((spritec.bodyanimation.id != AnimationType::Change) || (spritec.bodyanimation.currframe > 25)) &&
            (spritec.bonusstyle != bonus_flamegod) && (spritec.weapon.num != bow_num) &&
            (spritec.weapon.num != bow2_num) && (spritec.weapon.num != noweapon_num))
        {
            spritec.bodyapplyanimation(AnimationType::ThrowWeapon, 1);

            if (spritec.weapon.num == knife_num)
                spritec.bodyanimation.speed = 2;

#ifndef SERVER
            stopsound(spritec.reloadsoundchannel);
#endif
        }

        // reload
        if ((spritec.weapon.num == chainsaw_num) ||
            ((spritec.bodyanimation.id != AnimationType::Roll) && (spritec.bodyanimation.id != AnimationType::RollBack) &&
             (spritec.bodyanimation.id != AnimationType::Change)))
        {
            if (spritec.control.reload)
            {
                if (spritec.weapon.ammocount != spritec.weapon.ammo)
                {
                    if (spritec.weapon.num == spas12_num)
                    {
                        if (spritec.weapon.ammocount < spritec.weapon.ammo)
                        {
                            if (spritec.weapon.fireintervalcount == 0)
                                spritec.bodyapplyanimation(AnimationType::Reload, 1);
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
        if ((spritec.bodyanimation.id == AnimationType::Reload) && (spritec.bodyanimation.currframe == 7))
        {
#ifndef SERVER
            playsound(SfxEffect::spas12_reload, spritePartsPos, spritec.reloadsoundchannel);
#endif
            spritec.bodyanimation.currframe += 1;
        }

        if (!spritec.control.fire or (spritec.weapon.ammocount == 0))
            if ((spritec.bodyanimation.id == AnimationType::Reload) && (spritec.bodyanimation.currframe == 14))
            {
                spritec.weapon.ammocount = spritec.weapon.ammocount + 1;
                if (spritec.weapon.ammocount < spritec.weapon.ammo)
                    spritec.bodyanimation.currframe = 1;
            }

        // Change Weapon
        // sound
        if ((spritec.bodyanimation.id == AnimationType::Change) && (spritec.bodyanimation.currframe == 2))
        {
#ifndef SERVER
            if (spritec.secondaryweapon.num == colt_num)
                playsound(SfxEffect::changespin, spritePartsPos);
            else if (spritec.secondaryweapon.num == knife_num)
                playsound(SfxEffect::knife, spritePartsPos);
            else if (spritec.secondaryweapon.num == chainsaw_num)
                playsound(SfxEffect::chainsaw_d, spritePartsPos);
            else
                playsound(SfxEffect::changeweapon, spritePartsPos);
#endif
            spritec.bodyanimation.currframe += 1;
        }

        if ((spritec.bodyanimation.id == AnimationType::Change) && (spritec.bodyanimation.currframe == 25) &&
            (spritec.bonusstyle != bonus_flamegod))
        {
#ifdef SERVER
            if (spritec.player->controlmethod == bot)
#else
            if ((spritec.num == mysprite) || (spritec.player->controlmethod == bot))
#endif
            {
                new (&tempgun) tgun(spritec.weapon);
                spritec.SetFirstWeapon(spritec.secondaryweapon);
                spritec.SetSecondWeapon(tempgun);

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

        if (((spritec.bodyanimation.id == AnimationType::Change) &&
             (spritec.bodyanimation.currframe == AnimationSystem::Get().GetAnimation(AnimationType::Change).numframes)) &&
            (spritec.bonusstyle != bonus_flamegod) && (spritec.weapon.ammocount == 0))
        {
            spritec.bodyapplyanimation(AnimationType::Stand, 1);
#ifndef SERVER
            setsoundpaused(spritec.reloadsoundchannel, false);
#endif
        }

        // Throw away weapon
#ifndef SERVER
        if ((spritec.bodyanimation.id == AnimationType::ThrowWeapon) && (spritec.bodyanimation.currframe == 2))
            playsound(SfxEffect::throwgun, spritePartsPos);
#endif
        if (spritec.weapon.num != knife_num)
            if ((spritec.bodyanimation.id == AnimationType::ThrowWeapon) &&
                (spritec.bodyanimation.currframe == 19) &&
                (spritec.weapon.num != noweapon_num))
            {
                spritec.dropweapon();
                spritec.bodyapplyanimation(AnimationType::Stand, 1);
            }

        // Throw knife
        if ((spritec.weapon.num == knife_num) &&
            (spritec.bodyanimation.id == AnimationType::ThrowWeapon) &&
            (!spritec.control.throwweapon or (spritec.bodyanimation.currframe == 16)))
        {
#ifdef SERVER
            if (spritec.player->controlmethod == bot)
#else
            if ((spritec.player->controlmethod == bot) || (spritec.num == mysprite))
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
                vec2scale(playervelocity, spriteVelocity,
                          guns[thrownknife].inheritedvelocity);

                d = (float)(min(max(spritec.bodyanimation.currframe, 8), 16)) / 16;
                vec2scale(b, b, guns[thrownknife].speed * 1.5 * d);
                b = vec2add(b, playervelocity);
                a = spritec.skeleton.pos[16];
                createbullet(a, b, guns[thrownknife].num, spritec.num, 255,
                             guns[thrownknife].hitmultiply, true, false);
                spritec.applyweaponbynum(noweapon_num, 1);
                spritec.bodyapplyanimation(AnimationType::Stand, 1);
#ifndef SERVER
                if ((spritec.num == mysprite) && !spritec.deadmeat)
                    clientspritesnapshot();
#endif
            }
        }

        // Punch!
        if (!spritec.deadmeat)
            if ((spritec.bodyanimation.id == AnimationType::Punch) && (spritec.bodyanimation.currframe == 11) &&
                (spritec.weapon.num != law_num) && (spritec.weapon.num != m79_num))
            {
                a.x = spritec.skeleton.pos[16].x + 2 * spritec.direction;
                a.y = spritec.skeleton.pos[16].y + 3;
                b.x = spritec.direction * 0.1;
                b.y = 0;
                createbullet(a, b, spritec.weapon.num, spritec.num, 255, spritec.weapon.hitmultiply,
                             true, false);

#ifndef SERVER
                if (spritec.weapon.num == knife_num)
                    playsound(SfxEffect::slash, spritePartsPos);
#endif

                spritec.bodyanimation.currframe += 1;
            }

        // Buttstock!
        if (!spritec.deadmeat)
            if ((spritec.bodyanimation.id == AnimationType::Melee) && (spritec.bodyanimation.currframe == 12))
            {
                a.x = spritec.skeleton.pos[16].x + 2 * spritec.direction;
                a.y = spritec.skeleton.pos[16].y + 3;
                b.x = spritec.direction * 0.1;
                b.y = 0;
                createbullet(a, b, noweapon_num, spritec.num, 255, guns[noweapon].hitmultiply,
                             true, true);

#ifndef SERVER
                playsound(SfxEffect::slash, spritePartsPos);
#endif
            }

        if (spritec.bodyanimation.id == AnimationType::Melee)
            if (spritec.bodyanimation.currframe > 20)
                spritec.bodyapplyanimation(AnimationType::Stand, 1);

        // Shotgun luska
        if ((spritec.bodyanimation.id == AnimationType::Shotgun) && (spritec.bodyanimation.currframe == 24))
        {
#ifndef SERVER
            b = spritec.gethandsaimdirection();
            vec2scale(b, b, spritec.weapon.speed);
            b.x = spritec.direction * 0.025 * b.y + spriteVelocity.x;
            b.y = -spritec.direction * 0.025 * b.x + spriteVelocity.y;
            a.x = spritec.skeleton.pos[15].x + 2 - spritec.direction * 0.015 * b.x;
            a.y = spritec.skeleton.pos[15].y - 2 - spritec.direction * 0.015 * b.y;

            createspark(a, b, 51, spritec.num, 255); // czerwona luska
#endif
            spritec.bodyanimation.currframe += 1;
        }

        // M79 luska
        if ((spritec.weapon.num == m79_num) &&
            (spritec.weapon.reloadtimecount == spritec.weapon.clipouttime))
        {
#ifndef SERVER
            b = spritec.gethandsaimdirection();
            vec2scale(b, b, spritec.weapon.speed);
            b.x = spritec.direction * 0.08 * b.y + spriteVelocity.x;
            b.y = -spritec.direction * 0.08 * b.x + spriteVelocity.y;
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
            if ((spritec.legsanimation.id != AnimationType::GetUp) && (spritec.legsanimation.id != AnimationType::Prone) &&
                (spritec.legsanimation.id != AnimationType::ProneMove))
            {
#ifndef SERVER
                playsound(SfxEffect::goprone, spritePartsPos);
#endif

                spritec.legsapplyanimation(AnimationType::Prone, 1);
                if ((spritec.bodyanimation.id != AnimationType::Reload) &&
                    (spritec.bodyanimation.id != AnimationType::Change) &&
                    (spritec.bodyanimation.id != AnimationType::ThrowWeapon))
                    spritec.bodyapplyanimation(AnimationType::Prone, 1);

                spritec.olddirection = spritec.direction;
                spritec.control.prone = false;
            }
        }

        // Get up
        if (spritec.position == pos_prone)
            if (spritec.control.prone or (spritec.direction != spritec.olddirection))
                if (((spritec.legsanimation.id == AnimationType::Prone) &&
                     (spritec.legsanimation.currframe > 23)) ||
                    (spritec.legsanimation.id == AnimationType::ProneMove))
                {
                    if (spritec.legsanimation.id != AnimationType::GetUp)
                    {
                        spritec.legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::GetUp);
                        spritec.legsanimation.currframe = 9;
                        spritec.control.prone = false;
#ifndef SERVER
                        playsound(SfxEffect::standup, spritePartsPos);
#endif
                    }
                    if ((spritec.bodyanimation.id != AnimationType::Reload) &&
                        (spritec.bodyanimation.id != AnimationType::Change) &&
                        (spritec.bodyanimation.id != AnimationType::ThrowWeapon))
                        spritec.bodyapplyanimation(AnimationType::GetUp, 9);
                }

        unprone = false;
        // Immediately switch from unprone to jump/sidejump, because the end of the unprone
        // animation can be seen as the "wind up" for the jump
        if ((spritec.legsanimation.id == AnimationType::GetUp) &&
            (spritec.legsanimation.currframe > 23 - (4 - 1)) && // Possible during the last 4 frames
            spritec.onground && spritec.control.up and
            (spritec.control.right or spritec.control.left))
        {
            // Set sidejump frame 1 to 4 depending on which unprone frame we're in
            spritec.legsapplyanimation(AnimationType::JumpSide, spritec.legsanimation.currframe - (23 - (4 - 1)));
            unprone = true;
        }
        else if ((spritec.legsanimation.id == AnimationType::GetUp) &&
                 (spritec.legsanimation.currframe >
                  23 - (4 - 1)) && // Possible during the last 4 frames
                 spritec.onground and
                 spritec.control.up && !(spritec.control.right or spritec.control.left))
        {
            // Set jump frame 6 to 9 depending on which unprone frame we're in
            spritec.legsapplyanimation(AnimationType::Jump, spritec.legsanimation.currframe - (23 - (9 - 1)));
            unprone = true;
        }
        else if ((spritec.legsanimation.id == AnimationType::GetUp) && (spritec.legsanimation.currframe > 23))
        {
            if (spritec.control.right or spritec.control.left)
            {
                // Run back or forward depending on facing direction and direction key pressed
                if ((spritec.direction == 1) ^ spritec.control.left)
                    spritec.legsapplyanimation(AnimationType::Run, 1);
                else
                    spritec.legsapplyanimation(AnimationType::RunBack, 1);
            }
            else if (!spritec.onground && spritec.control.up)
                spritec.legsapplyanimation(AnimationType::Run, 1);
            else
                spritec.legsapplyanimation(AnimationType::Stand, 1);

            unprone = true;
        }

        if (unprone)
        {
            spritec.position = pos_stand;

            if ((spritec.bodyanimation.id != AnimationType::Reload) &&
                (spritec.bodyanimation.id != AnimationType::Change) &&
                (spritec.bodyanimation.id != AnimationType::ThrowWeapon))
                spritec.bodyapplyanimation(AnimationType::Stand, 1);
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
        if ((spritec.weapon.num == barrett_num) && (spritec.weapon.fireintervalcount > 0))
            if ((spritec.bodyanimation.id == AnimationType::Stand) || (spritec.bodyanimation.id == AnimationType::Crouch) ||
                (spritec.bodyanimation.id == AnimationType::Prone))
                spritec.bodyapplyanimation(AnimationType::Barret, 1);

        // IDLE
        if (spritec.stat == 0)
        {
            if (((spritec.bodyanimation.id == AnimationType::Stand) &&
                 (spritec.legsanimation.id == AnimationType::Stand) and !spritec.deadmeat and
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
                spritec.bodyapplyanimation(AnimationType::Smoke, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
            }

            if ((spritec.bodyanimation.id == AnimationType::Smoke) && (spritec.bodyanimation.currframe == 17))
            {
#ifndef SERVER
                playsound(SfxEffect::stuff, spritePartsPos);
#endif
                spritec.bodyanimation.currframe += 1;
            }

            if (!spritec.deadmeat)
            {
                if ((spritec.idletime == 1) && (spritec.bodyanimation.id != AnimationType::Smoke) &&
                    (spritec.legsanimation.id == AnimationType::Stand))
                {
#ifndef SERVER
                    a = spritec.skeleton.pos[12];
                    b = spritec.gethandsaimdirection();
                    vec2scale(b, b, 2);
                    createspark(a, b, 32, spritec.num, 245);
                    playsound(SfxEffect::spit, spritePartsPos);
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
                        if (spritec.bodyanimation.id == AnimationType::Stand)
                        {
                            // Step 1/8
                            spritec.bodyapplyanimation(AnimationType::Cigar, 1);
#ifdef SERVER
                            serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                            spritec.idletime = default_idletime;
                        }
                    }
                    else if (spritec.hascigar == 5)
                    {
                        if ((spritec.bodyanimation.id != AnimationType::Smoke) &&
                            (spritec.bodyanimation.id != AnimationType::Cigar))
                        {
                            // Step 4.5/8 (only occurrs if interrupted between step 2 and 5, so redo
                            // step 1)
                            spritec.hascigar = 0;
                            spritec.bodyapplyanimation(AnimationType::Cigar, 1);
                            spritec.idletime = default_idletime;
                        }
                    }
                    else if (spritec.hascigar == 10)
                    {
                        if (spritec.bodyanimation.id != AnimationType::Smoke)
                        {
                            // Step 6/8
                            spritec.bodyapplyanimation(AnimationType::Smoke, 1);
                            spritec.idletime = default_idletime;
                        }
                    }
                }

                if ((spritec.bodyanimation.id == AnimationType::Cigar) &&
                    (spritec.bodyanimation.currframe == 37))
                    if (spritec.hascigar == 5)
                    {
                        // Step 3/8
                        spritec.bodyapplyanimation(AnimationType::Stand, 1);
                        spritec.bodyapplyanimation(AnimationType::Cigar, 1);
                    }

                if ((spritec.bodyanimation.id == AnimationType::Cigar) &&
                    (spritec.bodyanimation.currframe == 9))
                    if (spritec.hascigar == 5)
                    {
                        // Step 4/8
#ifndef SERVER
                        playsound(SfxEffect::match, spritePartsPos);
#endif
                        spritec.bodyanimation.currframe += 1;
                    }

                if ((spritec.bodyanimation.id == AnimationType::Cigar) &&
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
                        playsound(SfxEffect::smoke, spritePartsPos);

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

                if ((spritec.bodyanimation.id == AnimationType::Smoke) &&
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
                    playsound(SfxEffect::smoke, spritePartsPos);
#endif
                    spritec.bodyanimation.currframe += 1;
                }

                if ((spritec.bodyanimation.id == AnimationType::Smoke) &&
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
                spritec.bodyapplyanimation(AnimationType::Wipe, 1);
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
                spritec.bodyapplyanimation(AnimationType::Groin, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
                spritec.idlerandom = -1;
            }
        }
        else if (spritec.idlerandom == 4) // TAKE OFF HELMET
        {
            if ((spritec.weapon.num != bow_num) && (spritec.weapon.num != bow2_num))
            {
                if (spritec.idletime == 0)
                {
                    if (spritec.wearhelmet == 1)
                        spritec.bodyapplyanimation(AnimationType::TakeOff, 1);
                    if (spritec.wearhelmet == 2)
                        spritec.bodyapplyanimation(AnimationType::TakeOff, 10);
#ifdef SERVER
                    serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                    spritec.idletime = default_idletime;
                }

                if (spritec.wearhelmet == 1)
                {
                    if ((spritec.bodyanimation.id == AnimationType::TakeOff) &&
                        (spritec.bodyanimation.currframe == 15))
                    {
                        spritec.wearhelmet = 2;
                        spritec.bodyanimation.currframe += 1;
                    }
                }
                else if (spritec.wearhelmet == 2)
                {
                    if ((spritec.bodyanimation.id == AnimationType::TakeOff) &&
                        (spritec.bodyanimation.currframe == 22))
                    {
                        spritec.bodyapplyanimation(AnimationType::Stand, 1);
                        spritec.idlerandom = -1;
                    }

                    if ((spritec.bodyanimation.id == AnimationType::TakeOff) &&
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
                spritec.bodyapplyanimation(AnimationType::Victory, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;
                spritec.idlerandom = -1;

#ifndef SERVER
                playsound(SfxEffect::roar, spritePartsPos);
#endif
            }
        }
        else if (spritec.idlerandom == 6) // PISS...
        {
            if (spritec.idletime == 0)
            {
                spritec.bodyapplyanimation(AnimationType::Piss, 1);
#ifdef SERVER
                serveridleanimation(spritec.num, spritec.idlerandom);
#endif
                spritec.idletime = default_idletime;

#ifndef SERVER
                playsound(SfxEffect::piss, spritePartsPos);
#endif
            }

            if (spritec.bodyanimation.id == AnimationType::Piss)
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
                    if ((spritec.weapon.num == m79_num) ||
                        (spritec.weapon.num == m249_num) ||
                        (spritec.weapon.num == spas12_num) ||
                        (spritec.weapon.num == law_num) ||
                        (spritec.weapon.num == chainsaw_num) ||
                        (spritec.weapon.num == barrett_num) ||
                        (spritec.weapon.num == minigun_num))
                    {
                        spritec.bodyapplyanimation(AnimationType::Mercy2, 1);
                        spritec.legsapplyanimation(AnimationType::Mercy2, 1);
                    }
                    else if (spritec.weapon.num != minigun_num)
                    {
                        spritec.bodyapplyanimation(AnimationType::Mercy, 1);
                        spritec.legsapplyanimation(AnimationType::Mercy, 1);
                    }

#ifndef SERVER
                    playsound(SfxEffect::mercy, spritePartsPos);
                    if (spritec.weapon.num == minigun_num)
                        playsound(SfxEffect::minigun_start, spritePartsPos);
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

            if ((spritec.bodyanimation.id == AnimationType::Mercy) || (spritec.bodyanimation.id == AnimationType::Mercy2))
            {
                if (spritec.bodyanimation.currframe == 20)
                {
                    spritec.fire();
#ifndef SERVER
                    if (spritec.weapon.num == knife_num)
                        playsound(SfxEffect::slash, spritePartsPos,
                                  SpriteSystem::Get().GetSprite(spritec.num).gattlingsoundchannel);
                    if (spritec.weapon.num == chainsaw_num)
                        playsound(SfxEffect::chainsaw_r, spritePartsPos,
                                  SpriteSystem::Get().GetSprite(spritec.num).gattlingsoundchannel);
                    if (spritec.weapon.num == noweapon_num)
                        playsound(SfxEffect::dead_hit, spritePartsPos,
                                  SpriteSystem::Get().GetSprite(spritec.num).gattlingsoundchannel);

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
                spritec.bodyapplyanimation(AnimationType::Own, 1);
                spritec.legsapplyanimation(AnimationType::Own, 1);
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
            if ((spritec.legsanimation.id == AnimationType::Jump) ||
                (spritec.legsanimation.id == AnimationType::JumpSide) ||
                (spritec.legsanimation.id == AnimationType::Roll) ||
                (spritec.legsanimation.id == AnimationType::RollBack) ||
                (spritec.legsanimation.id == AnimationType::Prone) || (spritec.legsanimation.id == AnimationType::Run) ||
                (spritec.legsanimation.id == AnimationType::RunBack))
            {

                spriteVelocity.x =
                    ((float)(spriteVelocity.x) / spritec.legsanimation.speed);
                spriteVelocity.y =
                    ((float)(spriteVelocity.y) / spritec.legsanimation.speed);
            }

            if (spritec.legsanimation.speed > 2)
                if ((spritec.legsanimation.id == AnimationType::ProneMove) ||
                    (spritec.legsanimation.id == AnimationType::CrouchRun))
                {
                    spriteVelocity.x =
                        ((float)(spriteVelocity.x) /
                         spritec.legsanimation.speed);
                    spriteVelocity.y =
                        ((float)(spriteVelocity.y) /
                         spritec.legsanimation.speed);
                }
        }

        // stat gun deactivate if needed
        if (spritec.control.up or spritec.control.jetpack)
            if (spritec.stat > 0)
            {
                things[spritec.stat].statictype = false;
                spritec.stat = 0;
            }

        // AimDistCoef (Sniper view)
        if (spritec.weapon.num == barrett_num)
        {
            if ((spritec.weapon.fireintervalcount == 0) &&
                ((spritec.bodyanimation.id == AnimationType::Prone) || (spritec.bodyanimation.id == AnimationType::Aim)))
            {
                if ((fabs(spritec.control.mouseaimx - spritePartsPos.x) >=
                     640 / 1.035) ||
                    (fabs(spritec.control.mouseaimy - spritePartsPos.y) >=
                     480 / 1.035))
                {
                    if (spritec.aimdistcoef == defaultaimdist)
                    {
#ifndef SERVER
                        playsound(SfxEffect::scope, spritePartsPos);
#else
                        serverspritedeltasmouse(spritec.num);
#endif
                    }

                    if (spritec.bodyanimation.id == AnimationType::Prone)
                        if (spritec.aimdistcoef > sniperaimdist)
                        {
                            spritec.aimdistcoef = spritec.aimdistcoef - aimdistincr;
                            if (maintickcounter % 27 == 0)
                            {
#ifndef SERVER
                                playsound(SfxEffect::scoperun, spritePartsPos);
#else
                                serverspritedeltasmouse(spritec.num);
#endif
                            }
                        }

                    if (spritec.bodyanimation.id == AnimationType::Aim)
                        if (spritec.aimdistcoef > crouchaimdist)
                        {
                            spritec.aimdistcoef = spritec.aimdistcoef - 2 * aimdistincr;
                            if (maintickcounter % 27 == 0)
                            {
#ifndef SERVER
                                playsound(SfxEffect::scoperun, spritePartsPos);
#else
                                serverspritedeltasmouse(spritec.num);
#endif
                            }
                        }
                }

                if ((fabs(spritec.control.mouseaimx - spritePartsPos.x) <
                     640 / 1.5) &&
                    (fabs(spritec.control.mouseaimy - spritePartsPos.y) < 480 / 1.5))
                {
                    if (spritec.aimdistcoef < defaultaimdist)
                    {
                        spritec.aimdistcoef = spritec.aimdistcoef + aimdistincr;
#ifndef SERVER
                        if (spritec.aimdistcoef == defaultaimdist)
                            playsound(SfxEffect::scope, spritePartsPos);
#endif
                        if (maintickcounter % 27 == 0)
                        {
#ifndef SERVER
                            playsound(SfxEffect::scoperun, spritePartsPos);
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
                    playsound(SfxEffect::scopeback, spritePartsPos);
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
            if (GS::GetGame().isteamgame())
            {
                for(auto& sprite : SpriteSystem::Get().GetActiveSprites())
                {
                    if (sprite.isinsameteam(spritec) and
                        (sprite.position == pos_crouch) && (sprite.num != spritec.num) &&
                        spritec.isnotspectator())
                    {
                        auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(sprite.num);
                        a = spritePartsPos;

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
            }
        }
#ifndef SERVER
        if (targetmode && (spritec.num == mysprite))
        {
            spritec.freecontrols();
        }
#endif
        // End any ongoing idle animations if a key is pressed
        if ((spritec.bodyanimation.id == AnimationType::Cigar) || (spritec.bodyanimation.id == AnimationType::Match) ||
            (spritec.bodyanimation.id == AnimationType::Smoke) || (spritec.bodyanimation.id == AnimationType::Wipe) ||
            (spritec.bodyanimation.id == AnimationType::Groin))
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
        if ((spritec.bodyanimation.id != AnimationType::TakeOff) && (spritec.bodyanimation.id != AnimationType::Piss) &&
            (spritec.bodyanimation.id != AnimationType::Mercy) && (spritec.bodyanimation.id != AnimationType::Mercy2) &&
            (spritec.bodyanimation.id != AnimationType::Victory) && (spritec.bodyanimation.id != AnimationType::Own))
        {
            if ((spritec.bodyanimation.id == AnimationType::Roll) || (spritec.bodyanimation.id == AnimationType::RollBack))
            {
                if (spritec.legsanimation.id == AnimationType::Roll)
                {
                    if (spritec.onground) // if staying on ground
                        spriteForces.x = spritec.direction * rollspeed;
                    else
                        spriteForces.x = spritec.direction * 2 * flyspeed;
                }
                else if (spritec.legsanimation.id == AnimationType::RollBack)
                {
                    if (spritec.onground) // if staying on ground
                        spriteForces.x = -spritec.direction * rollspeed;
                    else
                        spriteForces.x = -spritec.direction * 2 * flyspeed;

                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 1) &&
                        (spritec.legsanimation.currframe < 8))
                    {
                        if (spritec.control.up)
                        {
                            spriteForces.y =
                                spriteForces.y - jumpdirspeed * 1.5;
                            spriteForces.x =
                                spriteForces.x * 0.5;
                            spriteVelocity.x =
                                spriteVelocity.x * 0.8;
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
                    if ((spritec.legsanimation.id == AnimationType::Run) ||
                        (spritec.legsanimation.id == AnimationType::RunBack) ||
                        (spritec.legsanimation.id == AnimationType::Fall) ||
                        (spritec.legsanimation.id == AnimationType::ProneMove) ||
                        ((spritec.legsanimation.id == AnimationType::Prone) &&
                         (spritec.legsanimation.currframe >= 24)))
                    {
                        if ((spritec.legsanimation.id == AnimationType::ProneMove) ||
                            ((spritec.legsanimation.id == AnimationType::Prone) &&
                             (spritec.legsanimation.currframe == spritec.legsanimation.numframes)))
                        {
                            spritec.control.prone = false;
                            spritec.position = pos_stand;
                        }
#ifndef SERVER
                        if ((spritec.legsanimation.id != AnimationType::RollBack) &&
                            (spritec.legsanimation.id != AnimationType::Roll))
                            playsound(SfxEffect::roll, spritePartsPos);

                        setsoundpaused(spritec.reloadsoundchannel, true);
#endif

                        if (spritec.direction == 1)
                        {
                            spritec.bodyapplyanimation(AnimationType::Roll, 1);
                            spritec.legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::Roll);
                            spritec.legsanimation.currframe = 1;
                        }
                        else
                        {
                            spritec.bodyapplyanimation(AnimationType::RollBack, 1);
                            spritec.legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::RollBack);
                            spritec.legsanimation.currframe = 1;
                        }
                    }
                    else
                    {
                        if (spritec.direction == 1)
                            spritec.legsapplyanimation(AnimationType::CrouchRun, 1);
                        else
                            spritec.legsapplyanimation(AnimationType::CrouchRunBack, 1);
                    }

                    if ((spritec.legsanimation.id == AnimationType::CrouchRun) ||
                        (spritec.legsanimation.id == AnimationType::CrouchRunBack))
                        spriteForces.x = crouchrunspeed;
                    else if ((spritec.legsanimation.id == AnimationType::Roll) ||
                             (spritec.legsanimation.id == AnimationType::RollBack))
                        spriteForces.x = 2 * crouchrunspeed;
                }
            }
            // downleft
            else if (spritec.control.left && spritec.control.down)
            {
                if (spritec.onground) // if staying on ground
                {
                    // roll to the side
                    if ((spritec.legsanimation.id == AnimationType::Run) ||
                        (spritec.legsanimation.id == AnimationType::RunBack) ||
                        (spritec.legsanimation.id == AnimationType::Fall) ||
                        (spritec.legsanimation.id == AnimationType::ProneMove) ||
                        ((spritec.legsanimation.id == AnimationType::Prone) &&
                         (spritec.legsanimation.currframe >= 24)))
                    {
                        if ((spritec.legsanimation.id == AnimationType::ProneMove) ||
                            ((spritec.legsanimation.id == AnimationType::Prone) &&
                             (spritec.legsanimation.currframe == spritec.legsanimation.numframes)))
                        {
                            spritec.control.prone = false;
                            spritec.position = pos_stand;
                        }
#ifndef SERVER
                        if ((spritec.legsanimation.id != AnimationType::RollBack) &&
                            (spritec.legsanimation.id != AnimationType::Roll))
                            playsound(SfxEffect::roll, spritePartsPos);

                        setsoundpaused(spritec.reloadsoundchannel, true);
#endif

                        if (spritec.direction == 1)
                        {
                            spritec.bodyapplyanimation(AnimationType::RollBack, 1);
                            spritec.legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::RollBack);
                            spritec.legsanimation.currframe = 1;
                        }
                        else
                        {
                            spritec.bodyapplyanimation(AnimationType::Roll, 1);
                            spritec.legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::Roll);
                            spritec.legsanimation.currframe = 1;
                        }
                    }
                    else
                    {
                        if (spritec.direction == 1)
                            spritec.legsapplyanimation(AnimationType::CrouchRunBack, 1);
                        else
                            spritec.legsapplyanimation(AnimationType::CrouchRun, 1);
                    }

                    if ((spritec.legsanimation.id == AnimationType::CrouchRun) ||
                        (spritec.legsanimation.id == AnimationType::CrouchRunBack))
                        spriteForces.x = -crouchrunspeed;
                }
            }
            // Proning
            // FIXME(skoskav): The "and Body <> Throw|Punch" check is to keep the grenade tap and
            // punch/stab prone cancel bugs
            else if ((spritec.legsanimation.id == AnimationType::Prone) ||
                     (spritec.legsanimation.id == AnimationType::ProneMove) ||
                     ((spritec.legsanimation.id == AnimationType::GetUp) &&
                      (spritec.bodyanimation.id != AnimationType::Throw) &&
                      (spritec.bodyanimation.id != AnimationType::Punch)))
            {
                if (spritec.onground)
                {
                    if (((spritec.legsanimation.id == AnimationType::Prone) &&
                         (spritec.legsanimation.currframe > 25)) ||
                        (spritec.legsanimation.id == AnimationType::ProneMove))
                    {
                        if (spritec.control.left or spritec.control.right)
                        {
                            if ((spritec.legsanimation.currframe < 4) ||
                                (spritec.legsanimation.currframe > 14))
                                spriteForces.x =
                                    iif(spritec.control.left, -pronespeed, pronespeed);

                            spritec.legsapplyanimation(AnimationType::ProneMove, 1);
                            if ((spritec.bodyanimation.id != AnimationType::ClipIn) &&
                                (spritec.bodyanimation.id != AnimationType::ClipOut) &&
                                (spritec.bodyanimation.id != AnimationType::SlideBack) &&
                                (spritec.bodyanimation.id != AnimationType::Reload) &&
                                (spritec.bodyanimation.id != AnimationType::Change) &&
                                (spritec.bodyanimation.id != AnimationType::Throw) &&
                                (spritec.bodyanimation.id != AnimationType::ThrowWeapon))
                                spritec.bodyapplyanimation(AnimationType::ProneMove, 1);

                            if (spritec.legsanimation.id != AnimationType::ProneMove)
                                spritec.legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::ProneMove);
                        }
                        else
                        {
                            if (spritec.legsanimation.id != AnimationType::Prone)
                                spritec.legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::Prone);
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
                    if ((spritec.legsanimation.id == AnimationType::Run) ||
                        (spritec.legsanimation.id == AnimationType::RunBack) ||
                        (spritec.legsanimation.id == AnimationType::Stand) ||
                        (spritec.legsanimation.id == AnimationType::Crouch) ||
                        (spritec.legsanimation.id == AnimationType::CrouchRun) ||
                        (spritec.legsanimation.id == AnimationType::CrouchRunBack))
                    {
                        spritec.legsapplyanimation(AnimationType::JumpSide, 1);
#ifndef SERVER
                        playsound(SfxEffect::jump, spritePartsPos);
#endif
                    }

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(AnimationType::Run, 1);
                }
                else if ((spritec.legsanimation.id == AnimationType::Roll) ||
                         (spritec.legsanimation.id == AnimationType::RollBack))
                {
                    if (spritec.direction == 1)
                        spritec.legsapplyanimation(AnimationType::Run, 1);
                    else
                        spritec.legsapplyanimation(AnimationType::RunBack, 1);
                }

                if (spritec.legsanimation.id == AnimationType::Jump)
                {
                    if (spritec.legsanimation.currframe < 10)
                    {
                        spritec.legsapplyanimation(AnimationType::JumpSide, 1);
                    }
                }

                if (spritec.legsanimation.id == AnimationType::JumpSide)
                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 3) &&
                        (spritec.legsanimation.currframe < 11))
                    {
                        spriteForces.x = jumpdirspeed;
                        spriteForces.y = -jumpdirspeed / 1.2;
                    }
            }
            // upleft
            else if (spritec.control.left && spritec.control.up)
            {
                if (spritec.onground) // if staying on ground
                {
                    // jump to the side
                    if ((spritec.legsanimation.id == AnimationType::Run) ||
                        (spritec.legsanimation.id == AnimationType::RunBack) ||
                        (spritec.legsanimation.id == AnimationType::Stand) ||
                        (spritec.legsanimation.id == AnimationType::Crouch) ||
                        (spritec.legsanimation.id == AnimationType::CrouchRun) ||
                        (spritec.legsanimation.id == AnimationType::CrouchRunBack))
                    {
                        spritec.legsapplyanimation(AnimationType::JumpSide, 1);
#ifndef SERVER
                        playsound(SfxEffect::jump, spritePartsPos);
#endif
                    }

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(AnimationType::Run, 1);
                }
                else if ((spritec.legsanimation.id == AnimationType::Roll) ||
                         (spritec.legsanimation.id == AnimationType::RollBack))
                {
                    if (spritec.direction == -1)
                        spritec.legsapplyanimation(AnimationType::Run, 1);
                    else
                        spritec.legsapplyanimation(AnimationType::RunBack, 1);
                }

                if (spritec.legsanimation.id == AnimationType::Jump)
                {
                    if (spritec.legsanimation.currframe < 10)
                    {
                        spritec.legsapplyanimation(AnimationType::JumpSide, 1);
                    }
                }

                if (spritec.legsanimation.id == AnimationType::JumpSide)
                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 3) &&
                        (spritec.legsanimation.currframe < 11))
                    {
                        spriteForces.x = -jumpdirspeed;
                        spriteForces.y = -jumpdirspeed / 1.2;
                    }
            }
            // up
            else if (spritec.control.up)
            {
                if (spritec.onground) // if staying on ground
                {
                    if (spritec.legsanimation.id != AnimationType::Jump)
                    {
                        spritec.legsapplyanimation(AnimationType::Jump, 1);
#ifndef SERVER
                        playsound(SfxEffect::jump, spritePartsPos);
#endif
                    }

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(AnimationType::Stand, 1);
                }

                if (spritec.legsanimation.id == AnimationType::Jump)
                {
                    // if appropriate frames to move
                    if ((spritec.legsanimation.currframe > 8) &&
                        (spritec.legsanimation.currframe < 15))
                        spriteForces.y = -jumpspeed;

                    if (spritec.legsanimation.currframe == spritec.legsanimation.numframes)
                        spritec.legsapplyanimation(AnimationType::Fall, 1);
                }
            }
            // down
            else if (spritec.control.down)
            {
                if (spritec.onground) // if staying on ground
                {
#ifndef SERVER
                    if ((spritec.legsanimation.id != AnimationType::CrouchRun) &&
                        (spritec.legsanimation.id != AnimationType::CrouchRunBack) &&
                        (spritec.legsanimation.id != AnimationType::Crouch))
                        playsound(SfxEffect::crouch, spritePartsPos);
#endif

                    spritec.legsapplyanimation(AnimationType::Crouch, 1);
                }
            }
            // right
            else if (spritec.control.right)
            {
                if (spritec.para == 0)
                {
                    if (spritec.direction == 1)
                        spritec.legsapplyanimation(AnimationType::Run, 1);
                    else
                        spritec.legsapplyanimation(AnimationType::RunBack, 1);
                }
                else if (spritec.holdedthing != 0)
                {
                    // parachute bend
                    things[spritec.holdedthing].skeleton.forces[3].y =
                        things[spritec.holdedthing].skeleton.forces[3].y - 0.5;
                    things[spritec.holdedthing].skeleton.forces[2].y =
                        things[spritec.holdedthing].skeleton.forces[2].y + 0.5;
                }

                if (spritec.onground) // if staying on ground
                {
                    spriteForces.x = runspeed;
                    spriteForces.y = -runspeedup;
                }
                else
                    spriteForces.x = flyspeed;
            }
            // left
            else if (spritec.control.left)
            {
                if (spritec.para == 0)
                {
                    if (spritec.direction == -1)
                        spritec.legsapplyanimation(AnimationType::Run, 1);
                    else
                        spritec.legsapplyanimation(AnimationType::RunBack, 1);
                }
                else if (spritec.holdedthing != 0)
                {
                    // parachute bend
                    things[spritec.holdedthing].skeleton.forces[2].y =
                        things[spritec.holdedthing].skeleton.forces[2].y - 0.5;
                    things[spritec.holdedthing].skeleton.forces[3].y =
                        things[spritec.holdedthing].skeleton.forces[3].y + 0.5;
                }

                if (spritec.onground) // if staying on ground
                {
                    spriteForces.x = -runspeed;
                    spriteForces.y = -runspeedup;
                }
                else
                    spriteForces.x = -flyspeed;
            }
            // else all keys not pressed
            else
            {
                if (spritec.onground) // if staying on ground
                {
#ifndef SERVER
                    if (!spritec.deadmeat)
                        if (spritec.legsanimation.id != AnimationType::Stand)
                            playsound(SfxEffect::stop, spritePartsPos);
#endif
                    spritec.legsapplyanimation(AnimationType::Stand, 1);
                }
                else
                    spritec.legsapplyanimation(AnimationType::Fall, 1);
            }
        }

        // Body animations

        // reloading
        if ((spritec.weapon.reloadtimecount == spritec.weapon.clipouttime) &&
            (spritec.bodyanimation.id != AnimationType::Reload) && (spritec.bodyanimation.id != AnimationType::ReloadBow) &&
            (spritec.bodyanimation.id != AnimationType::Roll) && (spritec.bodyanimation.id != AnimationType::RollBack))
            spritec.bodyapplyanimation(AnimationType::ClipIn, 1);
        if (spritec.weapon.reloadtimecount == spritec.weapon.clipintime)
            spritec.bodyapplyanimation(AnimationType::SlideBack, 1);

        // this piece of code fixes the infamous crouch bug
        // how you ask? well once upon time soldat's code decided that
        // randomly the animation for roll for the body and legs will magically
        // go out of sync, which is causing the crouch bug. so this piece of
        // awesome code simply syncs them when they go out of sync <3
        if ((spritec.legsanimation.id == AnimationType::Roll) && (spritec.bodyanimation.id != AnimationType::Roll))
            spritec.bodyapplyanimation(AnimationType::Roll, 1);
        if ((spritec.bodyanimation.id == AnimationType::Roll) && (spritec.legsanimation.id != AnimationType::Roll))
            spritec.legsapplyanimation(AnimationType::Roll, 1);
        if ((spritec.legsanimation.id == AnimationType::RollBack) && (spritec.bodyanimation.id != AnimationType::RollBack))
            spritec.bodyapplyanimation(AnimationType::RollBack, 1);
        if ((spritec.bodyanimation.id == AnimationType::RollBack) && (spritec.legsanimation.id != AnimationType::RollBack))
            spritec.legsapplyanimation(AnimationType::RollBack, 1);

        if ((spritec.bodyanimation.id == AnimationType::Roll) || (spritec.bodyanimation.id == AnimationType::RollBack))
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
        if (((spritec.bodyanimation.id == AnimationType::Roll) || (spritec.bodyanimation.id == AnimationType::RollBack)) &&
            (spritec.bodyanimation.currframe == spritec.bodyanimation.numframes))
        {
            // Was probably a roll
            if (spritec.onground)
            {
                if (spritec.control.down)
                {
                    if (spritec.control.left or spritec.control.right)
                    {
                        if (spritec.bodyanimation.id == AnimationType::Roll)
                            spritec.legsapplyanimation(AnimationType::CrouchRun, 1);
                        else
                            spritec.legsapplyanimation(AnimationType::CrouchRunBack, 1);
                    }
                    else
                        spritec.legsapplyanimation(AnimationType::Crouch, 15);
                }
            }
            // Was probably a backflip
            else if ((spritec.bodyanimation.id == AnimationType::RollBack) && spritec.control.up)
            {
                if (spritec.control.left or spritec.control.right)
                {
                    // Run back or forward depending on facing direction and direction key pressed
                    if ((spritec.direction == 1) ^ spritec.control.left)
                        spritec.legsapplyanimation(AnimationType::Run, 1);
                    else
                        spritec.legsapplyanimation(AnimationType::RunBack, 1);
                }
                else
                    spritec.legsapplyanimation(AnimationType::Fall, 1);
            }
            // Was probably a roll (that ended mid-air)
            else if (spritec.control.down)
            {
                if (spritec.control.left or spritec.control.right)
                {
                    if (spritec.bodyanimation.id == AnimationType::Roll)
                        spritec.legsapplyanimation(AnimationType::CrouchRun, 1);
                    else
                        spritec.legsapplyanimation(AnimationType::CrouchRunBack, 1);
                }
                else
                    spritec.legsapplyanimation(AnimationType::Crouch, 15);
            }

            spritec.bodyapplyanimation(AnimationType::Stand, 1);
        }

        if (spritec.weapon.ammocount > 0)
            if ((!spritec.control.thrownade && (spritec.bodyanimation.id != AnimationType::Recoil) &&
                 (spritec.bodyanimation.id != AnimationType::SmallRecoil) &&
                 (spritec.bodyanimation.id != AnimationType::AimRecoil) &&
                 (spritec.bodyanimation.id != AnimationType::HandSupRecoil) &&
                 (spritec.bodyanimation.id != AnimationType::Shotgun) &&
                 (spritec.bodyanimation.id != AnimationType::Barret) &&
                 (spritec.bodyanimation.id != AnimationType::Change) &&
                 (spritec.bodyanimation.id != AnimationType::ThrowWeapon) &&
                 (spritec.bodyanimation.id != AnimationType::WeaponNone) &&
                 (spritec.bodyanimation.id != AnimationType::Punch) && (spritec.bodyanimation.id != AnimationType::Roll) &&
                 (spritec.bodyanimation.id != AnimationType::RollBack) &&
                 (spritec.bodyanimation.id != AnimationType::ReloadBow) &&
                 (spritec.bodyanimation.id != AnimationType::Cigar) && (spritec.bodyanimation.id != AnimationType::Match) &&
                 (spritec.bodyanimation.id != AnimationType::Smoke) && (spritec.bodyanimation.id != AnimationType::Wipe) &&
                 (spritec.bodyanimation.id != AnimationType::TakeOff) &&
                 (spritec.bodyanimation.id != AnimationType::Groin) && (spritec.bodyanimation.id != AnimationType::Piss) &&
                 (spritec.bodyanimation.id != AnimationType::Mercy) &&
                 (spritec.bodyanimation.id != AnimationType::Mercy2) &&
                 (spritec.bodyanimation.id != AnimationType::Victory) && (spritec.bodyanimation.id != AnimationType::Own) &&
                 (spritec.bodyanimation.id != AnimationType::Reload) &&
                 (spritec.bodyanimation.id != AnimationType::Prone) && (spritec.bodyanimation.id != AnimationType::GetUp) &&
                 (spritec.bodyanimation.id != AnimationType::ProneMove) &&
                 (spritec.bodyanimation.id != AnimationType::Melee)) ||
                ((spritec.bodyanimation.currframe == spritec.bodyanimation.numframes) &&
                 (spritec.bodyanimation.id != AnimationType::Prone)) ||
                ((spritec.weapon.fireintervalcount == 0) &&
                 (spritec.bodyanimation.id == AnimationType::Barret)))
            {
                if (spritec.position != pos_prone)
                {
                    if (spritec.position == pos_stand)
                        spritec.bodyapplyanimation(AnimationType::Stand, 1);

                    if (spritec.position == pos_crouch)
                    {
                        if (spritec.colliderdistance < 255)
                        {
                            if (spritec.bodyanimation.id == AnimationType::HandSupRecoil)
                                spritec.bodyapplyanimation(AnimationType::HandSupAim, 11);
                            else
                                spritec.bodyapplyanimation(AnimationType::HandSupAim, 1);
                        }
                        else
                        {
                            if (spritec.bodyanimation.id == AnimationType::AimRecoil)
                                spritec.bodyapplyanimation(AnimationType::Aim, 6);
                            else
                                spritec.bodyapplyanimation(AnimationType::Aim, 1);
                        }
                    }
                }
                else
                    spritec.bodyapplyanimation(AnimationType::Prone, 26);
            }

        if ((spritec.legsanimation.id == AnimationType::Crouch) || (spritec.legsanimation.id == AnimationType::CrouchRun) ||
            (spritec.legsanimation.id == AnimationType::CrouchRunBack))
            spritec.position = pos_crouch;
        else
            spritec.position = pos_stand;

        if ((spritec.legsanimation.id == AnimationType::Prone) || (spritec.legsanimation.id == AnimationType::ProneMove))
            spritec.position = pos_prone;

#ifndef SERVER
        if (clientstopmovingcounter < 1)
            spritec.freecontrols();
#endif
    }
    break;
    }
}
