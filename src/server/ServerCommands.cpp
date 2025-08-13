// automatically converted

#include "ServerCommands.hpp"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "BanSystem.hpp"
#include "Server.hpp"
#include "ServerHelper.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/Logging.hpp"
#include "shared/Command.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/Version.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkServer.hpp"
#include "shared/network/NetworkServerConnection.hpp"
#include "shared/network/NetworkServerFunctions.hpp"
#include "shared/network/NetworkServerGame.hpp"
#include "shared/network/NetworkServerMessages.hpp"
#include "shared/network/NetworkUtils.hpp"
#include "common/PolyMap.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include "common/WeaponSystem.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"
#include "shared/Cvar.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"

using string = std::string;

namespace
{

/*$PUSH*/
/*$WARN 5024 OFF : Parameter "$1" not used*/
void commandaddbot(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;
  std::string tempstr;
  std::int32_t teamset;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  if (GS::GetGame().GetPlayersNum() == max_players)
  {
    return;
  }

  tempstr = args[0];
  teamset = strtointdef({tempstr[6], 1}, 1);
  gGlobalStateServer.addbotplayer(name, teamset);
}

void commandaddbots(std::vector<std::string> &args, std::uint8_t sender)
{
  if (length(args) == 1)
  {
    return;
  }

  if (GS::GetGame().GetPlayersNum() == max_players)
  {
    return;
  }

  auto amount = strtointdef(args[1], 2);
  for (auto i = 0; i < amount; i++)
  {
    gGlobalStateServer.addbotplayer("Terminator", i % 2 + 1);
  }
}

void commandnextmap(std::vector<std::string> &args, std::uint8_t sender)
{
  gGlobalStateServer.nextmap();
}

void commandmap(std::vector<std::string> &args, std::uint8_t sender)
{
  tmapinfo status;

  if (length(args) == 1)
  {
    return;
  }

  if (length(args[1]) < 1)
  {
    return;
  }

  if (getmapinfo(GS::GetFileSystem(), args[1], GS::GetGame().GetUserDirectory(), status))
  {
    gGlobalStateServer.preparemapchange(args[1]);
  }
  else
  {
      GS::GetMainConsole().console(string("Map not found (") + args[1] + ')',
                                     warning_message_color, sender);
  }

  // if not MapExists(MapChangeName, userdirectory) then
  // begin
  //  GS::GetMainConsole().Console('Map not found (' + MapChangeName + ')',
  //  WARNING_MESSAGE_COLOR, Sender); Exit;
  // end;
}

void commandpause(std::vector<std::string> &args, std::uint8_t sender)
{
  GS::GetGame().SetMapchangecounter(999999999);
  serversyncmsg();
}

void commandunpause(std::vector<std::string> &args, std::uint8_t sender)
{
  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
  serversyncmsg();
}

void commandrestart(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &game = GS::GetGame();
  auto &map = game.GetMap();
  game.SetMapchangename(map.name);
  game.SetMapchangecounter(game.GetMapchangetime());
  servermapchange(all_players); // Inform clients of Map Change
#ifdef SCRIPT
  scrptdispatcher.onbeforemapchange(mapchangename);
#endif
}

void commandkick(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  targets = commandtarget(name, sender);
  for (i = 0; i <= high(targets); i++)
  {
    gGlobalStateServer.kickplayer(targets[i], false, kick_console, 0);
  }
}

void commandkicklast(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  if ((gGlobalStateServer.lastplayer > 0) && (gGlobalStateServer.lastplayer < max_sprites + 1))
  {
    if (sprite_system.GetSprite(gGlobalStateServer.lastplayer).IsActive())
    {
      gGlobalStateServer.kickplayer(gGlobalStateServer.lastplayer, false, kick_console, 0);
    }
  }
}

void commandban(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string name;
  std::string tempstr;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  if (sender == 255)
  {
    tempstr = "an admin";
  }
  else
  {
    tempstr = sprite_system.GetSprite(sender).player->name;
  }

  targets = commandtarget(name, sender);
  for (i = 0; i <= high(targets); i++)
  {
    gGlobalStateServer.kickplayer(targets[i], true, kick_console, (day * 30),
                                  std::string("Banned by ") + tempstr);
  }
}

void commandbaniphw(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string name;
  std::string tempstr;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  if (sender == 255)
  {
    tempstr = "an admin";
  }
  else
  {
    tempstr = sprite_system.GetSprite(sender).player->name;
  }

  if (args[0] == "banhw")
  {
    gGlobalStateBanSystem.addbannedhw(name, string("Banned by ") + tempstr, (day * 30));
    GS::GetMainConsole().console(string("HWID ") + name + " banned", client_message_color,
                                   sender);
  }
  else
  {
    gGlobalStateBanSystem.addbannedip(name, string("Banned by ") + tempstr, day * 30);
    GS::GetMainConsole().console(string("IP number ") + name + " banned", client_message_color,
                                   sender);
  }

  savetxtlists();
}

void commandunban(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  if (gGlobalStateBanSystem.delbannedip(name))
  {
    GS::GetMainConsole().console(string("IP number ") + name + " unbanned", client_message_color,
                                 sender);
  }

  if (gGlobalStateBanSystem.delbannedhw(name))
  {
    GS::GetMainConsole().console(string("HWID ") + name + " unbanned", client_message_color,
                                 sender);
  }

  savetxtlists();
}

void commandunbanlast(std::vector<std::string> &args, std::uint8_t sender)
{
  if (gGlobalStateBanSystem.delbannedip(gGlobalStateBanSystem.lastban))
  {
    GS::GetMainConsole().console(string("IP number ") + gGlobalStateBanSystem.lastban + " unbanned",
                                 client_message_color, sender);
  }

  if (gGlobalStateBanSystem.delbannedhw(gGlobalStateBanSystem.lastbanhw))
  {
    GS::GetMainConsole().console(string("HWID ") + gGlobalStateBanSystem.lastbanhw + " unbanned",
                                 client_message_color, sender);
  }

  savetxtlists();
}

void commandadm(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string name;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  targets = commandtarget(name, sender);
  for (i = 0; i <= high(targets); i++)
  {
    if (isremoteadminip(sprite_system.GetSprite(targets[i]).player->ip))
    {
      gGlobalStateServer.remoteips.add(sprite_system.GetSprite(targets[i]).player->ip);
      GS::GetMainConsole().console(string("IP number ") +
                                     sprite_system.GetSprite(targets[i]).player->ip +
                                     " added to Remote Admins",
                                   client_message_color, sender);
      savetxtlists();
    }
  }
}

void commandadmip(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  if (!isremoteadminip(name))
  {
    gGlobalStateServer.remoteips.add(name);
    GS::GetMainConsole().console(string("IP number ") + name + " added to Remote Admins",
                                   client_message_color, sender);
    savetxtlists();
  }
}

void commandunadm(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  if (isremoteadminip(name))
  {
    NotImplemented("network");
#if 0
    std::int32_t j;
        j = remoteips.indexof(name);
        remoteips.delete_(j);
#endif
    GS::GetMainConsole().console(string("IP number ") + name + " removed from Remote Admins",
                                   client_message_color, sender);
    savetxtlists();
  }
}

void commandsetteam(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string name;
  std::string tempstr;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  tempstr = args[0];
  NotImplemented();
  std::uint8_t teamset = 1;
#if 0
    teamset = strtointdef(std::string(tempstr.at(8)), 1);
#endif

  targets = commandtarget(name, sender);
  for (i = 0; i <= high(targets); i++)
  {
    sprite_system.GetSprite(targets[i]).changeteam_ServerVariant(teamset, true);
  }
}

void commandsay(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  serversendstringmessage((name), all_players, 255, msgtype_pub);
}

void commandkill(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string name;
  tvector2 a;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  targets = commandtarget(name, sender);
  for (i = 0; i <= high(targets); i++)
  {
    sprite_system.GetSprite(targets[i]).vest = 0;
    sprite_system.GetSprite(targets[i]).healthhit(3430, targets[i], 1, -1, a);
    GS::GetMainConsole().console(sprite_system.GetSprite(targets[i]).player->name +
                                   " killed by admin",
                                 client_message_color, sender);
  }
}

void commandloadwep(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    if (CVar::sv_realisticmode)
    {
      name = "weapons_floatistic";
    }
    else
    {
      name = "weapons";
    }
  }
  else
  {
    name = args[1];
  }

