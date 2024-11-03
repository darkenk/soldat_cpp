// automatically converted

#include "UpdateFrame.hpp"

#include "Client.hpp"
#include "ClientGame.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "GameStrings.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "WeatherEffects.hpp"
#include "common/Calc.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/LogFile.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include <Tracy.hpp>
#include <cstdint>
#include <shared/network/NetworkClient.hpp>

std::int32_t idlecounter, oldmousex;

void update_frame()
{
  ZoneScopedN("update_frame");
  std::int32_t j;
  tvector2 norm, camv, s, m, p;
  float displayratio;
  std::string screenfile;

#ifdef ENABLE_FAE
  faeontick;
#endif

  auto &game = GS::GetGame();

  cameraprev.x = camerax;
  cameraprev.y = cameray;
  mouseprev.x = mx;
  mouseprev.y = my;

  auto &map = game.GetMap();

  if (game.GetMapchangecounter() < 0)
  {
    ZoneScopedN("Update1");
    if (demoplayer.active() && escmenu->active)
      return;

    {
      ZoneScopedN("SpriteParts");
      if (clientstopmovingcounter > 0)
      {
        SpriteSystem::Get().UpdateSpriteParts();
      }
    }

    {
      ZoneScopedN("UpdateSprites");
      auto &activeSprites = SpriteSystem::Get().GetActiveSprites();
      std::for_each(std::begin(activeSprites), std::end(activeSprites),
                    [](auto &sprite) { sprite.update(); });
    }

    // Bullets update
    {
      ZoneScopedN("Bullets");
      auto &bullet = GS::GetBulletSystem().GetBullets();
      for (j = 1; j <= max_bullets; j++)
      {
        if (bullet[j].active)
          bullet[j].update();

        if (bullet[j].pingadd > 0)
          bullet[j].pingadd -= 4;
      }
    }

    {
      ZoneScopedN("BulletParts");
      GetBulletParts().doeulertimestep();
    }

    {
      ZoneScopedN("Sparks");
      sparkscount = 0;
      for (j = 1; j <= max_sparks; j++)
        if (spark[j].active)
        {
          spark[j].update();
          sparkscount += 1;
        }
    }

    {
      ZoneScopedN("Things");
      auto &things = GS::GetThingSystem().GetThings();
      // update Things
      for (j = 1; j <= max_things; j++)
        if (things[j].active)
          things[j].update();
    }

    if (GS::GetGame().GetMainTickCounter() % second == 0)
      if (screencounter != 255)
        // TODO: don't rely on underflow
        screencounter = 0xff & (screencounter - 1);

    // Change spectate target away from dead player
    if (GS::GetGame().GetMainTickCounter() % (second * 5) == 0)
      if ((camerafollowsprite > 0) && SpriteSystem::Get().GetSprite(camerafollowsprite).deadmeat and
          (CVar::sv_realisticmode) && (CVar::sv_survivalmode) && !game.GetSurvivalEndRound())
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
    //   if SpriteSystem::Get().GetSprite(j).Active then
    //    ControlSprite(SpriteSystem::Get().GetSprite(j));
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
  {
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      j = sprite.num;
      if (sprite.isnotspectator() && (j != mysprite) && (sprite.bonusstyle != bonus_predator) and
          ((sprite.position == pos_stand) or
           (sprite.isnotsolo() && sprite.isinsameteam(SpriteSystem::Get().GetSprite(mysprite))) or
           SpriteSystem::Get().GetSprite(mysprite).deadmeat or sprite.deadmeat) and
          ((sprite.visible > 40) or (!CVar::sv_realisticmode)))
      {
        const auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(j);
        if (distance(-gamewidthhalf + camerax + mx, -gameheighthalf + cameray + my,
                     spritePartsPos.x, spritePartsPos.y) < cursorsprite_distance)
        {
          cursortext = sprite.player->name;
          if (game.isteamgame())
            if (sprite.isinsameteam(SpriteSystem::Get().GetSprite(mysprite)))
            {
              cursortext =
                cursortext + ' ' +
                inttostr(round(((float)(sprite.GetHealth()) / game.GetStarthealth()) * 100)) + '%';
              cursorfriendly = true;
            }

          break;
        }
      }
    }
  }
  cursortextlength = length(cursortext);

  // bullet timer
  game.TickBulletTimeTimer();

  if (game.GetBulletTimeTimer() == 0)
  {
    game.togglebullettime(false);
    game.TickBulletTimeTimer();
  }
  else if (game.GetBulletTimeTimer() < 1)
  {
    // MapChange counter update
    if ((game.GetMapchangecounter() > -60) && (game.GetMapchangecounter() < 99999999))
    {
      game.SetMapchangecounter(game.GetMapchangecounter() - 1);
    }
    if ((game.GetMapchangecounter() < 0) && (game.GetMapchangecounter() > -59))
    {
      if (game.GetMapchangename() != "EXIT*!*")
      {
        game.changemap();
        resetweaponstats();
      }
    }

    // Game Stats save
    if ((GS::GetGame().GetMainTickCounter() % CVar::log_filesupdate) == 0)
    {
      if (CVar::log_enable)
      {
        auto& fs = GS::GetFileSystem();
        if (fs.Size(GetGameLogFilename()) > max_logfilesize)
        {
          newlogfiles(fs);
        }

        writelogfile(fs, GetGameLog(), GetGameLogFilename());
      }
    }

    if (GS::GetGame().GetMainTickCounter() % (second * 6) == 0)
    {
      if (GS::GetGame().GetPlayersNum() == 0)
        if (game.GetMapchangecounter() > 99999999)
        {
          game.SetMapchangecounter(game.GetMapchangecounter() - 60);
        }
    }

    {
      auto v = GS::GetGame().GetSinusCounter() + iluminatespeed;
      GS::GetGame().SetSinusCounter(v);
    }

    if (grenadeeffecttimer > -1)
      grenadeeffecttimer = grenadeeffecttimer - 1;

    // Spray counter
    if (hitspraycounter > 0)
      hitspraycounter -= 1;

    // Idle counter
    if (mysprite > 0)
      if (game.GetMapchangecounter() < 99999999)
        if (SpriteSystem::Get().GetSprite(mysprite).isnotspectator() &&
            (!SpriteSystem::Get().GetSprite(mysprite).player->demoplayer))
        {
          if (oldmousex - round(mx) == 0)
            idlecounter += 1;
          else
            idlecounter = 0;

          if (idlecounter > Constants::IDLE_KICK)
          {
            clientdisconnect(*GetNetwork());
            exittomenu();
          }

          oldmousex = round(mx);
        }

    // Time Limit decrease
    if (game.GetMapchangecounter() < 99999999)
    {
      if (game.GetTimelimitcounter() > 0)
      {
        game.SetTimelimitcounter(game.GetTimelimitcounter() - 1);
      }
    }

    game.SetTimeleftmin(game.GetTimelimitcounter() / 3600);
    game.SetTimeleftsec((game.GetTimelimitcounter() - game.GetTimeleftmin() * 3600) / 60);

    if (game.GetTimelimitcounter() > 0)
    {
      if (game.GetTimelimitcounter() < 601)
      {
        if (game.GetTimelimitcounter() % 60 == 0)
        {
          if (game.GetMapchangecounter() == -60)
          {
            GS::GetMainConsole().console(_("Time Left:") + ' ' +
                                           (inttostr(game.GetTimelimitcounter() / 60)) + ' ' +
                                           _("seconds"),
                                         game_message_color);
            playsound(SfxEffect::signal);
          }
        }
      }
      else if (game.GetTimelimitcounter() < 3601)
      {
        if (game.GetTimelimitcounter() % 600 == 0)
        {
          GS::GetMainConsole().console(_("Time Left:") + ' ' +
                                         (inttostr(game.GetTimelimitcounter() / 60)) + ' ' +
                                         _("seconds"),
                                       game_message_color);
          playsound(SfxEffect::signal);
        }
      }
      else if (game.GetTimelimitcounter() < 18001)
      {
        if (game.GetTimelimitcounter() % 3600 == 0)
        {
          GS::GetMainConsole().console(_("Time Left:") + ' ' +
                                         (inttostr(game.GetTimelimitcounter() / 3600)) + ' ' +
                                         _("minutes"),
                                       game_message_color);
          playsound(SfxEffect::signal);
        }
      }
      else if (game.GetTimelimitcounter() % 18000 == 0)
      {
        GS::GetMainConsole().console(_("Time Left:") + ' ' +
                                       (inttostr(game.GetTimelimitcounter() / 3600)) + ' ' +
                                       _("minutes"),
                                     game_message_color);
        playsound(SfxEffect::signal);
      }
    }

    // Map voting timer
    GS::GetGame().timervote();

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
    GS::GetMainConsole().Update();
    GetKillConsole().Update(true);

    if (chattimecounter > 0)
      chattimecounter = chattimecounter - 1;
  } // bullettime off

  // MOVE -=CAMERA=-
  if ((camerafollowsprite > 0) && (camerafollowsprite < max_sprites + 1))
  {
    if (SpriteSystem::Get().GetSprite(camerafollowsprite).IsActive() &&
        SpriteSystem::Get().GetSprite(camerafollowsprite).isnotspectator())
    {
      // FIXME(skoskav): Scope zoom and non-default resolution makes this a bit complicated.
      // Why does the magic number ~6.8 work so well?

      m.x = exp(CVar::r_zoom) *
            ((float)((mx - gamewidthhalf)) /
             SpriteSystem::Get().GetSprite(camerafollowsprite).aimdistcoef *
             (((float)(2 * 640) / gamewidth - 1) +
              (float)((gamewidth - 640)) / (float)gamewidth *
                (float)(defaultaimdist -
                        SpriteSystem::Get().GetSprite(camerafollowsprite).aimdistcoef) /
                6.8));

      m.y = exp(CVar::r_zoom) * ((float)((my - gameheighthalf)) /
                                 SpriteSystem::Get().GetSprite(camerafollowsprite).aimdistcoef);
      camv.x = camerax;
      camv.y = cameray;
      auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(camerafollowsprite);
      p.x = spritePartsPos.x;
      p.y = spritePartsPos.y;
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
  if ((mysprite > 0) && (SpriteSystem::Get().GetSprite(mysprite).isspectator()))
    if ((camerax > max_sectorz * map.GetSectorsDivision()) ||
        (camerax < min_sectorz * map.GetSectorsDivision()) ||
        (cameray > max_sectorz * map.GetSectorsDivision()) ||
        (cameray < min_sectorz * map.GetSectorsDivision()))
    {
      camerax = 0;
      cameray = 0;
      targetmode = false;
    }

  // end game screen
  if (screentaken)
    if (game.GetMapchangecounter() < ((float)(default_mapchange_time) / 3))
    {
      screentaken = false;
#ifdef STEAM
      if (CVar::cl_steam_screenshots)
      {
        steamapi.screenshots.triggerscreenshot(0);
        return;
      }
#endif
      NotImplemented("No now() function");
#if 0
            screenfile = GS::GetGame().GetUserDirectory() + "screens/" +
                         formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + "_endgame.png";
#endif
      takescreenshot(screenfile);
    }

  if ((CVar::demo_autorecord) && (GS::GetDemoRecorder().active() == false) && (map.name != ""))
  {
    NotImplemented("No now() function");
#if 0
        GS::GetDemoRecorder().startrecord(GS::GetGame().GetUserDirectory() + "demos/" +
                                 formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + ".sdm");
#endif
  }
}
