// automatically converted

#include "ClientGame.hpp"

#include <common/Console.hpp>
#include <Tracy.hpp>
#include <SDL3/SDL_timer.h>
#include <thread>
#include <array>
#include <chrono>
#include <cmath>
#include <memory>

#include "Client.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "Gfx.hpp"
#include "InterfaceGraphics.hpp"
#include "UpdateFrame.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "shared/network/NetworkClientSprite.hpp"
#include "shared/network/NetworkClient.hpp"
#include "common/Util.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"

using string = std::string;

// used for blinking chat input
// DK_FIXME set it to true for now. I want to see something on screen
// false during game request phase

// us std::uint8_t  action snap

// resolution

// cha std::uint8_t f

GlobalStateClientGame gGlobalStateClientGame{
  .mouseprev{},
  .mx{},
  .my{},
  .mapchanged = false,
  .chatchanged = true,
  .shouldrenderframes = true,
  .actionsnap = 1,
  .actionsnaptaken = false,
  .capscreen = 255,
  .showscreen = 0u,
  .screencounter = 255,
  .isfullscreen{},
  .screenwidth = default_width,
  .screenheight = default_height,
  .renderwidth = 0,
  .renderheight = 0,
  .windowwidth = 0,
  .windowheight = 0,
  .chattext{},
  .lastchattext{},
  .firechattext{},
  .chattype{},
  .completionbase{},
  .completionbaseseparator{},
  .currenttabcompleteplayer = 0,
  .cursorposition = 0,
  .tabcompletepressed{},
  .chattimecounter{},
  .clientstopmovingcounter = 99999,
  .forceclientspritesnapshotmov{},
  .lastforceclientspritesnapshotmovtick{},
  .menutimer{},
};

struct tframetiming
{
  std::int64_t frequency;
  std::int64_t starttime;
  double prevtime;
  double prevrendertime;
  double accumulator;
  double mindeltatime;
  double elapsed;
  std::int32_t counter;
  std::int32_t fps;
  double fpsaccum;
};

tframetiming frametiming;

void resetframetiming()
{
  frametiming.frequency = SDL_GetPerformanceFrequency();
  frametiming.starttime = SDL_GetPerformanceCounter();

  frametiming.prevtime = getcurrenttime();
  frametiming.prevrendertime = frametiming.prevtime;
  frametiming.accumulator = 0;
  frametiming.mindeltatime = 0;
  frametiming.elapsed = 0;

  frametiming.counter = 0;
  frametiming.fps = 0;
  frametiming.fpsaccum = 0;

  if (CVar::r_fpslimit)
  {
    frametiming.mindeltatime = 1.0 / CVar::r_maxfps;
  }

  GS::GetGame().SetTickTime(0);
  GS::GetGame().SetTickTimeLast(0);
}

auto getcurrenttime() -> float
{
  auto x = SDL_GetPerformanceCounter();
  return (float)((x - frametiming.starttime)) / frametiming.frequency;
}

void bigmessage(const std::string &text, std::int32_t delay, std::uint32_t col)
{
  float w;
  float s;

  gfxtextpixelratio(vector2(1, 1));
  setfontstyle(font_big);

  w = rectwidth(gfxtextmetrics(text));
  s = 4.8f * ((float)(gGlobalStateClientGame.renderheight) / 480.f);

  gGlobalStateInterfaceGraphics.bigx[1] = 0;
  gGlobalStateInterfaceGraphics.bigtext[1] = text;
  gGlobalStateInterfaceGraphics.bigdelay[1] = delay;
  gGlobalStateInterfaceGraphics.bigscale[1] =
    std::fmin(1.f / 4.8f, (0.7f * gGlobalStateClientGame.renderwidth / w) / s);
  gGlobalStateInterfaceGraphics.bigcolor[1] = col;
  gGlobalStateInterfaceGraphics.bigposx[1] =
    (float)((gGlobalStateClientGame.renderwidth -
             s * w * gGlobalStateInterfaceGraphics.bigscale[1])) /
    2.f;
  gGlobalStateInterfaceGraphics.bigposy[1] = 420.f * gGlobalStateInterfaceGraphics._iscala.y;

  if (CVar::r_scaleinterface)
  {
    gGlobalStateInterfaceGraphics.bigposx[1] =
      gGlobalStateInterfaceGraphics.bigposx[1] *
      ((float)(gGlobalStateGame.gamewidth) / gGlobalStateClientGame.renderwidth);
  }
}