  gGlobalStateServer.loadweapons(name);

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      servervars(sprite.num);
    }
  }
}

void commandloadcon(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (CVar::sv_lockedmode)
  {
    GS::GetMainConsole().console(
      std::string("Locked Mode is enabled. Settings can't be changed mid-game."),
      server_message_color, sender);
    return;
  }

  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
  serverdisconnect();
  GS::GetBulletSystem().KillAll();
  GS::GetThingSystem().KillAll();
  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    sprite.player->team = fixteam(sprite.player->team);
    sprite.respawn();
    sprite.player->kills = 0;
    sprite.player->deaths = 0;
    sprite.player->flags = 0;

    sprite.player->tkwarnings = 0;
    sprite.player->chatwarnings = 0;
    sprite.player->knifewarnings = 0;

    sprite.player->scorespersecond = 0;
    sprite.player->grabspersecond = 0;
  }

  loadconfig(name, GS::GetFileSystem());
  GS::GetMainConsole().console(string("Config reloaded ") + gGlobalStateServer.currentconf,
                               client_message_color, sender);
  gGlobalStateServer.startserver();
}

void commandloadlist(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 0)
  {
    NotImplemented("missing stringreplace");
#if 0
        name = stringreplace(CVar::sv_maplist, ".txt", "", rfreplaceall);
#endif
  }
  else
  {
    name = args[1];
  }

  if (fileexists(GS::GetGame().GetUserDirectory() + name + ".txt"))
  {
    gGlobalStateServer.mapslist.loadfromfile(GS::GetGame().GetUserDirectory() + name + ".txt");
    gGlobalStateServer.mapslist.erase(
      std::remove(gGlobalStateServer.mapslist.begin(), gGlobalStateServer.mapslist.end(), ""),
      gGlobalStateServer.mapslist.end());
    CVar::sv_maplist = name + ".txt";
    GS::GetMainConsole().console(string("Mapslist loaded ") + name, client_message_color, sender);
  }
}

