// automatically converted

#include "UpdateFrame.hpp"

#include <Tracy.hpp>
#include <math.h>
#include <cstdint>
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <string>

#include "Client.hpp"
#include "ClientGame.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "WeatherEffects.hpp"
#include "common/Calc.hpp"
#include "common/Console.hpp"
#include "common/GameStrings.hpp"
#include "common/LogFile.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClient.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "common/Constants.hpp"
#include "common/MapFile.hpp"
#include "common/Parts.hpp"
#include "common/PolyMap.hpp"
#include "common/Vector.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SafeType.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sparks.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/mechanics/Things.hpp"
#include "shared/network/Net.hpp"

std::int32_t idlecounter, oldmousex;

void update_frame()
{
  ZoneScopedN("update_frame");
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t j;
  tvector2 norm;
  tvector2 camv;
  tvector2 s;
  tvector2 m;
  tvector2 p;
  float displayratio;
  std::string screenfile;

#ifdef ENABLE_FAE
  faeontick;
#endif

  auto &game = GS::GetGame();

  gGlobalStateClient.cameraprev.x = gGlobalStateClient.camerax;
  gGlobalStateClient.cameraprev.y = gGlobalStateClient.cameray;
  gGlobalStateClientGame.mouseprev.x = gGlobalStateClientGame.mx;
  gGlobalStateClientGame.mouseprev.y = gGlobalStateClientGame.my;

  auto &map = game.GetMap();

  if (game.GetMapchangecounter() < 0)
  {
    ZoneScopedN("Update1");
    if (gGlobalStateDemo.demoplayer.active() && gGlobalStateGameMenus.escmenu->active)
    {
      return;
    }

    {
      ZoneScopedN("SpriteParts");
      if (gGlobalStateClientGame.clientstopmovingcounter > 0)
      {
        sprite_system.UpdateSpriteParts();
      }
    }

    {
      ZoneScopedN("UpdateSprites");
      auto &activeSprites = sprite_system.GetActiveSprites();
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
        {
          bullet[j].update();
        }

        if (bullet[j].pingadd > 0)
        {
          bullet[j].pingadd -= 4;
        }
      }
    }

    {
      ZoneScopedN("BulletParts");
      GetBulletParts().doeulertimestep();
    }

    {
      ZoneScopedN("Sparks");
      gGlobalStateSparks.sparkscount = 0;
      for (j = 1; j <= max_sparks; j++)
      {
        if (gGlobalStateGame.spark[j].active)
        {
          gGlobalStateGame.spark[j].update();
          gGlobalStateSparks.sparkscount += 1;
        }
      }
    }

    {
      ZoneScopedN("Things");
      auto &things = GS::GetThingSystem().GetThings();
      // update Things
      for (j = 1; j <= max_things; j++)
      {
        if (things[j].active)
        {
          things[j].update();
        }
      }
    }

    if (GS::GetGame().GetMainTickCounter() % second == 0)
    {
      if (gGlobalStateClientGame.screencounter != 255)
      {
        // TODO: don't rely on underflow
        gGlobalStateClientGame.screencounter = 0xff & (gGlobalStateClientGame.screencounter - 1);
      }
    }

    // Change spectate target away from dead player
    if (GS::GetGame().GetMainTickCounter() % (second * 5) == 0)
    {
      if ((gGlobalStateClient.camerafollowsprite > 0) &&
          sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).deadmeat and
          (CVar::sv_realisticmode) && (CVar::sv_survivalmode) && !game.GetSurvivalEndRound())
      {
        gGlobalStateClient.camerafollowsprite = gGlobalStateClientGame.getcameratarget(false);
      }
    }

    // Weather effects
    if (CVar::r_weathereffects)
    {
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

  // >> cursor on player <<
  gGlobalStateInterfaceGraphics.cursortext = "";
  gGlobalStateInterfaceGraphics.cursorfriendly = false;

  // TODO(helloer): While watching demos this code needs to use SpectNumber instead of MySprite
  if ((sprite_system.IsPlayerSpriteValid()) && (!gGlobalStateDemo.demoplayer.active()))
  {
    for (auto &sprite : sprite_system.GetActiveSprites())
    {
      j = sprite.num;
      if (sprite.isnotspectator() && (!sprite_system.IsPlayerSprite(j)) &&
          (sprite.bonusstyle != bonus_predator) and
          ((sprite.position == pos_stand) or
           (sprite.isnotsolo() && sprite.isinsameteam(sprite_system.GetPlayerSprite())) or
           sprite_system.GetPlayerSprite().deadmeat or sprite.deadmeat) and
          ((sprite.visible > 40) or (!CVar::sv_realisticmode)))
      {
        const auto &spritePartsPos = sprite_system.GetSpritePartsPos(j);
        if (distance(-gGlobalStateGame.gamewidthhalf + gGlobalStateClient.camerax +
                       gGlobalStateClientGame.mx,
                     -gGlobalStateGame.gameheighthalf + gGlobalStateClient.cameray +
                       gGlobalStateClientGame.my,
                     spritePartsPos.x, spritePartsPos.y) < cursorsprite_distance)
        {
          gGlobalStateInterfaceGraphics.cursortext = sprite.player->name;
          if (game.isteamgame())
          {
            if (sprite.isinsameteam(sprite_system.GetPlayerSprite()))
            {
              gGlobalStateInterfaceGraphics.cursortext =
                gGlobalStateInterfaceGraphics.cursortext + ' ' +
                inttostr(round((sprite.GetHealth() / game.GetStarthealth()) * 100)) + '%';
              gGlobalStateInterfaceGraphics.cursorfriendly = true;
            }
          }

          break;
        }
      }
    }
  }
  gGlobalStateInterfaceGraphics.cursortextlength = length(gGlobalStateInterfaceGraphics.cursortext);

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
        gGlobalStateClientGame.resetweaponstats();
      }
    }

    // Game Stats save
    if ((GS::GetGame().GetMainTickCounter() % CVar::log_filesupdate) == 0)
    {
      GS::GetConsoleLogFile().Enable(CVar::log_enable);
      GS::GetConsoleLogFile().SetLogLevel(CVar::log_level);
      if (CVar::log_enable)
      {
        GS::GetConsoleLogFile().CreateNewLogIfCurrentLogIsTooBig();
        GS::GetConsoleLogFile().WriteToFile();
      }
    }

    if (GS::GetGame().GetMainTickCounter() % (second * 6) == 0)
    {
      if (GS::GetGame().GetPlayersNum() == 0)
      {
        if (game.GetMapchangecounter() > 99999999)
        {
          game.SetMapchangecounter(game.GetMapchangecounter() - 60);
        }
      }
    }

    {
      auto v = GS::GetGame().GetSinusCounter() + iluminatespeed;
      GS::GetGame().SetSinusCounter(v);
    }

    if (gGlobalStateClient.grenadeeffecttimer > -1)
    {
      gGlobalStateClient.grenadeeffecttimer = gGlobalStateClient.grenadeeffecttimer - 1;
    }

    // Spray counter
    if (gGlobalStateClient.hitspraycounter > 0)
    {
      gGlobalStateClient.hitspraycounter -= 1;
    }

    // Idle counter
    if (sprite_system.IsPlayerSpriteValid())
    {
      if (game.GetMapchangecounter() < 99999999)
      {
        if (sprite_system.GetPlayerSprite().isnotspectator() &&
            (!sprite_system.GetPlayerSprite().player->demoplayer))
        {
          if (oldmousex - round(gGlobalStateClientGame.mx) == 0)
          {
            idlecounter += 1;
          }
          else
          {
            idlecounter = 0;
          }

          if (idlecounter > Constants::IDLE_KICK)
          {
            clientdisconnect(*gGlobalStateNetworkClient.GetNetwork());
            gGlobalStateClient.exittomenu();
          }

          oldmousex = round(gGlobalStateClientGame.mx);
        }
      }
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
            gGlobalStateSound.playsound(SfxEffect::signal);
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
          gGlobalStateSound.playsound(SfxEffect::signal);
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
          gGlobalStateSound.playsound(SfxEffect::signal);
        }
      }
      else if (game.GetTimelimitcounter() % 18000 == 0)
      {
        GS::GetMainConsole().console(_("Time Left:") + ' ' +
                                       (inttostr(game.GetTimelimitcounter() / 3600)) + ' ' +
                                       _("minutes"),
                                     game_message_color);
        gGlobalStateSound.playsound(SfxEffect::signal);
      }
    }

    // Map voting timer
    GS::GetGame().timervote();

    // Chat Update
    for (j = 1; j <= max_sprites; j++)
    {
      if (gGlobalStateInterfaceGraphics.chatdelay[j] > 0)
      {
        gGlobalStateInterfaceGraphics.chatdelay[j] = gGlobalStateInterfaceGraphics.chatdelay[j] - 1;
      }
    }

    // Big and World Message update
    for (j = 0; j < max_big_messages; j++)
    {
      if (gGlobalStateInterfaceGraphics.bigdelay[j] > 0)
      {
        gGlobalStateInterfaceGraphics.bigdelay[j] = gGlobalStateInterfaceGraphics.bigdelay[j] - 1;
      }
      if (gGlobalStateInterfaceGraphics.worlddelay[j] > 0)
      {
        gGlobalStateInterfaceGraphics.worlddelay[j] =
          gGlobalStateInterfaceGraphics.worlddelay[j] - 1;
      }
    }

    // Shot dist update
    if (gGlobalStateClient.shotdistanceshow > 0)
    {
      gGlobalStateClient.shotdistanceshow = gGlobalStateClient.shotdistanceshow - 1;
    }

    // Consoles Update
    GS::GetMainConsole().Update();
    gGlobalStateClient.GetKillConsole().Update(true);

    if (gGlobalStateClientGame.chattimecounter > 0)
    {
      gGlobalStateClientGame.chattimecounter = gGlobalStateClientGame.chattimecounter - 1;
    }
  } // bullettime off

  // MOVE -=CAMERA=-
  if ((gGlobalStateClient.camerafollowsprite > 0) &&
      (gGlobalStateClient.camerafollowsprite < max_sprites + 1))
  {
    if (sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).IsActive() &&
        sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).isnotspectator())
    {
      // FIXME(skoskav): Scope zoom and non-default resolution makes this a bit complicated.
      // Why does the magic number ~6.8 work so well?

      m.x =
        exp(CVar::r_zoom) *
        ((gGlobalStateClientGame.mx - gGlobalStateGame.gamewidthhalf) /
         sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).aimdistcoef *
         (((float)(2 * 640) / gGlobalStateGame.gamewidth - 1) +
          (float)((gGlobalStateGame.gamewidth - 640)) / (float)gGlobalStateGame.gamewidth *
            (float)(defaultaimdist -
                    sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).aimdistcoef) /
            6.8));

      m.y = exp(CVar::r_zoom) *
            ((gGlobalStateClientGame.my - gGlobalStateGame.gameheighthalf) /
             sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).aimdistcoef);
      camv.x = gGlobalStateClient.camerax;
      camv.y = gGlobalStateClient.cameray;
      auto &spritePartsPos = sprite_system.GetSpritePartsPos(gGlobalStateClient.camerafollowsprite);
      p.x = spritePartsPos.x;
      p.y = spritePartsPos.y;
      norm = vec2subtract(p, camv);
      vec2scale(s, norm, camspeed);
      camv = vec2add(camv, s);
      camv = vec2add(camv, m);
      gGlobalStateClient.camerax = camv.x;
      gGlobalStateClient.cameray = camv.y;
    }
    else
    {
      gGlobalStateClient.camerafollowsprite = 0;
    }
  }
  else if (gGlobalStateClient.camerafollowsprite == 0)
  {
    displayratio = (float)(gGlobalStateGame.gamewidth) / 640;

    if ((gGlobalStateClientGame.mx > 310 * displayratio) &&
        (gGlobalStateClientGame.mx < 330 * displayratio) && (gGlobalStateClientGame.my > 230) &&
        (gGlobalStateClientGame.my < 250))
    {
      m.x = 0;
      m.y = 0;
    }
    else
    {
      m.x = (gGlobalStateClientGame.mx - gGlobalStateGame.gamewidthhalf) / spectatoraimdist;
      m.y = (gGlobalStateClientGame.my - gGlobalStateGame.gameheighthalf) / spectatoraimdist;
    }
    camv.x = gGlobalStateClient.camerax;
    camv.y = gGlobalStateClient.cameray;
    camv = vec2add(camv, m);
    gGlobalStateClient.camerax = camv.x;
    gGlobalStateClient.cameray = camv.y;
  }

  // safety
  if ((sprite_system.IsPlayerSpriteValid()) && (sprite_system.GetPlayerSprite().isspectator()))
  {
    if ((gGlobalStateClient.camerax > max_sectorz * map.GetSectorsDivision()) ||
        (gGlobalStateClient.camerax < min_sectorz * map.GetSectorsDivision()) ||
        (gGlobalStateClient.cameray > max_sectorz * map.GetSectorsDivision()) ||
        (gGlobalStateClient.cameray < min_sectorz * map.GetSectorsDivision()))
    {
      gGlobalStateClient.camerax = 0;
      gGlobalStateClient.cameray = 0;
      gGlobalStateClient.targetmode = false;
    }
  }

  // end game screen
  if (gGlobalStateClient.screentaken)
  {
    if (game.GetMapchangecounter() < ((float)(default_mapchange_time) / 3))
    {
      gGlobalStateClient.screentaken = false;
      NotImplemented("No now() function");
#if 0
            screenfile = GS::GetGame().GetUserDirectory() + "screens/" +
                         formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + "_endgame.png";
#endif
      gGlobalStateGameRendering.takescreenshot(screenfile);
    }
  }

  if ((CVar::demo_autorecord) && (!GS::GetDemoRecorder().active()) && (!map.name.empty()))
  {
    NotImplemented("No now() function");
#if 0
        GS::GetDemoRecorder().startrecord(GS::GetGame().GetUserDirectory() + "demos/" +
                                 formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + ".sdm");
#endif
  }
}
