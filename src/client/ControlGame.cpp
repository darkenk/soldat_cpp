// automatically converted

#include "ControlGame.hpp"

#include "Client.hpp"
#include "ClientGame.hpp"
#include "GameMenus.hpp"
#include "GameStrings.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "common/Logging.hpp"
#include "shared/Command.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClientBullet.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "shared/network/NetworkClientGame.hpp"
#include "shared/network/NetworkClientMessages.hpp"
#include <SDL2/SDL.h>
#include <numeric>

bool votekickreasontype = false;

void clearchattext()
{
  lastchattext = chattext;
  firechattext = "";
  completionbase = "";
  currenttabcompleteplayer = 0;
  cursorposition = 1;
  votekickreasontype = false;
  chattext = "";
  SDL_StopTextInput();
}

std::string filterchattext(const std::string &str1)
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

bool chatkeydown(std::uint8_t keymods, SDL_Keycode keycode)
{
  std::int32_t len;
  std::string str1;
  std::string consolestr;

  bool result;
  result = false;

  if (length(chattext) > 0)
  {
    if ((keymods == km_ctrl) && (keycode == SDLK_v))
    {
      str1 = filterchattext(SDL_GetClipboardText());
      len = length(chattext);
      chattext.insert(cursorposition + 1, str1);
      cursorposition += length(chattext) - len;

      len = iif(votekickreasontype, REASON_CHARS - 1, maxchattext);

      if (length(chattext) > len)
      {
        chattext = chattext.substr(1);
        cursorposition = min(cursorposition, (std::uint8_t)len);
      }

      currenttabcompleteplayer = 0;
      chatchanged = true;
      result = true;
    }
    else if (keymods == km_none)
    {
      result = true;

      switch (keycode)
      {
      case SDLK_ESCAPE: {
        chattext = lastchattext;
        clearchattext();
      }
      break;

      case SDLK_BACKSPACE: {
        chatchanged = true;
        if ((cursorposition > 1) || (length(chattext) == 1))
        {
          currenttabcompleteplayer = 0;
          chattext.erase(cursorposition - 1, length(chattext));
          cursorposition -= 1;
          if (length(chattext) == 0)
          {
            chattext = lastchattext;
            clearchattext();
          }
        }
      }
      break;

      case SDLK_DELETE: {
        chatchanged = true;
        if (length(chattext) > cursorposition)
        {
          chattext.erase(cursorposition, 1);
          currenttabcompleteplayer = 0;
        }
      }
      break;

      case SDLK_HOME: {
        chatchanged = true;
        cursorposition = 1;
      }
      break;

      case SDLK_END: {
        chatchanged = true;
        cursorposition = length(chattext);
      }
      break;

      case SDLK_RIGHT: {
        chatchanged = true;
        if (length(chattext) > cursorposition)
          cursorposition += 1;
      }
      break;

      case SDLK_LEFT: {
        chatchanged = true;
        if (cursorposition > 1)
          cursorposition -= 1;
      }
      break;

      case SDLK_TAB:
        tabcomplete();
        break;

      case SDLK_RETURN:
      case SDLK_KP_ENTER: {
        if (chattext[1] == '/')
        {
          chattype = msgtype_cmd;
          consolestr = std::string(chattext).substr(2);
          if (parseinput(consolestr))
          {
            clearchattext();
            return result;
          }
        }
        if (mysprite > 0)
        {
          if (votekickreasontype)
          {
            if (length(chattext) > 3)
            {
              clientvotekick(kickmenuindex, false, std::string(chattext));
              votekickreasontype = false;
            }
          }
          else
          {
            clientsendstringmessage(chattext.substr(1), chattype);
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

bool menukeydown(std::uint8_t keymods, SDL_Scancode keycode)
{
  bool result;
  result = false;

  if ((keymods == km_none) && (keycode == SDL_SCANCODE_ESCAPE))
  {
    result = true;

    if (showradiomenu)
    {
      showradiomenu = false;
      rmenustate[0] = ' ';
      rmenustate[1] = ' ';
    }
    else if (kickmenu->active || mapmenu->active)
      gamemenushow(escmenu);
    else
      gamemenushow(escmenu, !escmenu->active);
  }
  else if ((keycode >= SDL_SCANCODE_1) && (keycode <= SDL_SCANCODE_0))
  {
    if (teammenu->active)
    {
      if (keymods == km_none)
        result = gamemenuaction(teammenu, ((keycode - SDL_SCANCODE_1) + 1) % 10);
    }
    else if (escmenu->active)
    {
      if (keymods == km_none)
        result = gamemenuaction(escmenu, keycode - SDL_SCANCODE_1);
    }
    else if (limbomenu->active)
    {
      switch (keymods)
      {
      case km_none:
        result = gamemenuaction(limbomenu, keycode - SDL_SCANCODE_1);
        break;
      case km_ctrl:
        result = gamemenuaction(limbomenu, keycode - SDL_SCANCODE_1 + 10);
        break;
      }
    }
  }
  return result;
}

bool keydown(SDL_KeyboardEvent &keyevent)
{
  std::int32_t i;
  std::uint8_t keymods;
  SDL_Scancode keycode;
  pbind bind;
  taction action;

  bool result = true;
  auto &game = GS::GetGame();
  keycode = keyevent.keysym.scancode;

  keymods = (ord(0 != (keyevent.keysym.mod & KMOD_ALT)) << 0) |
            (ord(0 != (keyevent.keysym.mod & KMOD_CTRL)) << 1) |
            (ord(0 != (keyevent.keysym.mod & KMOD_SHIFT)) << 2);

  if (chatkeydown(keymods, keyevent.keysym.sym))
    return result;

  if (keyevent.repeat != 0)
  {
    result = false;
    return result;
  }

  if (menukeydown(keymods, keycode))
    return result;

  // other hard coded key bindings

  if (keymods == km_none)
    switch (keycode)
    {
    case SDL_SCANCODE_PAGEDOWN: {
      if (fragsmenushow)
        fragsscrolllev += ord(fragsscrolllev < fragsscrollmax);
    }
    break;

    case SDL_SCANCODE_PAGEUP: {
      if (fragsmenushow)
        fragsscrolllev -= ord(fragsscrolllev > 0);
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
          GetMainConsole().console(wideformat(_("You have voted on " + game.GetVoteTarget())),
                                   vote_message_color);
        }
        else if (game.GetVoteType() == vote_kick)
        {
          i = strtoint(game.GetVoteTarget());
          clientvotekick(i, true, "");
          GetMainConsole().console(wideformat(_("You have voted to kick " +
                                                SpriteSystem::Get().GetSprite(i).player->name)),
                                   vote_message_color);
        }
      }
    }
    break;

    case SDL_SCANCODE_F9: {
      SDL_MinimizeWindow(gamewindow);
      result = true;
    }
    break;

    case SDL_SCANCODE_F8: {
      result = false;

      if (demoplayer.active())
      {
        result = true;
        CVar::demo_speed = GS::GetGame().IsDefaultGoalTicks() ? 8.0 : 1.0;
      }
    }
    break;

    case SDL_SCANCODE_F10: {
      result = false;

      if (demoplayer.active())
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

      if ((chattext == "") & (CVar::sv_radio) && showradiomenu)
      {
        result = true;
        i = ord(rmenustate[0] != ' ');

        switch (keycode)
        {
        case SDL_SCANCODE_1:
          rmenustate[i] = '1';
          break;
        case SDL_SCANCODE_2:
          rmenustate[i] = '2';
          break;
        case SDL_SCANCODE_3:
          rmenustate[i] = '3';
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

          clientsendstringmessage(chattext, msgtype_radio);
          chattext = "";
          // RadioCooldown := 3;
          showradiomenu = false;
          rmenustate[0] = ' ';
          rmenustate[1] = ' ';
        }
      }
    }
    break;

    default:
      result = false;
    }
  else if (keymods == km_alt)
    switch (keycode)
    {
    case SDL_SCANCODE_F4:
      exittomenu();
      break;

    case SDL_SCANCODE_F9:
      exittomenu();
      break;
    default:
      result = false;
    }
  else if ((keymods == km_ctrl) || (keymods == km_shift))
    result = false;

  if (result)
    return result;

  // bindings
  bind = findkeybind(keymods, keycode);
  result = bind != nullptr;

  if (!result)
    return result;

  action = bind->action;

  if (action == taction::sniperline)
  {
    if (!CVar::sv_sniperline)
      sniperline_client_hpp = !sniperline_client_hpp;
  }
  else if (action == taction::statsmenu)
  {
    if (!escmenu->active)
    {
      statsmenushow = !statsmenushow;
      if (statsmenushow)
        fragsmenushow = false;
    }
  }
  else if (action == taction::gamestats)
  {
    coninfoshow = !coninfoshow;
  }
  else if (action == taction::minimap)
  {
    minimapshow = !minimapshow;
  }
  else if (action == taction::playername)
  {
    playernamesshow = !playernamesshow;
  }
  else if (action == taction::fragslist)
  {
    if (!escmenu->active)
    {
      fragsscrolllev = 0;
      fragsmenushow = !fragsmenushow;
      if (fragsmenushow)
        statsmenushow = false;
    }
  }
  else if (action == taction::radio)
  {
    if ((chattext == "") && (CVar::sv_radio) && (mysprite > 0) and
        (SpriteSystem::Get().GetSprite(mysprite).isnotspectator)())
    {
      showradiomenu = !showradiomenu;
      rmenustate[0] = ' ';
      rmenustate[1] = ' ';
    }
  }
  else if (action == taction::recorddemo)
  {
    if (!demoplayer.active())
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
    if ((chattext == "") && !escmenu->active)
    {
      i = CVar::snd_volume;

      if (action == taction::volumeup)
        CVar::snd_volume = (min(CVar::snd_volume + 10, 100));

      if (action == taction::volumedown)
        CVar::snd_volume = (max(CVar::snd_volume - 10, 0));

      if (CVar::snd_volume != i)
      {
        volumeinternal = scalevolumesetting(CVar::snd_volume);
        setvolume(-1, volumeinternal);
        GetMainConsole().console(std::string("Volume: ") + inttostr(CVar::snd_volume) + "%",
                                 music_message_color);
      }
    }
  }
  else if ((action == taction::mousesensitivityup) || (action == taction::mousesensitivitydown))
  {
    if ((chattext == "") && !escmenu->active)
    {
      i = iif(action == taction::mousesensitivitydown, -5, 5);
      CVar::cl_sensitivity = ((float)(max(0.f, i + floor(100 * CVar::cl_sensitivity))) / 100);
      GetMainConsole().console(
        _("Sensitivity:") + (std::string(" ") + inttostr(floor(100 * CVar::cl_sensitivity)) + "%"),
        music_message_color);
    }
  }
  else if (action == taction::cmd)
  {
    if (chattext == "")
    {
      chattext = '/';
      chattype = msgtype_cmd;
      chatchanged = true;
      cursorposition = 1;
      votekickreasontype = false;
      SDL_StartTextInput();
    }
  }
  else if (action == taction::chat)
  {
    if (chattext == "")
    {
      SDL_StartTextInput();
      chatchanged = true;
      chattext = ' ';
      chattype = msgtype_pub;

      if (length(firechattext) > 0)
        chattext = firechattext;

      // force spectator chat to teamchat in survival mode when Round hasn't ended
      if ((CVar::sv_survivalmode)&SpriteSystem::Get().GetSprite(mysprite).isspectator() &&
          !game.GetSurvivalEndRound() && (CVar::sv_survivalmode_antispy))
        chattype = msgtype_team;

      cursorposition = length(chattext);
    }
  }
  else if (action == taction::teamchat)
  {
    if ((chattext == "") && (mysprite > 0) &&
        (SpriteSystem::Get().GetSprite(mysprite).isspectator() || GS::GetGame().isteamgame()))
    {
      SDL_StartTextInput();
      chattext = ' ';
      chattype = msgtype_team;
      chatchanged = true;
      cursorposition = length(chattext);
    }
  }
  else if (action == taction::snap)
  {
    if ((CVar::cl_actionsnap) && (screencounter < 255) && actionsnaptaken)
    {
      showscreen = !showscreen;

      if (showscreen == false)
        screencounter = 255;
      else
        playsound(SfxEffect::snapshot);
    }
    else
    {
      screencounter = 255;
      showscreen = false;
    }
  }
  else if (action == taction::weapons)
  {
    if ((chattext == "") && (mysprite > 0) && !escmenu->active &&
        !SpriteSystem::Get().GetSprite(mysprite).isspectator())
    {
      if (SpriteSystem::Get().GetSprite(mysprite).deadmeat)
      {
        gamemenushow(limbomenu, !limbomenu->active);
        limbolock = !limbomenu->active;
        GetMainConsole().console(
          iif(limbolock, _("Weapons menu disabled"), _("Weapons menu active")), game_message_color);
      }
      else
      {
        auto &weaponSystem = GS::GetWeaponSystem();
        auto pricount = weaponSystem.CountEnabledPrimaryWeapons();
        auto seccount = weaponSystem.CountEnabledSecondaryWeapons();
        auto prinum = SpriteSystem::Get().GetSprite(mysprite).weapon.num;
        auto secnum = SpriteSystem::Get().GetSprite(mysprite).secondaryweapon.num;

        if (!limbomenu->active or (((prinum != noweapon_num) || (pricount == 0)) &&
                                   ((secnum != noweapon_num) || (seccount == 0))))
        {
          gamemenushow(limbomenu, false);
          limbolock = !limbolock;
          GetMainConsole().console(
            iif(limbolock, _("Weapons menu disabled"), _("Weapons menu active")),
            game_message_color);
        }
      }
    }
  }
  else if (action == taction::bind)
  {
    if ((chattimecounter == 0) && (bind->command != ""))
    {
      if ((chattext == "") && !escmenu->active)
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
#ifdef STEAM
    if (!voicespeakingnow)
    {
      steamapi.user.startvoicerecording(0);
      voicespeakingnow = true;
    }
#endif
  }
  else
  {
    result = false;
  }
  return result;
}

bool keyup(SDL_KeyboardEvent &keyevent)
{
  std::uint8_t keymods;
  SDL_Scancode keycode;
  pbind bind;
  taction action;

  bool result;
  result = true;
  keycode = keyevent.keysym.scancode;

  keymods = (ord(0 != (keyevent.keysym.mod & KMOD_ALT)) << 0) |
            (ord(0 != (keyevent.keysym.mod & KMOD_CTRL)) << 1) |
            (ord(0 != (keyevent.keysym.mod & KMOD_SHIFT)) << 2);

  if (keyevent.repeat != 0)
  {
    result = false;
    return result;
  }

  // bindings
  bind = findkeybind(keymods, keycode);
  result = bind != nullptr;

  if (!result)
    return result;

  action = bind->action;

  if (action == taction::voicechat)
  {
#ifdef STEAM
    if (voicespeakingnow)
    {
      steamapi.user.stopvoicerecording(0);
      voicespeakingnow = false;
    }
#endif
  }
  else
  {
    result = false;
  }
  return result;
}

void gameinput()
{
  SDL_Event event;
  std::string str1;
  bool chatenabled;

  chatenabled = length(chattext) > 0;

  while (SDL_PollEvent(&event) == 1)
  {
    switch (event.type)
    {
    case SDL_QUIT: {
      clientdisconnect();
      shutdown();
    }
    break;

    case SDL_KEYDOWN: {
      if (!keydown(event.key))
        keystatus[event.key.keysym.scancode] = true;
    }
    break;

    case SDL_KEYUP: {
      keystatus[event.key.keysym.scancode] = false;
      keyup(event.key);
    }
    break;

    case SDL_MOUSEBUTTONDOWN: {
      if (!gamemenuclick())
        keystatus[event.button.button + 300] = true;
    }
    break;

    case SDL_MOUSEBUTTONUP:
      keystatus[event.button.button + 300] = false;
      break;

    case SDL_TEXTINPUT: {
      if (chatenabled)
      {
        str1 = event.text.text[0];
        str1 = filterchattext(str1);

        if ((chattext == "/") && (str1 == "/") && (length(lastchattext) > 1))
        {
          chatchanged = true;
          currenttabcompleteplayer = 0;
          chattext = lastchattext;
          cursorposition = length(chattext);
        }
        else if (length(chattext) > 0)
        {
          if (length(chattext) < iif(votekickreasontype, REASON_CHARS - 1, maxchattext))
          {
            chatchanged = true;
            currenttabcompleteplayer = 0;
            if (cursorposition + 1 > chattext.size())
            {
              chattext.append(str1);
            }
            else
            {
              chattext.insert(cursorposition + 1, str1);
            }
            cursorposition += length(str1);
          }
        }
      }
    }
    break;

    case SDL_MOUSEMOTION: {
      if (0 != (SDL_GetWindowFlags(gamewindow) & SDL_WINDOW_INPUT_FOCUS))
      {
        mx = max(0.f, min((float)gamewidth, mx + event.motion.xrel * CVar::cl_sensitivity));
        my = max(0.f, min((float)gameheight, my + event.motion.yrel * CVar::cl_sensitivity));

        gamemenumousemove();
      }
    }
    break;
    }
  }
}