void commandpm(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string pmtoid;
  std::string pmmessage;
  std::int32_t i;
  tcommandtargets targets;

  if (length(args) <= 2)
  {
    return;
  }

  pmtoid = args[1];

  targets = commandtarget(pmtoid, sender);
  for (i = 0; i <= high(targets); i++)
  {
    pmmessage = args[2];
    GS::GetMainConsole().console(string("Private Message sent to ") + idtoname(targets[i]),
                                   server_message_color, sender);
    GS::GetMainConsole().console(string("(PM) To: ") + idtoname(targets[i]) +
                                   " From: " + idtoname(sender) + " Message: " + pmmessage,
                                 server_message_color);
    serversendstringmessage(string("(PM) ") + (pmmessage), targets[i], 255, msgtype_pub);
  }
}

void commandgmute(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string name;
  std::int32_t i;
  std::int32_t j;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  targets = commandtarget(name, sender);
  for (i = 0; i <= high(targets); i++)
  {
    sprite_system.GetSprite(targets[i]).player->muted = 1;
    for (j = 1; j <= max_players; j++)
    {
      if (trim(gGlobalStateServer.mutelist[j]).empty())
      {
        gGlobalStateServer.mutelist[j] = sprite_system.GetSprite(targets[i]).player->ip;
        gGlobalStateServer.mutename[j] = sprite_system.GetSprite(targets[i]).player->name;
        break;
      }
    }
    GS::GetMainConsole().console(sprite_system.GetSprite(targets[i]).player->name +
                                   " has been muted.",
                                 client_message_color, sender);
  }
}

