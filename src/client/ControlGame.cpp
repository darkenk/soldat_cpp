// automatically converted

#include "ControlGame.hpp"

#include <shared/network/NetworkClient.hpp>
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <math.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <numeric>
#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "Client.hpp"
#include "ClientGame.hpp"
#include "GameMenus.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "common/Console.hpp"
#include "common/GameStrings.hpp"
#include "shared/Command.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "shared/network/NetworkClientGame.hpp"
#include "shared/network/NetworkClientMessages.hpp"
#include "common/Util.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"

GlobalStateControlGame gGlobalStateControlGame{
  .votekickreasontype = false,
};

void clearchattext()
{
  gGlobalStateClientGame.lastchattext = gGlobalStateClientGame.chattext;
  gGlobalStateClientGame.firechattext = "";
  gGlobalStateClientGame.completionbase = "";
  gGlobalStateClientGame.currenttabcompleteplayer = 0;
  gGlobalStateClientGame.cursorposition = 1;
  gGlobalStateControlGame.votekickreasontype = false;
  gGlobalStateClientGame.chattext = "";
  SDL_StopTextInput(gGlobalStateInput.gamewindow);
}

auto filterchattext(const std::string &str1) -> std::string
{
  std::string result;
  result = std::accumulate(str1.begin(), str1.end(), result, [](const auto &out, const auto &c) {
    if ((c >= '\40') && (c != '\177'))
    {
      return out + c;
    }
    return out;
  });
  return result;
}

auto chatkeydown(std::uint8_t keymods, SDL_Keycode keycode) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t len;
  std::string str1;
  std::string consolestr;

  bool result;
  result = false;

  if (length(gGlobalStateClientGame.chattext) > 0)
  {
    if ((keymods == km_ctrl) && (keycode == SDLK_V))
    {
      str1 = filterchattext(SDL_GetClipboardText());
      len = length(gGlobalStateClientGame.chattext);
      gGlobalStateClientGame.chattext.insert(gGlobalStateClientGame.cursorposition + 1, str1);
      gGlobalStateClientGame.cursorposition += length(gGlobalStateClientGame.chattext) - len;

      len = iif(gGlobalStateControlGame.votekickreasontype, REASON_CHARS - 1, maxchattext);

      if (length(gGlobalStateClientGame.chattext) > len)
      {
        gGlobalStateClientGame.chattext = gGlobalStateClientGame.chattext.substr(1);
        gGlobalStateClientGame.cursorposition =
          min(gGlobalStateClientGame.cursorposition, (std::uint8_t)len);
      }

      gGlobalStateClientGame.currenttabcompleteplayer = 0;
      gGlobalStateClientGame.chatchanged = true;
      result = true;
    }
    else if (keymods == km_none)
    {
      result = true;

      switch (keycode)
      {
      case SDLK_ESCAPE: {
        gGlobalStateClientGame.chattext = gGlobalStateClientGame.lastchattext;
        clearchattext();
      }
      break;

      case SDLK_BACKSPACE: {
        gGlobalStateClientGame.chatchanged = true;
        if ((gGlobalStateClientGame.cursorposition > 1) ||
            (length(gGlobalStateClientGame.chattext) == 1))
        {
          gGlobalStateClientGame.currenttabcompleteplayer = 0;
          gGlobalStateClientGame.chattext.erase(gGlobalStateClientGame.cursorposition - 1,
                                                length(gGlobalStateClientGame.chattext));
          gGlobalStateClientGame.cursorposition -= 1;
          if (length(gGlobalStateClientGame.chattext) == 0)
          {
            gGlobalStateClientGame.chattext = gGlobalStateClientGame.lastchattext;
            clearchattext();
          }
        }
      }
      break;

      case SDLK_DELETE: {
        gGlobalStateClientGame.chatchanged = true;
        if (length(gGlobalStateClientGame.chattext) > gGlobalStateClientGame.cursorposition)
        {
          gGlobalStateClientGame.chattext.erase(gGlobalStateClientGame.cursorposition, 1);
          gGlobalStateClientGame.currenttabcompleteplayer = 0;
        }
      }
      break;

      case SDLK_HOME: {
        gGlobalStateClientGame.chatchanged = true;
        gGlobalStateClientGame.cursorposition = 1;
      }
      break;

      case SDLK_END: {
        gGlobalStateClientGame.chatchanged = true;
        gGlobalStateClientGame.cursorposition = length(gGlobalStateClientGame.chattext);
      }
      break;

      case SDLK_RIGHT: {
        gGlobalStateClientGame.chatchanged = true;
        if (length(gGlobalStateClientGame.chattext) > gGlobalStateClientGame.cursorposition)
        {
          gGlobalStateClientGame.cursorposition += 1;
        }
      }
      break;

      case SDLK_LEFT: {
        gGlobalStateClientGame.chatchanged = true;
        if (gGlobalStateClientGame.cursorposition > 1)
        {
          gGlobalStateClientGame.cursorposition -= 1;
        }
      }
      break;

      case SDLK_TAB:
        gGlobalStateClientGame.tabcomplete();
        break;

      case SDLK_RETURN:
      case SDLK_KP_ENTER: {
        if (gGlobalStateClientGame.chattext[1] == '/')
        {
          gGlobalStateClientGame.chattype = msgtype_cmd;
          consolestr = std::string(gGlobalStateClientGame.chattext).substr(2);
          if (parseinput(consolestr))
          {
            clearchattext();
            return result;
          }
        }
        if (sprite_system.IsPlayerSpriteValid())
        {
          if (gGlobalStateControlGame.votekickreasontype)
          {
            if (length(gGlobalStateClientGame.chattext) > 3)
            {
              clientvotekick(gGlobalStateGameMenus.kickmenuindex, false,
                             std::string(gGlobalStateClientGame.chattext));
              gGlobalStateControlGame.votekickreasontype = false;
            }
          }
          else
          {
            clientsendstringmessage(gGlobalStateClientGame.chattext.substr(1),
                                    gGlobalStateClientGame.chattype);
          }
        }

        clearchattext();
      }
      break;
      default:
        result = false;
      }
    }
  }
  return result;
}

