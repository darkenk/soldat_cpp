// automatically converted
#include "ClientCommands.hpp"
#include "Client.hpp"
#include "ClientGame.hpp"
#include "GameRendering.hpp"
#include "Input.hpp"
#include "Sound.hpp"
#include "common/Util.hpp"
#include "common/Console.hpp"
#include "shared/Command.hpp"
#include "shared/Constants.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClientMessages.hpp"
#include <regex>

std::uint8_t screenshotsinarow = 0;

namespace
{

void commandbind(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string bindkeyname;
  std::string commandstring;
  std::uint64_t modifier;

  if (length(args) < 3)
  {
    GS::GetMainConsole().console(R"(Usage: bind "key" "command")", game_message_color);
    return;
  }

  bindkeyname = lowercase(args[1]);
  commandstring = args[2];
  modifier = km_none;

  if (bindkeyname.find("+") != std::string::npos)
  {
    if (bindkeyname.find("ctrl") != std::string::npos)
    {
      modifier = modifier | km_ctrl;
    }
    if (bindkeyname.find("shift") != std::string::npos)
    {
      modifier = modifier | km_shift;
    }
    if (bindkeyname.find("alt") != std::string::npos)
    {
      modifier = modifier | km_alt;
    }

    bindkeyname = std::regex_replace(bindkeyname, std::regex("shift"), "");
    bindkeyname = std::regex_replace(bindkeyname, std::regex("alt"), "");
    bindkeyname = std::regex_replace(bindkeyname, std::regex("ctrl"), "");
    bindkeyname = std::regex_replace(bindkeyname, std::regex("\\+"), "");
  }

  if (args[2][0] == '+')
  {
    bindkey(bindkeyname, commandstring, commandstring, modifier);
  }
  else
  {
    bindkey(bindkeyname, "+bind", commandstring, modifier);
  }
}

void commandconnect(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string s;

  if (length(args) <= 1)
  {
    GS::GetMainConsole().console("Usage: connect ip port password", game_message_color);
    return;
  }
  gGlobalStateClient.gClient.exittomenu();
  if (args[0] == "joinurl")
  {
    s = args[1];
    gGlobalStateClient.joinip = getpiece(s, '/', 2);
    gGlobalStateClient.joinip = getpiece(gGlobalStateClient.joinip, ':', 1);

    gGlobalStateClient.joinport = getpiece(s, ':', 3);
    gGlobalStateClient.joinport = getpiece(gGlobalStateClient.joinport, '/', 1);
    NotImplemented();
#if 0
        joinport = ansireplacestr(joinport, '/', "");
#endif

    gGlobalStateClient.joinpassword = getpiece(s, '/', 4);
  }
  else
  {
    gGlobalStateClient.joinip = args[1];
    if (length(args) == 2)
    {
      gGlobalStateClient.joinport = "23073";
    }
    else
    {
      gGlobalStateClient.joinport = args[2];
    }
    if (length(args) > 3)
    {
      gGlobalStateClient.joinpassword = args[3];
    }
  }
  gGlobalStateClient.gClient.joinserver();
}

void commandretry(std::vector<std::string> &args, std::uint8_t sender)
{
  gGlobalStateClient.gClient.exittomenu();
  gGlobalStateClient.gClient.joinserver();
}

void commanddisconnect(std::vector<std::string> &args, std::uint8_t sender)
{
  gGlobalStateClient.gClient.exittomenu();
}

void commandsay(std::vector<std::string> &args, std::uint8_t sender)
{
  if (length(args) <= 1)
  {
    GS::GetMainConsole().console("Usage: say \"text\"", game_message_color);
    return;
  }
  clientsendstringmessage((args[1]), msgtype_pub);
}

void commandsayteam(std::vector<std::string> &args, std::uint8_t sender)
{
  if (length(args) <= 1)
  {
    GS::GetMainConsole().console("Usage: say_team \"text\"", game_message_color);
    return;
  }
  clientsendstringmessage((args[1]), msgtype_team);
}

void commandrecord(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string str1;

  NotImplemented();
#if 0
    if (length(args) == 2)
        str1 = args[1];
    else
        str1 = formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name;
#endif

  GS::GetDemoRecorder().stoprecord();
  GS::GetDemoRecorder().startrecord(GS::GetGame().GetUserDirectory() + "demos/" + str1 + ".sdm");
}

void commandmute(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string str1;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  str1 = args[1];

  if (str1 == "all")
  {
    gGlobalStateClient.muteall = !gGlobalStateClient.muteall;

    if (gGlobalStateClient.muteall)
    {
      GS::GetMainConsole().console(("Everyone is muted"), client_message_color);
    }
    else
    {
      GS::GetMainConsole().console(("Everyone is unmuted"), client_message_color);
    }

    return;
  }

  targets = commandtarget(str1, sender);
  for (i = 0; i <= high(targets); i++)
  {
    sprite_system.GetSprite(targets[i]).muted = true;
    GS::GetMainConsole().console(sprite_system.GetSprite(targets[i]).player->name + " is muted",
                                 client_message_color);
  }
}

void commandunbindall(std::vector<std::string> &args, std::uint8_t sender)
{
  unbindall();
  GS::GetMainConsole().console("Unbinded all binds", game_message_color);
}

void commandunmute(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string str1;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  str1 = args[1];

  targets = commandtarget(str1, sender);
  for (i = 0; i <= high(targets); i++)
  {
    sprite_system.GetSprite(targets[i]).muted = false;
    GS::GetMainConsole().console(sprite_system.GetSprite(targets[i]).player->name + " is unmuted",
                                 client_message_color);
  }
}

void commandstop(std::vector<std::string> &args, std::uint8_t sender)
{
  GS::GetDemoRecorder().stoprecord();
}

void commandshutdown(std::vector<std::string> &args, std::uint8_t sender)
{
  gGlobalStateClient.gClient.exittomenu();
}

void commandscreenshot(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string screenfile;

  if (screenshotsinarow < 3)
  {
    screenshotsinarow += 1;
    NotImplemented();
#if 0
        screenfile = userdirectory + "screens/" + formatdatetime("yyyy-mm-dd_hh-nn-ss_", now(0)) +
                     map.name + "_screenshot.png";
#endif

    GS::GetMainConsole().console((("Screenshot saved to ") + screenfile), debug_message_color);

    takescreenshot(screenfile);

    playsound(SfxEffect::snapshot);

    if (gGlobalStateClientGame.showscreen != 0u)
    {
      gGlobalStateClientGame.showscreen = 0u;
    }
  }
}

void commandswitchcam(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  if (length(args) <= 1)
  {
    GS::GetMainConsole().console("Usage: switchcam \"id\"", game_message_color);
    return;
  }
  if (sprite_system.GetPlayerSprite().isspectator())
  {
    gGlobalStateClient.camerafollowsprite = strtointdef(args[1], 0);
  }
}

void commandswitchcamflag(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;

  if (length(args) <= 1)
  {
    GS::GetMainConsole().console("Usage: switchcamflag \"id\"", game_message_color);
    return;
  }

  auto &things = GS::GetThingSystem().GetThings();

  if (sprite_system.GetPlayerSprite().isspectator())
  {
    for (i = 1; i <= max_things; i++)
    {
      if (things[i].style == strtointdef(args[1], 0))
      {
        gGlobalStateClient.camerafollowsprite = 0;
        gGlobalStateClient.camerax = things[i].skeleton.pos[1].x;
        gGlobalStateClient.cameray = things[i].skeleton.pos[1].y;
      }
    }
  }
}

void commanddemotick(std::vector<std::string> &args, std::uint8_t sender)
{
  if (args[0] == "demo_tick")
  {
    gGlobalStateDemo.demoplayer.position(strtointdef(args[1], 0));
  }
  else
  {
    gGlobalStateDemo.demoplayer.position(GS::GetGame().GetMainTickCounter() +
                                         strtointdef(args[1], 0));
  }
}

} // namespace