// In-game nickname tab completion
void tabcomplete()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::int32_t chattextlen;
  std::int32_t offset;
  std::int32_t continuedtabcompleteplayer;
  std::int32_t next;
  std::int32_t availablechatspace;
  std::string spacefittedname;

  if (!sprite_system.IsPlayerSpriteValid())
  {
    return;
  }

  chattextlen = length(gGlobalStateClientGame.chattext);

  if ((chattextlen > 1) && (gGlobalStateClientGame.chattext[2] == '^'))
  {
    offset = 1;
  }
  else
  {
    offset = 0;
  }

  // If not already tab-completing, save and use this base text for tab completetion
  if (gGlobalStateClientGame.currenttabcompleteplayer == 0)
  {
    NotImplemented("string operation");
#if 0
        std::int32_t completionbaselen, lastseparator;
        // Find where the current std::uint64_t starts
        lastseparator = lastdelimiter(' ', string(chattext));

        if (lastseparator < offset)
            lastseparator = offset;

        completionbaselen = chattextlen - lastseparator;
        completionbase = ansimidstr(string(chattext), lastseparator + 1, completionbaselen);
        completionbaseseparator = lastseparator;
#endif
  }

  // Next potential match
  continuedtabcompleteplayer = (gGlobalStateClientGame.currenttabcompleteplayer + 1) % max_players;

  if (chattextlen > offset) // Dont complete if chat is empty
  {
    for (i = continuedtabcompleteplayer; i <= (continuedtabcompleteplayer + max_players - 1); i++)
    {
      next = ((i - 1) % max_players) + 1;
      auto &sprite = sprite_system.GetSprite(next);
      if (sprite.IsActive() && (!sprite.player->demoplayer) &&
          (!sprite_system.IsPlayerSprite(next)))
      {
        if ((gGlobalStateClientGame.completionbase.empty()) ||
            std::string::npos != sprite.player->name.find(gGlobalStateClientGame.completionbase))
        {
          availablechatspace = maxchattext - gGlobalStateClientGame.completionbaseseparator;
          spacefittedname = sprite.player->name.substr(0, availablechatspace);
          gGlobalStateClientGame.chattext = gGlobalStateClientGame.chattext.substr(
                                              0, gGlobalStateClientGame.completionbaseseparator) +
                                            spacefittedname;
          gGlobalStateClientGame.currenttabcompleteplayer = next;
          gGlobalStateClientGame.cursorposition = length((gGlobalStateClientGame.chattext));
          gGlobalStateClientGame.tabcompletepressed = true;
          break;
        }
      }
    }
  }
}

// Resets the stats of all weapons
void resetweaponstats()
{
  for (auto &w : gGlobalStateClient.wepstats)
  {
    w.shots = 0;
    w.hits = 0;
    w.kills = 0;
    w.headshots = 0;
    w.accuracy = 0;
  }
}

auto getgamefps() -> std::int32_t { return frametiming.fps; }