auto menukeydown(std::uint8_t keymods, SDL_Scancode keycode) -> bool
{
  bool result;
  result = false;

  if ((keymods == km_none) && (keycode == SDL_SCANCODE_ESCAPE))
  {
    result = true;

    if (gGlobalStateClient.showradiomenu)
    {
      gGlobalStateClient.showradiomenu = false;
      gGlobalStateClient.rmenustate[0] = ' ';
      gGlobalStateClient.rmenustate[1] = ' ';
    }
    else if (gGlobalStateGameMenus.kickmenu->active || gGlobalStateGameMenus.mapmenu->active)
    {
      gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.escmenu);
    }
    else
    {
      gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.escmenu,
                                         !gGlobalStateGameMenus.escmenu->active);
    }
  }
  else if ((keycode >= SDL_SCANCODE_1) && (keycode <= SDL_SCANCODE_0))
  {
    if (gGlobalStateGameMenus.teammenu->active)
    {
      if (keymods == km_none)
      {
        result = gGlobalStateGameMenus.gamemenuaction(gGlobalStateGameMenus.teammenu,
                                                      ((keycode - SDL_SCANCODE_1) + 1) % 10);
      }
    }
    else if (gGlobalStateGameMenus.escmenu->active)
    {
      if (keymods == km_none)
      {
        result = gGlobalStateGameMenus.gamemenuaction(gGlobalStateGameMenus.escmenu,
                                                      keycode - SDL_SCANCODE_1);
      }
    }
    else if (gGlobalStateGameMenus.limbomenu->active)
    {
      switch (keymods)
      {
      case km_none:
        result = gGlobalStateGameMenus.gamemenuaction(gGlobalStateGameMenus.limbomenu,
                                                      keycode - SDL_SCANCODE_1);
        break;
      case km_ctrl:
        result = gGlobalStateGameMenus.gamemenuaction(gGlobalStateGameMenus.limbomenu,
                                                      keycode - SDL_SCANCODE_1 + 10);
        break;
      }
    }
  }
  return result;
}