void commandungmute(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string name;
  std::int32_t i;
  std::int32_t j;
  tcommandtargets targets;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  targets = commandtarget(name, sender);
  for (i = 0; i <= high(targets); i++)
  {
    sprite_system.GetSprite(targets[i]).player->muted = 0;
    for (j = 1; j <= max_players; j++)
    {
      if (trim(gGlobalStateServer.mutelist[j]) == sprite_system.GetSprite(targets[i]).player->ip)
      {
        gGlobalStateServer.mutelist[j] = "";
        break;
      }
    }
    GS::GetMainConsole().console(sprite_system.GetSprite(targets[i]).player->name +
                                   " has been unmuted.",
                                 client_message_color, sender);
  }
}

void commandaddmap(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  // if not (MapExists(Name, userdirectory)) then
  // begin
  //  GS::GetMainConsole().Console('Map not found (' + Name + ')',
  //    SERVER_MESSAGE_COLOR, Sender);
  //  Exit;
  // end;

  gGlobalStateServer.mapslist.add(name);
  GS::GetMainConsole().console(name + " has been added to the map list.", server_message_color,
                                 sender);
  savemaplist();
}

void commanddelmap(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;
  std::int32_t tempint;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  for (tempint = 0; tempint <= (gGlobalStateServer.mapslist.size() - 1); tempint++)
  {
    NotImplemented();
#if 0
        if (uppercase(mapslist[tempint]) == uppercase(name))
        {
            GS::GetMainConsole().console(name + " has been removed from the map list.", server_message_color,
                                sender);
            mapslist.delete_(tempint);
            break;
        }
#endif
  }
  savemaplist();
}

void commandtempban(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;
  std::string tempstr;

  if (length(args) <= 2)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  tempstr = args[2];
  // *BAN*
  gGlobalStateBanSystem.addbannedip(tempstr, "Temporary Ban by an Admin",
                                    strtointdef(name, 1) * minute);
  GS::GetMainConsole().console(string("IP number ") + tempstr + " banned for " + name +
                                   " minutes.",
                                 client_message_color, sender);
  savetxtlists();
}

void commandweaponon(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;
  std::int32_t j;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      j = strtointdef(name, -1);
      if ((j > -1) && (j < 15))
      {
        setweaponactive(sprite.num, j, true);
      }
    }
  }
}

void commandweaponoff(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;
  std::int32_t j;

  if (length(args) == 1)
  {
    return;
  }

  name = args[1];

  if (length(name) < 1)
  {
    return;
  }

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      j = strtointdef(name, -1);
      if ((j > -1) && (j < 15))
      {
        setweaponactive(sprite.num, j, false);
      }
    }
  }
}

void commandbanlist(std::vector<std::string> &args, std::uint8_t sender)
{
  NotImplemented();
#if 0
    std::int32_t i;
    tdatetime banduration;
    std::string bandurationtext;

    GS::GetMainConsole().console(format("%-15s | %-9s | %s", set::of("HWID", "Duration", "Reason", eos)),
                        server_message_color, sender);
    for (i = 1; i <= high(bannedhwlist); i++)
    {
        if (bannedhwlist[i].time == permanent)
            bandurationtext = "PERMANENT";
        else
        {
            banduration = incsecond(now, bannedhwlist[i].time / 60) - now;
            bandurationtext =
                format("%dd%s", set::of(trunc(banduration),
                                        formatdatetime("h\"h\"n\"m\"", banduration), eos));
        }
        GS::GetMainConsole().console(format("%-15s | %-9s | %s", set::of(bannedhwlist[i].hw, bandurationtext,
                                                                bannedhwlist[i].reason, eos)),
                            server_message_color, sender);
    }

    GS::GetMainConsole().console(format("%-15s | %-9s | %s", set::of("IP", "Duration", "Reason", eos)),
                        server_message_color, sender);
    for (i = 1; i <= high(bannediplist); i++)
    {
        if (bannediplist[i].time == permanent)
            bandurationtext = "PERMANENT";
        else
        {
            banduration = incsecond(now, bannediplist[i].time / 60) - now;
            bandurationtext =
                format("%dd%s", set::of(trunc(banduration),
                                        formatdatetime("h\"h\"n\"m\"", banduration), eos));
        }
        GS::GetMainConsole().console(format("%-15s | %-9s | %s", set::of(bannediplist[i].ip, bandurationtext,
                                                                bannediplist[i].reason, eos)),
                            server_message_color, sender);
    }
#endif
}