void gameloop()
{
  ZoneScopedN("gameloop");
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t maincontrol;
  std::int32_t heavysendersnum;
  float adjust;
  double currenttime;
  double frametime;
  double simtime;
  double framepercent;
  double dt;
  bool gamepaused;

  gamepaused = (GS::GetGame().GetMapchangecounter() >= 0);

  currenttime = getcurrenttime();

  frametime = currenttime - frametiming.prevtime;

  frametiming.fpsaccum = frametiming.fpsaccum + frametime;

  auto &game = GS::GetGame();

  frametiming.prevtime = currenttime;
  game.SetTickTimeLast(game.GetTickTime());

  if (frametime > 2)
  {
    frametime = 0;
  }

  dt = (float)(1) / game.GetGoalTicks();

  frametiming.accumulator = frametiming.accumulator + frametime;
  game.SetTickTime(game.GetTickTime() + trunc((float)(frametiming.accumulator) / dt));

  simtime = (game.GetTickTime() - game.GetTickTimeLast()) * dt;
  frametiming.accumulator = frametiming.accumulator - simtime;
  framepercent = std::fmin(1, std::fmax(0, (float)(frametiming.accumulator) / dt));

  for (maincontrol = 1; maincontrol <= (game.GetTickTime() - game.GetTickTimeLast()); maincontrol++)
  { // frame rate independant code
    if (!gamepaused)
    {
      frametiming.elapsed = frametiming.elapsed + ((float)(1) / default_goalticks);
    }

    gGlobalStateNetworkClient.clienttickcount += 1;
    // Update main tick counter
    GS::GetGame().TickMainTickCounter();

    if (gGlobalStateClientGame.menutimer > -1)
    {
      gGlobalStateClientGame.menutimer -= 1;
    }

    // General game updating
    update_frame();

    if (GS::GetDemoRecorder().active() &&
        (GS::GetGame().GetMainTickCounter() % CVar::demo_rate == 0))
    {
      GS::GetDemoRecorder().saveposition();
    }

    if ((game.GetMapchangecounter() < 0) && (gGlobalStateGameMenus.escmenu->active))
    {
      // DEMO
      if (GS::GetDemoRecorder().active())
      {
        GS::GetDemoRecorder().savenextframe();
      }
      if (gGlobalStateDemo.demoplayer.active())
      {
        tdemoplayer::processdemo();
      }
    }

    // Radio Cooldown
    if ((GS::GetGame().GetMainTickCounter() % second == 0) &&
        (gGlobalStateClient.radiocooldown > 0) && (CVar::sv_radio))
    {
      gGlobalStateClient.radiocooldown -= 1;
    }

    // Packet rate send adjusting
    if (gGlobalStateClient.packetadjusting == 1)
    {
      heavysendersnum = GS::GetGame().GetPlayersNum() - GS::GetGame().GetSpectatorsNum();

      if (heavysendersnum < 5)
      {
        adjust = 0.75;
      }
      else if (heavysendersnum < 9)
      {
        adjust = 0.87;
      }
      else
      {
        adjust = 1.0;
      }
    }
    else
    {
      adjust = 1.0;
    }

    if ((sprite_system.IsPlayerSpriteValid()) && (!gGlobalStateDemo.demoplayer.active()))
    {
      // connection problems
      if ((game.GetMapchangecounter() < 0) && gGlobalStateGameMenus.escmenu->active)
      {
        gGlobalStateNetworkClient.noheartbeattime += 1;
      }

      if (gGlobalStateNetworkClient.noheartbeattime > connectionproblem_time)
      {
        if (GS::GetGame().GetMainTickCounter() % 120 == 0)
        {
          if (gGlobalStateNetworkClient.noheartbeattime > disconnection_time)
          {
            GS::GetMainConsole().console(("Connection timeout"), warning_message_color);
          }
          else
          {
            GS::GetMainConsole().console(("Connection problem"), warning_message_color);
          }
        }

        gGlobalStateClientGame.clientstopmovingcounter = 0;
      }

      if (gGlobalStateNetworkClient.noheartbeattime == disconnection_time)
      {
        GS::GetGame().showmapchangescoreboard();

        gamemenushow(gGlobalStateGameMenus.teammenu, false);

        GS::GetMainConsole().console(("Connection timeout"), warning_message_color);

        clientdisconnect(*GetNetwork());
      }

      if (gGlobalStateNetworkClient.noheartbeattime < 0)
      {
        gGlobalStateNetworkClient.noheartbeattime = 0;
      }

      gGlobalStateClientGame.clientstopmovingcounter -= 1;

      auto &sprite = sprite_system.GetPlayerSprite();

      if (gGlobalStateClient.connection == INTERNET)
      {
        if (sprite.IsActive())
        {
          if (!sprite.deadmeat)
          {
            if ((GS::GetGame().GetMainTickCounter() % (std::int32_t)round(7 * adjust) == 1) &&
                (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(5 * adjust) != 0))
            {
              clientspritesnapshot();
            }
            if ((GS::GetGame().GetMainTickCounter() % (std::int32_t)round(5 * adjust) == 0) ||
                gGlobalStateClientGame.forceclientspritesnapshotmov)
            {
              clientspritesnapshotmov();
            }
          }
          else if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(30 * adjust) == 0)
          {
            clientspritesnapshotdead();
          }
        }
      }
      else if (gGlobalStateClient.connection == LAN)
      {
        if (!sprite.deadmeat)
        {
          if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(4 * adjust) == 0)
          {
            clientspritesnapshot();
          }

          if ((GS::GetGame().GetMainTickCounter() % (std::int32_t)round(3 * adjust) == 0) ||
              gGlobalStateClientGame.forceclientspritesnapshotmov)
          {
            clientspritesnapshotmov();
          }
        }
        else if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(15 * adjust) == 0)
        {
          clientspritesnapshotdead();
        }
      }

      gGlobalStateClientGame.forceclientspritesnapshotmov = false;
    } // playing

    // UDP.FlushMsg;
  } // Client

  // this shouldn't happen but still done for safety
  if (frametiming.prevrendertime > currenttime)
  {
    frametiming.prevrendertime = currenttime - frametiming.mindeltatime;
  }

  if (gGlobalStateClientGame.shouldrenderframes &&
      ((currenttime - frametiming.prevrendertime) >= frametiming.mindeltatime))
  {
    frametiming.prevrendertime = currenttime;
    frametiming.counter += 1;

    if (frametiming.counter >= 30)
    {
      frametiming.fps = round((float)(frametiming.counter) / frametiming.fpsaccum);
      frametiming.counter = 0;
      frametiming.fpsaccum = 0;
    }

    if (gamepaused)
    {
      renderframe(frametiming.elapsed, framepercent, true);
    }
    else
    {
      renderframe(frametiming.elapsed - dt * (1 - framepercent), framepercent, false);
    }
  }

  if ((game.GetMapchangecounter() < 0) && (game.GetMapchangecounter() > -59))
  {
    if (game.GetMapchangename() == "EXIT*!*")
    {
      gGlobalStateClient.gClient.exittomenu();
    }
  }

  if (gGlobalStateClientGame.mapchanged)
  {
    gGlobalStateClientGame.mapchanged = false;
    resetframetiming();
  }

  if (CVar::r_sleeptime > 0)
  {
    ZoneScopedN("Sleeping");
    std::this_thread::sleep_for(std::chrono::milliseconds((std::int32_t)CVar::r_sleeptime));
  }
}