auto keydown(SDL_KeyboardEvent &keyevent) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::uint8_t keymods;
  SDL_Scancode keycode;
  pbind bind;
  taction action;

  bool result = true;
  auto &game = GS::GetGame();
  keycode = keyevent.scancode;

  keymods = (ord(0 != (keyevent.mod & SDL_KMOD_ALT)) << 0) |
            (ord(0 != (keyevent.mod & SDL_KMOD_CTRL)) << 1) |
            (ord(0 != (keyevent.mod & SDL_KMOD_SHIFT)) << 2);

  if (chatkeydown(keymods, keyevent.key))
  {
    return result;
  }

  if (keyevent.repeat != 0)
  {
    result = false;
    return result;
  }

  if (menukeydown(keymods, keycode))
  {
    return result;
  }

  // other hard coded key bindings

  if (keymods == km_none)
  {
    switch (keycode)
    {
    case SDL_SCANCODE_PAGEDOWN: {
      if (gGlobalStateInterfaceGraphics.fragsmenushow)
      {
        gGlobalStateInterfaceGraphics.fragsscrolllev +=
          ord(gGlobalStateInterfaceGraphics.fragsscrolllev <
              gGlobalStateInterfaceGraphics.fragsscrollmax);
      }
    }
    break;

    case SDL_SCANCODE_PAGEUP: {
      if (gGlobalStateInterfaceGraphics.fragsmenushow)
      {
        gGlobalStateInterfaceGraphics.fragsscrolllev -=
          ord(gGlobalStateInterfaceGraphics.fragsscrolllev > 0);
      }
    }
    break;

    case SDL_SCANCODE_F11: {
      result = game.IsVoteActive();
      game.SetVoteActive(false);
    }
    break;

    case SDL_SCANCODE_F12: {
      result = game.IsVoteActive();

      if (game.IsVoteActive())
      {
        game.SetVoteActive(false);

        if (game.GetVoteType() == vote_map)
        {
          clientsendstringmessage(std::string("votemap ") + (game.GetVoteTarget()), msgtype_cmd);
          GS::GetMainConsole().console(wideformat(_("You have voted on " + game.GetVoteTarget())),
                                       vote_message_color);
        }
        else if (game.GetVoteType() == vote_kick)
        {
          i = strtoint(game.GetVoteTarget());
          clientvotekick(i, true, "");
          GS::GetMainConsole().console(
            wideformat(_("You have voted to kick " + sprite_system.GetSprite(i).player->name)),
            vote_message_color);
        }
      }
    }
    break;

    case SDL_SCANCODE_F9: {
      SDL_MinimizeWindow(gGlobalStateInput.gamewindow);
      result = true;
    }
    break;

    case SDL_SCANCODE_F8: {
      result = false;

      if (gGlobalStateDemo.demoplayer.active())
      {
        result = true;
        CVar::demo_speed = GS::GetGame().IsDefaultGoalTicks() ? 8.0 : 1.0;
      }
    }
    break;

    case SDL_SCANCODE_F10: {
      result = false;

      if (gGlobalStateDemo.demoplayer.active())
      {
        result = true;
        if ((GS::GetGame().GetMapchangecounter() < 0) ||
            (GS::GetGame().GetMapchangecounter() > 99999999))
        {
          if (GS::GetGame().GetMapchangecounter() < 0)
          {
            GS::GetGame().SetMapchangecounter(999999999);
          }
          else
          {
            GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
          }
        }
      }
    }
    break;

    case SDL_SCANCODE_1:
    case SDL_SCANCODE_2:
    case SDL_SCANCODE_3: {
      result = false;

      if ((gGlobalStateClientGame.chattext.empty()) && (CVar::sv_radio) &&
          gGlobalStateClient.showradiomenu)
      {
        result = true;
        i = ord(gGlobalStateClient.rmenustate[0] != ' ');

        switch (keycode)
        {
        case SDL_SCANCODE_1:
          gGlobalStateClient.rmenustate[i] = '1';
          break;
        case SDL_SCANCODE_2:
          gGlobalStateClient.rmenustate[i] = '2';
          break;
        case SDL_SCANCODE_3:
          gGlobalStateClient.rmenustate[i] = '3';
          break;
        default:
          break;
        }

        if (i == 1)
        {
          NotImplemented();
#if 0
                    chattext = (CVar::radiomenus[string("Menu1") +
                                                 choose(strtoint(rmenustate[0]) - 1,
                                                        set::of("EFC", "FFC", "ES", eos))] +
                                " " +
                                CVar::radiomenus[string("Menu2") +
                                                 choose(strtoint(rmenustate[0]) - 1,
                                                        set::of("EFC", "FFC", "ES", eos)) +
                                                 choose(strtoint(rmenustate[1]) - 1,
                                                        set::of('U', 'M', 'D', eos))]);
#endif

          clientsendstringmessage(gGlobalStateClientGame.chattext, msgtype_radio);
          gGlobalStateClientGame.chattext = "";
          // RadioCooldown := 3;
          gGlobalStateClient.showradiomenu = false;
          gGlobalStateClient.rmenustate[0] = ' ';
          gGlobalStateClient.rmenustate[1] = ' ';
        }
      }
    }
    break;

    default:
      result = false;
    }
  }
  else if (keymods == km_alt)
  {
    switch (keycode)
    {
    case SDL_SCANCODE_F4:
    case SDL_SCANCODE_F9:
      gGlobalStateClient.exittomenu();
      break;
    default:
      result = false;
    }
  }
  else if ((keymods == km_ctrl) || (keymods == km_shift))
  {
    result = false;
  }

  if (result)
  {
    return result;
  }

  // bindings
  bind = gGlobalStateInput.findkeybind(keymods, keycode);
  result = bind != nullptr;

  if (!result)
  {
    return result;
  }

  action = bind->action;

  if (action == taction::sniperline)
  {
    if (!CVar::sv_sniperline)
    {
      gGlobalStateClient.sniperline_client_hpp = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(gGlobalStateClient.sniperline_client_hpp) == 0u);
    }
  }
  else if (action == taction::statsmenu)
  {
    if (!gGlobalStateGameMenus.escmenu->active)
    {
      gGlobalStateInterfaceGraphics.statsmenushow = !gGlobalStateInterfaceGraphics.statsmenushow;
      if (gGlobalStateInterfaceGraphics.statsmenushow)
      {
        gGlobalStateInterfaceGraphics.fragsmenushow = false;
      }
    }
  }
  else if (action == taction::gamestats)
  {
    gGlobalStateInterfaceGraphics.coninfoshow = !gGlobalStateInterfaceGraphics.coninfoshow;
  }
  else if (action == taction::minimap)
  {
    gGlobalStateInterfaceGraphics.minimapshow = !gGlobalStateInterfaceGraphics.minimapshow;
  }
  else if (action == taction::playername)
  {
    gGlobalStateInterfaceGraphics.playernamesshow = !gGlobalStateInterfaceGraphics.playernamesshow;
  }
  else if (action == taction::fragslist)
  {
    if (!gGlobalStateGameMenus.escmenu->active)
    {
      gGlobalStateInterfaceGraphics.fragsscrolllev = 0;
      gGlobalStateInterfaceGraphics.fragsmenushow = !gGlobalStateInterfaceGraphics.fragsmenushow;
      if (gGlobalStateInterfaceGraphics.fragsmenushow)
      {
        gGlobalStateInterfaceGraphics.statsmenushow = false;
      }
    }
  }
  else if (action == taction::radio)
  {
    if ((gGlobalStateClientGame.chattext.empty()) && (CVar::sv_radio) &&
        (sprite_system.IsPlayerSpriteValid()) and
        (sprite_system.GetPlayerSprite().isnotspectator)())
    {
      gGlobalStateClient.showradiomenu = !gGlobalStateClient.showradiomenu;
      gGlobalStateClient.rmenustate[0] = ' ';
      gGlobalStateClient.rmenustate[1] = ' ';
    }
  }
  else if (action == taction::recorddemo)
  {
    if (!gGlobalStateDemo.demoplayer.active())
    {
      if (CVar::demo_autorecord)
      {
        GS::GetDemoRecorder().stoprecord();
        NotImplemented();
#if 0
                GS::GetDemoRecorder().startrecord(GS::GetGame().GetUserDirectory() + "demos/" +
                                         sniperline_client_hppformatdatetime("yyyy-mm-dd_hh-nn-ss_", now(0)) + map.name +
                                         ".sdm");
#endif
      }
      else if (GS::GetDemoRecorder().active())
      {
        GS::GetDemoRecorder().stoprecord();
      }
      else
      {
        NotImplemented();
#if 0
                GS::GetDemoRecorder().startrecord(GS::GetGame().GetUserDirectory() + "demos/" +
                                         formatdatetime("yyyy-mm-dd_hh-nn-ss_", now(0)) + map.name +
                                         ".sdm");
#endif
      }
    }
  }
  else if ((action == taction::volumeup) || (action == taction::volumedown))
  {
    if ((gGlobalStateClientGame.chattext.empty()) && !gGlobalStateGameMenus.escmenu->active)
    {
      i = CVar::snd_volume;

      if (action == taction::volumeup)
      {
        CVar::snd_volume = (min(CVar::snd_volume + 10, 100));
      }

      if (action == taction::volumedown)
      {
        CVar::snd_volume = (max(CVar::snd_volume - 10, 0));
      }

      if (CVar::snd_volume != i)
      {
        gGlobalStateSound.volumeinternal = gGlobalStateSound.scalevolumesetting(CVar::snd_volume);
        gGlobalStateSound.setvolume(-1, gGlobalStateSound.volumeinternal);
        GS::GetMainConsole().console(std::string("Volume: ") + inttostr(CVar::snd_volume) + "%",
                                     music_message_color);
      }
    }
  }
  else if ((action == taction::mousesensitivityup) || (action == taction::mousesensitivitydown))
  {
    if ((gGlobalStateClientGame.chattext.empty()) && !gGlobalStateGameMenus.escmenu->active)
    {
      i = iif(action == taction::mousesensitivitydown, -5, 5);
      CVar::cl_sensitivity = ((float)(max(0.0f, i + floor(100 * CVar::cl_sensitivity))) / 100);
      GS::GetMainConsole().console(
        _("Sensitivity:") + (std::string(" ") + inttostr(floor(100 * CVar::cl_sensitivity)) + "%"),
        music_message_color);
    }
  }
  else if (action == taction::cmd)
  {
    if (gGlobalStateClientGame.chattext.empty())
    {
      gGlobalStateClientGame.chattext = '/';
      gGlobalStateClientGame.chattype = msgtype_cmd;
      gGlobalStateClientGame.chatchanged = true;
      gGlobalStateClientGame.cursorposition = 1;
      gGlobalStateControlGame.votekickreasontype = false;
      SDL_StartTextInput(gGlobalStateInput.gamewindow);
    }
  }
  else if (action == taction::chat)
  {
    if (gGlobalStateClientGame.chattext.empty())
    {
      SDL_StartTextInput(gGlobalStateInput.gamewindow);
      gGlobalStateClientGame.chatchanged = true;
      gGlobalStateClientGame.chattext = ' ';
      gGlobalStateClientGame.chattype = msgtype_pub;

      if (length(gGlobalStateClientGame.firechattext) > 0)
      {
        gGlobalStateClientGame.chattext = gGlobalStateClientGame.firechattext;
      }

      // force spectator chat to teamchat in survival mode when Round hasn't ended
      if ((CVar::sv_survivalmode) && sprite_system.GetPlayerSprite().isspectator() &&
          !game.GetSurvivalEndRound() && (CVar::sv_survivalmode_antispy))
      {
        gGlobalStateClientGame.chattype = msgtype_team;
      }

      gGlobalStateClientGame.cursorposition = length(gGlobalStateClientGame.chattext);
    }
  }
  else if (action == taction::teamchat)
  {
    if ((gGlobalStateClientGame.chattext.empty()) && (sprite_system.IsPlayerSpriteValid()) &&
        (sprite_system.GetPlayerSprite().isspectator() || GS::GetGame().isteamgame()))
    {
      SDL_StartTextInput(gGlobalStateInput.gamewindow);
      gGlobalStateClientGame.chattext = ' ';
      gGlobalStateClientGame.chattype = msgtype_team;
      gGlobalStateClientGame.chatchanged = true;
      gGlobalStateClientGame.cursorposition = length(gGlobalStateClientGame.chattext);
    }
  }
  else if (action == taction::snap)
  {
    if ((CVar::cl_actionsnap) && (gGlobalStateClientGame.screencounter < 255) &&
        gGlobalStateClientGame.actionsnaptaken)
    {
      gGlobalStateClientGame.showscreen = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(gGlobalStateClientGame.showscreen) == 0u);

      if (!static_cast<bool>(gGlobalStateClientGame.showscreen))
      {
        gGlobalStateClientGame.screencounter = 255;
      }
      else
      {
        gGlobalStateSound.playsound(SfxEffect::snapshot);
      }
    }
    else
    {
      gGlobalStateClientGame.screencounter = 255;
      gGlobalStateClientGame.showscreen = 0u;
    }
  }
  else if (action == taction::weapons)
  {
    if ((gGlobalStateClientGame.chattext.empty()) && (sprite_system.IsPlayerSpriteValid()) &&
        !gGlobalStateGameMenus.escmenu->active && !sprite_system.GetPlayerSprite().isspectator())
    {
      if (sprite_system.GetPlayerSprite().deadmeat)
      {
        gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.limbomenu,
                                           !gGlobalStateGameMenus.limbomenu->active);
        gGlobalStateClient.limbolock = !gGlobalStateGameMenus.limbomenu->active;
        GS::GetMainConsole().console(
          iif(gGlobalStateClient.limbolock, _("Weapons menu disabled"), _("Weapons menu active")),
          game_message_color);
      }
      else
      {
        auto &weaponSystem = GS::GetWeaponSystem();
        auto pricount = weaponSystem.CountEnabledPrimaryWeapons();
        auto seccount = weaponSystem.CountEnabledSecondaryWeapons();
        auto prinum = sprite_system.GetPlayerSprite().weapon.num;
        auto secnum = sprite_system.GetPlayerSprite().secondaryweapon.num;

        if (!gGlobalStateGameMenus.limbomenu->active or
            (((prinum != noweapon_num) || (pricount == 0)) &&
             ((secnum != noweapon_num) || (seccount == 0))))
        {
          gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.limbomenu, false);
          gGlobalStateClient.limbolock = !gGlobalStateClient.limbolock;
          GS::GetMainConsole().console(
            iif(gGlobalStateClient.limbolock, _("Weapons menu disabled"), _("Weapons menu active")),
            game_message_color);
        }
      }
    }
  }
  else if (action == taction::bind)
  {
    if ((gGlobalStateClientGame.chattimecounter == 0) && (!bind->command.empty()))
    {
      if ((gGlobalStateClientGame.chattext.empty()) && !gGlobalStateGameMenus.escmenu->active)
      {
        if (!parseinput(std::string(bind->command)))
        {
          clientsendstringmessage(bind->command, msgtype_cmd);
        }
      }
    }
  }
  else if (action == taction::voicechat)
  {
  }
  else
  {
    result = false;
  }
  return result;
}