void commandnetstats(std::vector<std::string> &args, std::uint8_t sender)
{
  {
    auto statstext = gGlobalStateNetworkServer.GetServerNetwork()->GetDetailedConnectionStatus(1);
    LogInfo("network", "{}", statstext);
  }
  auto players = gGlobalStateNetworkServer.GetServerNetwork()->GetPlayers();
  for (auto &dstplayer : players)
  {
    auto statstext =
      gGlobalStateNetworkServer.GetServerNetwork()->GetDetailedConnectionStatus(dstplayer->peer);
    {
      LogInfo("network", "----------------\n{}\n{}\n{}\n----------------",
        dstplayer->name, dstplayer->peer, statstext);
    }
  }
}

void commandplayercommand(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string command;
  tvector2 a;

  command = args[0];

  if (command == "tabac")
  {
    sprite_system.GetSprite(sender).idlerandom = 0;
    sprite_system.GetSprite(sender).idletime = 1;
  }
  else if (command == "smoke")
  {
    sprite_system.GetSprite(sender).idlerandom = 1;
    sprite_system.GetSprite(sender).idletime = 1;
  }
  else if (command == "takeoff")
  {
    sprite_system.GetSprite(sender).idlerandom = 4;
    sprite_system.GetSprite(sender).idletime = 1;
  }
  else if (command == "victory")
  {
    sprite_system.GetSprite(sender).idlerandom = 5;
    sprite_system.GetSprite(sender).idletime = 1;
  }
  else if (command == "piss")
  {
    sprite_system.GetSprite(sender).idlerandom = 6;
    sprite_system.GetSprite(sender).idletime = 1;
  }
  else if (command == "mercy")
  {
    sprite_system.GetSprite(sender).idlerandom = 7;
    sprite_system.GetSprite(sender).idletime = 1;
    if (sprite_system.GetSprite(sender).player->kills > 0)
    {
      sprite_system.GetSprite(sender).player->kills -= 1;
    }
  }
  else if (command == "pwn")
  {
    sprite_system.GetSprite(sender).idlerandom = 8;
    sprite_system.GetSprite(sender).idletime = 1;
  }
  else if (command == "kill")
  {
    sprite_system.GetSprite(sender).vest = 0;
    sprite_system.GetSprite(sender).healthhit(150, sender, 1, -1, a);
    if (sprite_system.GetSprite(sender).player->kills > 0)
    {
      sprite_system.GetSprite(sender).player->kills -= 1;
    }
  }
  else if (command == "brutalkill")
  {
    sprite_system.GetSprite(sender).vest = 0;
    sprite_system.GetSprite(sender).healthhit(3423, sender, 1, -1, a);
    if (sprite_system.GetSprite(sender).player->kills > 0)
    {
      sprite_system.GetSprite(sender).player->kills -= 1;
    }
  }
}