void initclientcommands()
{
  commandadd("bind", commandbind, "Binds command to key", 0);
  commandadd("connect", commandconnect, "connect to server", cmd_deferred);
  commandadd("join", commandconnect, "connect to server", cmd_deferred);
  commandadd("joinurl", commandconnect, "connect to server using url", cmd_deferred);
  commandadd("disconnect", commanddisconnect, "disconnect from server", cmd_deferred);
  commandadd("retry", commandretry, "retry connect to last server", cmd_deferred);
  commandadd("screenshot", commandscreenshot, "take a screenshot of game", cmd_deferred);
  commandadd("say", commandsay, "send chat message", 0);
  commandadd("say_team", commandsayteam, "send team chat message", 0);
  commandadd("record", commandrecord, "record demo", 0);
  commandadd("mute", commandmute, "mute specific nick or id", 0);
  commandadd("unbindall", commandunbindall, "Unbinds all binds", 0);
  commandadd("unmute", commandunmute, "unmute specific nick or id", 0);
  commandadd("stop", commandstop, "stop recording demo", 0);
  commandadd("shutdown", commandshutdown, "shutdown game", 0);
  commandadd("switchcam", commandswitchcam, "switches camera to specific player", 0);
  commandadd("switchcamflag", commandswitchcamflag, "switches camera to specific flag", 0);
  commandadd("demo_tick", commanddemotick, "skips to a tick in demo", 0);
  commandadd("demo_tick_r", commanddemotick, "skips to a tick (relatively) in demo", 0);
}
/*$pop*/
