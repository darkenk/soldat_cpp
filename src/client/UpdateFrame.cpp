// automatically converted

#include "UpdateFrame.hpp"

#include "Client.hpp"
#include "ClientGame.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "GameStrings.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "WeatherEffects.hpp"
#include "shared/Calc.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include <cstdint>

std::int32_t idlecounter, oldmousex;

void update_frame()
{
    std::int32_t j;
    tvector2 norm, camv, s, m, p;
    float displayratio;
    std::string screenfile;

    NotImplemented(NITag::GAME);
#if 0
    checksynchronize;
#endif

#ifdef ENABLE_FAE
    faeontick;
#endif

    cameraprev.x = camerax;
    cameraprev.y = cameray;
    mouseprev.x = mx;
    mouseprev.y = my;

    if (mapchangecounter < 0)
    {
        if (demoplayer.active() && escmenu->active)
            return;

        for (j = 1; j <= max_sprites; j++)
            if (sprite[j].active)
                if (sprite[j].isnotspectator())
                    if (clientstopmovingcounter > 0)
                        spriteparts.doeulertimestepfor(j); // integrate sprite particles

        for (j = 1; j <= max_sprites; j++)
            if (sprite[j].active)
                sprite[j].update(); // update sprite

        // Bullets update
        for (j = 1; j <= max_bullets; j++)
        {
            if (bullet[j].active)
                bullet[j].update();

            if (bullet[j].pingadd > 0)
                bullet[j].pingadd -= 4;
        }

        bulletparts.doeulertimestep();

        sparkscount = 0;
        for (j = 1; j <= max_sparks; j++)
            if (spark[j].active)
            {
                spark[j].update();
                sparkscount += 1;
            }

        // update Things
        for (j = 1; j <= max_things; j++)
            if (thing[j].active)
                thing[j].update();

        if (maintickcounter % second == 0)
            if (screencounter != 255)
                // TODO: don't rely on underflow
                screencounter = 0xff & (screencounter - 1);

        // Change spectate target away from dead player
        if (maintickcounter % (second * 5) == 0)
            if ((camerafollowsprite > 0) && sprite[camerafollowsprite].deadmeat and
                (CVar::sv_realisticmode) && (CVar::sv_survivalmode) && !survivalendround)
            {
                camerafollowsprite = getcameratarget(0);
            }

        // Weather effects
        if (CVar::r_weathereffects)
            switch (map.weather)
            {
            case 1:
                makerain();
                break;
            case 2:
                makesandstorm();
                break;
            case 3:
                makesnow();
                break;
            }
    } // mapchangecounter < 0
    else
    {
        ;
        // allow camera switching in demos while paused
        // if DemoPlay then
        //  for j := 1 to MAX_SPRITES do
        //   if Sprite[j].Active then
        //    ControlSprite(Sprite[j]);
    }

#ifdef STEAM
    if (voicespeakingnow)
        getmicdata;
#endif

    // >> cursor on player <<
    cursortext = "";
    cursorfriendly = false;

    // TODO(helloer): While watching demos this code needs to use SpectNumber instead of MySprite
    if ((mysprite > 0) && (!demoplayer.active()))
        for (j = 1; j <= max_sprites; j++)
            if (sprite[j].active && sprite[j].isnotspectator() && (j != mysprite) &&
                (sprite[j].bonusstyle != bonus_predator) and
                ((sprite[j].position == pos_stand) or
                 (sprite[j].isnotsolo() && sprite[j].isinsameteam(sprite[mysprite])) or
                 sprite[mysprite].deadmeat or sprite[j].deadmeat) and
                ((sprite[j].visible > 40) or (!CVar::sv_realisticmode)))
            {
                if (distance(-gamewidthhalf + camerax + mx, -gameheighthalf + cameray + my,
                             spriteparts.pos[j].x, spriteparts.pos[j].y) < cursorsprite_distance)
                {
                    cursortext = sprite[j].player->name;
                    if (isteamgame())
                        if (sprite[j].isinsameteam(sprite[mysprite]))
                        {
                            cursortext =
                                cursortext + ' ' +
                                inttostr(round(((float)(sprite[j].health) / starthealth) * 100)) +
                                '%';
                            cursorfriendly = true;
                        }

                    break;
                }
            }
    cursortextlength = length(cursortext);

    // bullet timer
    if (bullettimetimer > -1)
        bullettimetimer -= 1;

    if (bullettimetimer == 0)
    {
        togglebullettime(false);
        bullettimetimer = -1;
    }
    else if (bullettimetimer < 1)
    {
        // MapChange counter update
        if ((mapchangecounter > -60) && (mapchangecounter < 99999999))
            mapchangecounter = mapchangecounter - 1;
        if ((mapchangecounter < 0) && (mapchangecounter > -59))
        {
            if (mapchangename != "EXIT*!*")
            {
                changemap();
                resetweaponstats();
            }
        }

        // Game Stats save
        if ((maintickcounter % CVar::log_filesupdate) == 0)
        {
            if (CVar::log_enable)
            {
                if (checkfilesize(consolelogfilename) > max_logfilesize)
                    newlogfiles();

                writelogfile(gamelog, consolelogfilename);
            }
        }

        if (maintickcounter % (second * 6) == 0)
        {
            if (playersnum == 0)
                if (mapchangecounter > 99999999)
                    mapchangecounter = -60;
        }

        sinuscounter = sinuscounter + iluminatespeed;

        if (grenadeeffecttimer > -1)
            grenadeeffecttimer = grenadeeffecttimer - 1;

        // Spray counter
        if (hitspraycounter > 0)
            hitspraycounter -= 1;

        // Idle counter
        if (mysprite > 0)
            if (mapchangecounter < 99999999)
                if (sprite[mysprite].isnotspectator() && (!sprite[mysprite].player->demoplayer))
                {
                    if (oldmousex - round(mx) == 0)
                        idlecounter += 1;
                    else
                        idlecounter = 0;

                    if (idlecounter > idle_kick)
                    {
                        clientdisconnect();
                        exittomenu();
                    }

                    oldmousex = round(mx);
                }

        // Time Limit decrease
        if (mapchangecounter < 99999999)
            if (timelimitcounter > 0)
                timelimitcounter = timelimitcounter - 1;

        timeleftmin = timelimitcounter / 3600;
        timeleftsec = (timelimitcounter - timeleftmin * 3600) / 60;

        if (timelimitcounter > 0)
            if (timelimitcounter < 601)
            {
                if (timelimitcounter % 60 == 0)
                    if (mapchangecounter == -60)
                    {
                        GetMainConsole().console(_("Time Left:") + ' ' +
                                                     (inttostr(timelimitcounter / 60)) + ' ' +
                                                     _("seconds"),
                                                 game_message_color);
                        playsound(sfx_signal);
                    }
            }
            else if (timelimitcounter < 3601)
            {
                if (timelimitcounter % 600 == 0)
                {
                    GetMainConsole().console(_("Time Left:") + ' ' +
                                                 (inttostr(timelimitcounter / 60)) + ' ' +
                                                 _("seconds"),
                                             game_message_color);
                    playsound(sfx_signal);
                }
            }
            else if (timelimitcounter < 18001)
            {
                if (timelimitcounter % 3600 == 0)
                {
                    GetMainConsole().console(_("Time Left:") + ' ' +
                                                 (inttostr(timelimitcounter / 3600)) + ' ' +
                                                 _("minutes"),
                                             game_message_color);
                    playsound(sfx_signal);
                }
            }
            else if (timelimitcounter % 18000 == 0)
            {
                GetMainConsole().console(_("Time Left:") + ' ' +
                                             (inttostr(timelimitcounter / 3600)) + ' ' +
                                             _("minutes"),
                                         game_message_color);
                playsound(sfx_signal);
            }

        // Map voting timer
        timervote();

        // Chat Update
        for (j = 1; j <= max_sprites; j++)
            if (chatdelay[j] > 0)
                chatdelay[j] = chatdelay[j] - 1;

        // Big and World Message update
        for (j = 0; j < max_big_messages; j++)
        {
            if (bigdelay[j] > 0)
                bigdelay[j] = bigdelay[j] - 1;
            if (worlddelay[j] > 0)
                worlddelay[j] = worlddelay[j] - 1;
        }

        // Shot dist update
        if (shotdistanceshow > 0)
            shotdistanceshow = shotdistanceshow - 1;

        // Consoles Update
        GetMainConsole().scrolltick = GetMainConsole().scrolltick + 1;
        if (GetMainConsole().scrolltick == GetMainConsole().scrolltickmax)
            GetMainConsole().scrollconsole();

        if (GetMainConsole().alphacount > 0)
            GetMainConsole().alphacount = GetMainConsole().alphacount - 1;

        GetKillConsole().scrolltick = GetKillConsole().scrolltick + 1;
        if (GetKillConsole().scrolltick == GetKillConsole().scrolltickmax)
        {
            GetKillConsole().scrollconsole();
            if ((GetKillConsole().count > 0) &&
                (GetKillConsole().nummessage[GetKillConsole().count] == -255))
                GetKillConsole().scrollconsole();
        }

        if (chattimecounter > 0)
            chattimecounter = chattimecounter - 1;
    } // bullettime off

    // MOVE -=CAMERA=-
    if ((camerafollowsprite > 0) && (camerafollowsprite < max_sprites + 1))
    {
        if (sprite[camerafollowsprite].active && sprite[camerafollowsprite].isnotspectator())
        {
            // FIXME(skoskav): Scope zoom and non-default resolution makes this a bit complicated.
            // Why does the magic number ~6.8 work so well?

            m.x = exp(CVar::r_zoom) *
                  ((float)((mx - gamewidthhalf)) / sprite[camerafollowsprite].aimdistcoef *
                   (((float)(2 * 640) / gamewidth - 1) +
                    (float)((gamewidth - 640)) / (float)gamewidth *
                        (float)(defaultaimdist - sprite[camerafollowsprite].aimdistcoef) / 6.8));

            m.y = exp(CVar::r_zoom) *
                  ((float)((my - gameheighthalf)) / sprite[camerafollowsprite].aimdistcoef);
            camv.x = camerax;
            camv.y = cameray;
            p.x = spriteparts.pos[camerafollowsprite].x;
            p.y = spriteparts.pos[camerafollowsprite].y;
            norm = vec2subtract(p, camv);
            vec2scale(s, norm, camspeed);
            camv = vec2add(camv, s);
            camv = vec2add(camv, m);
            camerax = camv.x;
            cameray = camv.y;
        }
        else
            camerafollowsprite = 0;
    }
    else if (camerafollowsprite == 0)
    {
        displayratio = (float)(gamewidth) / 640;

        if ((mx > 310 * displayratio) && (mx < 330 * displayratio) && (my > 230) && (my < 250))
        {
            m.x = 0;
            m.y = 0;
        }
        else
        {
            m.x = (float)((mx - gamewidthhalf)) / spectatoraimdist;
            m.y = (float)((my - gameheighthalf)) / spectatoraimdist;
        }
        camv.x = camerax;
        camv.y = cameray;
        camv = vec2add(camv, m);
        camerax = camv.x;
        cameray = camv.y;
    }

    // safety
    if ((mysprite > 0) && (sprite[mysprite].isspectator()))
        if ((camerax > max_sectorz * map.sectorsdivision) ||
            (camerax < min_sectorz * map.sectorsdivision) ||
            (cameray > max_sectorz * map.sectorsdivision) ||
            (cameray < min_sectorz * map.sectorsdivision))
        {
            camerax = 0;
            cameray = 0;
            targetmode = false;
        }

    // end game screen
    if (screentaken)
        if (mapchangecounter < ((float)(default_mapchange_time) / 3))
        {
            screentaken = false;
#ifdef STEAM
            if (CVar::cl_steam_screenshots)
            {
                steamapi.screenshots.triggerscreenshot(0);
                return;
            }
#endif
            NotImplemented(NITag::OTHER, "No now() function");
#if 0
            screenfile = userdirectory + "screens/" +
                         formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + "_endgame.png";
#endif
            takescreenshot(screenfile);
        }

    if ((CVar::demo_autorecord) && (demorecorder.active() == false) && (map.name != ""))
    {
        NotImplemented(NITag::OTHER, "No now() function");
#if 0
        demorecorder.startrecord(userdirectory + "demos/" +
                                 formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + ".sdm");
#endif
    }
}