void commandinfo(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string gametype;
#ifdef SCRIPT
  std::int8_t i;
  tstringlist scriptlist;
#endif

  if (length((std::string)CVar::sv_greeting) > 0)
  {
    serversendstringmessage((CVar::sv_greeting), sender, 255, msgtype_pub);
  }
  if (length((std::string)CVar::sv_greeting2) > 0)
  {
    serversendstringmessage((CVar::sv_greeting2), sender, 255, msgtype_pub);
  }
  if (length((std::string)CVar::sv_greeting3) > 0)
  {
    serversendstringmessage((CVar::sv_greeting3), sender, 255, msgtype_pub);
  }

  serversendstringmessage(string("Server: ") + string(CVar::sv_hostname), sender, 255, msgtype_pub);
  serversendstringmessage(string("Address: ") + (gGlobalStateServer.serverip) + ':' +
                            (inttostr(gGlobalStateServer.serverport)),
                          sender, 255, msgtype_pub);
  serversendstringmessage(string("Version: ") + dedversion, sender, 255, msgtype_pub);

  switch (CVar::sv_gamemode)
  {
  case gamestyle_deathmatch:
    gametype = "Deathmatch";
    break;
  case gamestyle_pointmatch:
    gametype = "Pointmatch";
    break;
  case gamestyle_teammatch:
    gametype = "Teammatch";
    break;
  case gamestyle_ctf:
    gametype = "Capture the Flag";
    break;
  case gamestyle_rambo:
    gametype = "Rambomatch";
    break;
  case gamestyle_inf:
    gametype = "Infiltration";
    break;
  case gamestyle_htf:
    gametype = "Hold the Flag";
    break;
  }

  serversendstringmessage(string("Gamemode: ") + gametype, sender, 255, msgtype_pub);
  serversendstringmessage(string("Timelimit: ") + (inttostr(CVar::sv_timelimit / 3600)), sender,
                          255, msgtype_pub);
  serversendstringmessage(string("Nextmap: ") + (checknextmap()), sender, 255, msgtype_pub);

#ifdef SCRIPT
  serversendstringmessage(string("Scripting: ") + iif(CVar::sc_enable, "Enabled", "Disabled"),
                          sender, 255, msgtype_pub);
  if (CVar::sc_enable)
  {
    gametype = "";
    scriptlist = scrptdispatcher.scriptlist;
    for (i = 0; i <= scriptlist.count - 1; i++)
      gametype = gametype + iif(i != 0, ", ", "") + scriptlist.strings[i];
    serversendstringmessage(string("Scripts: ") + gametype, sender, 255, msgtype_pub);
    scriptlist.free(0);
  }
#endif

  if (GS::GetWeaponSystem().GetLoadedWMChecksum() != GS::GetWeaponSystem().GetDefaultWMChecksum())
  {
    serversendstringmessage(string("Server uses weapon mod \"") + (gGlobalStateServer.wmname) +
                              " v" + (gGlobalStateServer.wmversion) + "\" (checksum " +
                              (inttostr(GS::GetWeaponSystem().GetLoadedWMChecksum())) + ')',
                            sender, 255, msgtype_pub);
  }
}

void commandadminlog(std::vector<std::string> &args, std::uint8_t sender)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string adminlog;

  if (length(args) == 1)
  {
    return;
  }

  adminlog = args[1];
  if (length((std::string)CVar::sv_adminpassword) > 0)
  {
    if (adminlog == CVar::sv_adminpassword)
    {
      if (!isadminip(sprite_system.GetSprite(sender).player->ip))
      {
        gGlobalStateServer.adminips.add(sprite_system.GetSprite(sender).player->ip);
      }
      GS::GetMainConsole().console(sprite_system.GetSprite(sender).player->name +
                                     " added to Game Admins",
                                   server_message_color, sender);
    }
    else
    {
      GS::GetMainConsole().console(sprite_system.GetSprite(sender).player->name +
                                     " tried to login as Game Admin with bad password",
                                   server_message_color, sender);
    }
  }
}