auto keyup(SDL_KeyboardEvent &keyevent) -> bool
{
  std::uint8_t keymods;
  SDL_Scancode keycode;
  pbind bind;
  taction action;

  bool result;
  result = true;
  keycode = keyevent.scancode;

  keymods = (ord(0 != (keyevent.mod & SDL_KMOD_ALT)) << 0) |
            (ord(0 != (keyevent.mod & SDL_KMOD_CTRL)) << 1) |
            (ord(0 != (keyevent.mod & SDL_KMOD_SHIFT)) << 2);

  if (keyevent.repeat != 0)
  {
    result = false;
    return result;
  }

  // bindings
  bind = gGlobalStateInput.findkeybind(keymods, keycode);
  result = bind != nullptr;

  if (!result)
  {
    return result;
  }

  action = bind->action;

  if (action == taction::voicechat)
  {
  }
  else
  {
    result = false;
  }
  return result;
}

void GlobalStateControlGame::gameinput(SDL_Event &event)
{
  std::string str1;
  bool chatenabled;

  chatenabled = length(gGlobalStateClientGame.chattext) > 0;

  {
    switch (event.type)
    {
    case SDL_EVENT_QUIT: {
      clientdisconnect(*gGlobalStateNetworkClient.GetNetwork());
      gGlobalStateClient.shutdown();
    }
    break;

    case SDL_EVENT_KEY_DOWN: {
      if (!keydown(event.key))
      {
        gGlobalStateInput.keystatus[event.key.scancode] = true;
      }
    }
    break;

    case SDL_EVENT_KEY_UP: {
      gGlobalStateInput.keystatus[event.key.scancode] = false;
      keyup(event.key);
    }
    break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (!gGlobalStateGameMenus.gamemenuclick())
      {
        gGlobalStateInput.keystatus[event.button.button + 300] = true;
      }
    }
    break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      gGlobalStateInput.keystatus[event.button.button + 300] = false;
      break;

    case SDL_EVENT_TEXT_INPUT: {
      if (chatenabled)
      {
        str1 = event.text.text[0];
        str1 = filterchattext(str1);

        if ((gGlobalStateClientGame.chattext == "/") && (str1 == "/") &&
            (length(gGlobalStateClientGame.lastchattext) > 1))
        {
          gGlobalStateClientGame.chatchanged = true;
          gGlobalStateClientGame.currenttabcompleteplayer = 0;
          gGlobalStateClientGame.chattext = gGlobalStateClientGame.lastchattext;
          gGlobalStateClientGame.cursorposition = length(gGlobalStateClientGame.chattext);
        }
        else if (length(gGlobalStateClientGame.chattext) > 0)
        {
          if (length(gGlobalStateClientGame.chattext) <
              iif(gGlobalStateControlGame.votekickreasontype, REASON_CHARS - 1, maxchattext))
          {
            gGlobalStateClientGame.chatchanged = true;
            gGlobalStateClientGame.currenttabcompleteplayer = 0;
            if (gGlobalStateClientGame.cursorposition + 1 > gGlobalStateClientGame.chattext.size())
            {
              gGlobalStateClientGame.chattext.append(str1);
            }
            else
            {
              gGlobalStateClientGame.chattext.insert(gGlobalStateClientGame.cursorposition + 1,
                                                     str1);
            }
            gGlobalStateClientGame.cursorposition += length(str1);
          }
        }
      }
    }
    break;

    case SDL_EVENT_MOUSE_MOTION: {
      if (0 != (SDL_GetWindowFlags(gGlobalStateInput.gamewindow) & SDL_WINDOW_INPUT_FOCUS))
      {
        gGlobalStateClientGame.mx =
          max(0.f, min((float)gGlobalStateGame.gamewidth,
                       gGlobalStateClientGame.mx + event.motion.xrel * CVar::cl_sensitivity));
        gGlobalStateClientGame.my =
          max(0.f, min((float)gGlobalStateGame.gameheight,
                       gGlobalStateClientGame.my + event.motion.yrel * CVar::cl_sensitivity));

        gGlobalStateGameMenus.gamemenumousemove();
      }
    }
    break;
    }
  }
}