auto getcameratarget(bool backwards) -> std::uint8_t
{
  auto &sprite_system = SpriteSystem::Get();
  std::uint8_t newcam;
  std::uint8_t numloops;
  bool validcam;

  validcam = false;
  newcam = gGlobalStateClient.camerafollowsprite;
  numloops = 0;

  do
  {
    numloops += 1;
    if (numloops == 33)
    { // Shit, way too many loops...
      newcam = 0;
      validcam = true;
      break;
    }

    if (!backwards)
    {
      newcam += 1;
    }
    else
    {
      newcam -= 1;
    }
    if (newcam > max_sprites)
    {
      newcam = 1;
    }
    else if (newcam < 1)
    {
      newcam = max_sprites;
    }

    if (!sprite_system.GetSprite(newcam).active)
    {
      continue; // Sprite slot empty
    }
    if (sprite_system.GetSprite(newcam).deadmeat)
    {
      continue; // Sprite is dead
    }
    if (sprite_system.GetSprite(newcam).isspectator())
    {
      continue; // Sprite is a spectator
    }

    if (sprite_system.GetPlayerSprite().control.up && (!CVar::sv_realisticmode) &&
        sprite_system.GetPlayerSprite().isnotspectator())
    {
      newcam = 0;
      validcam = true;
      break;
    } // Freecam if not Realistic

    if (sprite_system.GetPlayerSprite().isspectator())
    {
      if (sprite_system.GetPlayerSprite().control.up)
      {
        newcam = 0;
        validcam = true;
        break;
      }

      // Allow spectators to go into Free Cam
      validcam = true;
      break;
      // Let spectator view all players
    }

    if (sprite_system.GetSprite(newcam).isnotinsameteam(sprite_system.GetPlayerSprite()))
    {
      continue; // Dont swap camera to a player not on my team
    }

    validcam = true;
  } while (!validcam);

  return iif(validcam, newcam, gGlobalStateClient.camerafollowsprite);
}

#ifdef STEAM
void getmicdata()
{
  evoiceresult availablevoice;
  std::uint32_t availablevoicebytes;
  array of std::uint8_t voicedata;

  availablevoice = steamapi.user.getavailablevoice(&availablevoicebytes, nullptr, 0);

  if ((availablevoice == k_evoiceresultok) && (availablevoicebytes > 0))
  {
    setlength(voicedata, availablevoicebytes);
    availablevoice = steamapi.user.getvoice(true, voicedata, availablevoicebytes,
                                            &availablevoicebytes, false, nullptr, 0, nullptr, 0);

    if ((availablevoice == k_evoiceresultok) && (availablevoicebytes > 0))
      clientsendvoicedata(voicedata, availablevoicebytes);
  }
}
#endif