void commandvotemap(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string mapname;

  if (length(args) == 1)
  {
    return;
  }

  mapname = args[1];

  if (GS::GetGame().IsVoteActive())
  {
    // check if the vote target is actually the target
    if (GS::GetGame().GetVoteTarget() != string(mapname))
    {
      return;
    }

    // check if he already voted
    if (GS::GetGame().HasVoted(sender))
    {
      return;
    }

#ifdef SCRIPT
    scrptdispatcher.onvotemap(sender, mapname);
#endif
    GS::GetGame().countvote(sender);
  }
  else
  {
    if (std::find(gGlobalStateServer.mapslist.begin(), gGlobalStateServer.mapslist.end(),
                  mapname) !=
        gGlobalStateServer.mapslist.end()) /*and (MapExists(MapName, userdirectory))*/
    {
      if (GS::GetGame().CanVote(sender))
      {
        if (!GS::GetGame().IsVoteActive())
        {
#ifdef SCRIPT
          if (scrptdispatcher.onvotemapstart(sender, mapname))
            return;
#endif
          GS::GetGame().startvote(sender, vote_map, mapname, "---");
          serversendvoteon(GS::GetGame().GetVoteType(), sender, mapname, "---");
        }
      }
      else
      {
        serversendstringmessage("Can't vote for 2:00 minutes after joining game or last vote",
                                sender, 255, msgtype_pub);
      }
    }
    else
    {
      serversendstringmessage(string("Map not found (") + (mapname) + ')', sender, 255,
                              msgtype_pub);
    }
  }
}

#ifdef SCRIPT
void commandrecompile(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string name;

  if (length(args) == 1)
  {
    if (!CVar::sc_enable)
      GS::GetMainConsole().console("Scripting is currently disabled.", client_message_color,
                                     sender);
    else
    {
      scrptdispatcher.prepare(0);
      scrptdispatcher.launch(0);
    }
  }
  else
  {
    name = args[1];
    if (!CVar::sc_enable)
      GS::GetMainConsole().console("Scripting is currently disabled.", client_message_color,
                                     sender);
    else
      scrptdispatcher.launch(name);
  }
}
#endif

void commandrecord(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string str1;

  if (length(args) == 2)
  {
    str1 = args[1];
  }
  else
  {
    NotImplemented();
#if 0
        str1 = formatdatetime("yyyy-mm-dd_hh-nn-ss_", now(0)) + map.name;
#endif
  }

  GS::GetDemoRecorder().stoprecord();
  GS::GetDemoRecorder().startrecord(GS::GetGame().GetUserDirectory() + "demos/" + str1 + ".sdm");
}

void commandstop(std::vector<std::string> &args, std::uint8_t sender)
{
  GS::GetDemoRecorder().stoprecord();
}

} // namespace

/*$POP*/

void initservercommands()
{
  commandadd("addbot", commandaddbot, "Add specific bot to game", cmd_adminonly);
  commandadd("addbot1", commandaddbot, "Add specific bot to alpha team", cmd_adminonly);
  commandadd("addbot2", commandaddbot, "Add specific bot to blue team", cmd_adminonly);
  commandadd("addbot3", commandaddbot, "Add specific bot to charlie team", cmd_adminonly);
  commandadd("addbot4", commandaddbot, "Add specific bot to delta team", cmd_adminonly);
  commandadd("addbot5", commandaddbot, "Add specific bot to spectators", cmd_adminonly);
  commandadd("addbots", commandaddbots, "Add multiple bots for test", cmd_adminonly);
  commandadd("nextmap", commandnextmap, "Change map to next in maplist", cmd_adminonly);
  commandadd("map", commandmap, "Change map to specified mapname", cmd_adminonly);
  commandadd("pause", commandpause, "Pause game", cmd_adminonly);
  commandadd("unpause", commandunpause, "Unpause game", cmd_adminonly);
  commandadd("restart", commandrestart, "Restart current map", cmd_adminonly);
  commandadd("kick", commandkick, "Kick player with specified nick or id", cmd_adminonly);
  commandadd("kicklast", commandkicklast, "Kick last connected player", cmd_adminonly);
  commandadd("ban", commandban, "Ban player with specified nick or id", cmd_adminonly);
  commandadd("banip", commandbaniphw, "Ban specified IP address", cmd_adminonly);
  commandadd("banhw", commandbaniphw, "Ban specified hwid", cmd_adminonly);
  commandadd("unban", commandunban, "Unban specified ip or hwid", cmd_adminonly);
  commandadd("unbanlast", commandunban, "Unban last player", cmd_adminonly);
  commandadd("adm", commandadm, "Give admin to specified nick or id", cmd_adminonly);
  commandadd("admip", commandadmip, "add the IP number to the Remote Admins list", cmd_adminonly);
  commandadd("unadm", commandunadm, "remove the IP number from the admins list", cmd_adminonly);
  commandadd("setteam1", commandsetteam, "move specified id or nick to alpha team", cmd_adminonly);
  commandadd("setteam2", commandsetteam, "move specified id or nick to bravo team", cmd_adminonly);
  commandadd("setteam3", commandsetteam, "move specified id or nick to charlie team",
             cmd_adminonly);
  commandadd("setteam4", commandsetteam, "move specified id or nick to delta team", cmd_adminonly);
  commandadd("setteam5", commandsetteam, "move specified id or nick to spectators", cmd_adminonly);
  commandadd("say", commandsay, "Send chat message", cmd_adminonly);
  commandadd("pkill", commandkill, "Kill specified id or nick", cmd_adminonly);
  commandadd("loadwep", commandloadwep, "Load weapons config", cmd_adminonly);
  commandadd("loadcon", commandloadcon, "Load server config", cmd_adminonly);
  commandadd("loadlist", commandloadlist, "Load maplist", cmd_adminonly);
  commandadd("pm", commandpm, "Send private message to other player", cmd_adminonly);
  commandadd("gmute", commandgmute, "Mute player on server", cmd_adminonly);
  commandadd("ungmute", commandungmute, "Unmute player on server", cmd_adminonly);
  commandadd("addmap", commandaddmap, "Add map to the maplist", cmd_adminonly);
  commandadd("delmap", commanddelmap, "Remove map from the maplist", cmd_adminonly);
  commandadd("weaponon", commandweaponon, "Enable specific weapon on the server", cmd_adminonly);
  commandadd("weaponoff", commandweaponoff, "Disable specific weapon on the server", cmd_adminonly);
  commandadd("banlist", commandbanlist, "Show banlist", cmd_adminonly);
  commandadd("net_stats", commandnetstats, "Show network stats", cmd_adminonly);

  commandadd("tabac", commandplayercommand, "tabac", cmd_playeronly);
  commandadd("smoke", commandplayercommand, "smoke", cmd_playeronly);
  commandadd("takeoff", commandplayercommand, "takeoff", cmd_playeronly);
  commandadd("victory", commandplayercommand, "victory", cmd_playeronly);
  commandadd("piss", commandplayercommand, "piss", cmd_playeronly);
  commandadd("mercy", commandplayercommand, "mercy", cmd_playeronly);
  commandadd("pwn", commandplayercommand, "pwn", cmd_playeronly);
  commandadd("kill", commandplayercommand, "kill", cmd_playeronly);
  commandadd("brutalkill", commandplayercommand, "brutalkill", cmd_playeronly);
  commandadd("info", commandinfo, "info", cmd_playeronly);
  commandadd("adminlog", commandadminlog, "adminlog", cmd_playeronly);
  commandadd("votemap", commandvotemap, "votemap", cmd_playeronly);
  commandadd("record", commandrecord, "record demo", cmd_adminonly);
  commandadd("stop", commandstop, "stop recording demo", cmd_adminonly);

#ifdef SCRIPT
  commandadd("recompile", commandrecompile(), "Recompile all or specific script", cmd_adminonly);
#endif
}
