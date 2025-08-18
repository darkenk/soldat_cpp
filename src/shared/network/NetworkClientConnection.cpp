// automatically converted

#include "NetworkClientConnection.hpp"

#include <physfs.h>
#include <alloca.h>
#include <endian.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>

#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/GameRendering.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../misc/BitStream.hpp"
#include "NetworkClient.hpp"
#include "NetworkClientSprite.hpp"
#include "NetworkUtils.hpp"
#include "common/Console.hpp"
#include "common/GameStrings.hpp"
#include "common/LogFile.hpp"
#include "common/Logging.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "shared/Version.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/Constants.hpp"
#include "common/PolyMap.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/RandomGenerator.hpp"
#include "common/misc/SHA1Helper.hpp"
#include "common/misc/SafeType.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/SourceLocation.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/AnimationSystem.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"

using string = std::string;

static void sPreprocessSprites(SpriteSystem &spriteSystem, tmsg_playerslist *playerslistmsg,
                               tvector2 &pos, tvector2 &vel)
{
  for (std::uint8_t i = 0; i < max_sprites; i++)
  {
    if (strcmp(playerslistmsg->name[i].data(), "0 ") == 0)
    {
      continue;
    }
    SpriteId id{static_cast<std::uint8_t>(i + 1)};
    auto newplayer = spriteSystem.GetSprite(id).player; // reuse object
    newplayer->name = returnfixedplayername(playerslistmsg->name[i].data());
    newplayer->shirtcolor = playerslistmsg->shirtcolor[i] | 0xff000000;
    newplayer->pantscolor = playerslistmsg->pantscolor[i] | 0xff000000;
    newplayer->skincolor = playerslistmsg->skincolor[i] | 0xff000000;
    newplayer->haircolor = playerslistmsg->haircolor[i] | 0xff000000;
    newplayer->jetcolor = playerslistmsg->jetcolor[i];
    newplayer->team = playerslistmsg->team[i];

    newplayer->secwep = 0;
    pos = playerslistmsg->pos[i];
    vel = playerslistmsg->vel[i];

    newplayer->hairstyle = 0;
    if ((playerslistmsg->look[i] & B1) == B1)
    {
      newplayer->hairstyle = 1;
    }
    if ((playerslistmsg->look[i] & B2) == B2)
    {
      newplayer->hairstyle = 2;
    }
    if ((playerslistmsg->look[i] & B3) == B3)
    {
      newplayer->hairstyle = 3;
    }
    if ((playerslistmsg->look[i] & B4) == B4)
    {
      newplayer->hairstyle = 4;
    }

    newplayer->headcap = 0;
    if ((playerslistmsg->look[i] & B5) == B5)
    {
      newplayer->headcap = GFX::GOSTEK_HELM;
    }
    if ((playerslistmsg->look[i] & B6) == B6)
    {
      newplayer->headcap = GFX::GOSTEK_KAP;
    }

    newplayer->chain = 0;
    if ((playerslistmsg->look[i] & B7) == B7)
    {
      newplayer->chain = 1;
    }
    if ((playerslistmsg->look[i] & B8) == B8)
    {
      newplayer->chain = 2;
    }

    createsprite(pos, id, newplayer);
    auto &sprite = spriteSystem.GetSprite(id);

    auto &spriteVelocity = spriteSystem.GetVelocity(sprite.num);
    spriteVelocity = vel;

    sprite.ceasefirecounter = 0;
    if (playerslistmsg->predduration[i] > 0)
    {
      sprite.alpha = predatoralpha;
      sprite.bonustime = playerslistmsg->predduration[i] * 60;
      sprite.bonusstyle = bonus_predator;
    }
  }
}

// REQUEST GAME FROM SERVER
void clientrequestgame(INetwork &network, std::string_view password)
{
  const std::int32_t size = tmsg_requestgame::sCalculateSize(password);
  auto *buff = static_cast<uint8_t *>(alloca(size));

  auto *requestmsg = new (buff) tmsg_requestgame(password);
  SoldatAssert(requestmsg->GetSize() == size);

  std::strcpy(requestmsg->version.data(), soldat_version);
  requestmsg->haveanticheat = actype_none;

#ifdef ENABLE_FAE
  if (faeisenabled)
    requestmsg.haveanticheat = actype_fae;
#endif

  if (!gGlobalStateClient.redirectip.empty())
  {
    requestmsg->forwarded = 1;
    gGlobalStateClient.redirectip = "";
    gGlobalStateClient.redirectport = 0;
    gGlobalStateClient.redirectmsg = "";
  }
  std::strcpy(requestmsg->hardwareid.data(), gGlobalStateClient.hwid.data());
  network.SendData(*requestmsg);

  gGlobalStateNetworkClient.requestinggame = true;
}

// SEND INFO ABOUT NAME, COLOR, PASS etc. TO SERVER OR CHANGE TEAM
void clientsendplayerinfo()
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_playerinfo playerinfo;
  tmsg_changeteam changemsg;

  if ((gGlobalStateClient.spectator == 1) && (gGlobalStateClient.selteam == 0))
  {
    gGlobalStateClient.selteam = team_spectator;
  }
  gGlobalStateClient.spectator = 0; // allow joining other teams after first join
  if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
      (CVar::sv_gamemode == gamestyle_htf))
  {
    if ((gGlobalStateClient.selteam < team_alpha) ||
        ((gGlobalStateClient.selteam > team_bravo) &&
         (gGlobalStateClient.selteam < team_spectator)))
    {
      gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu);
      return;
    }
  }
  if (CVar::sv_gamemode == gamestyle_teammatch)
  {
    if (gGlobalStateClient.selteam < team_alpha)
    {
      gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu);
      return;
    }
  }

  // sent a team change request instead if we're already ingame
  if (sprite_system.IsPlayerSpriteValid())
  {
    changemsg.header.id = msgid_changeteam;
    changemsg.team = gGlobalStateClient.selteam;
    gGlobalStateNetworkClient.GetNetwork()->SendData(&changemsg, sizeof(changemsg), true);
    return;
  }

  playerinfo.header.id = msgid_playerinfo;
  stringtoarray(playerinfo.name.data(), CVar::cl_player_name);
  playerinfo.shirtcolor = (255 << 24) + CVar::cl_player_shirt;
  playerinfo.pantscolor = (255 << 24) + CVar::cl_player_pants;
  playerinfo.skincolor = (255 << 24) + CVar::cl_player_skin;
  playerinfo.haircolor = (255 << 24) + CVar::cl_player_hair;
  playerinfo.jetcolor = (CVar::cl_player_jet & 0xffffff) + color_transparency_registered;
  //    {$IFDEF DEVELOPMENT}
  //    COLOR_TRANSPARENCY_SPECIAL
  //    {$ELSE}
  //    COLOR_TRANSPARENCY_REGISTERED
  //    {$ENDIF};
  playerinfo.team = gGlobalStateClient.selteam;
  playerinfo.look = 0;
  if (CVar::cl_player_hairstyle == 1)
  {
    playerinfo.look = playerinfo.look | B1;
  }
  if (CVar::cl_player_hairstyle == 2)
  {
    playerinfo.look = playerinfo.look | B2;
  }
  if (CVar::cl_player_hairstyle == 3)
  {
    playerinfo.look = playerinfo.look | B3;
  }
  if (CVar::cl_player_hairstyle == 4)
  {
    playerinfo.look = playerinfo.look | B4;
  }
  if (CVar::cl_player_headstyle == headstyle_helmet)
  {
    playerinfo.look = playerinfo.look | B5;
  }
  if (CVar::cl_player_headstyle == headstyle_hat)
  {
    playerinfo.look = playerinfo.look | B6;
  }
  if (CVar::cl_player_chainstyle == 1)
  {
    playerinfo.look = playerinfo.look | B7;
  }
  if (CVar::cl_player_chainstyle == 2)
  {
    playerinfo.look = playerinfo.look | B8;
  }

  playerinfo.gamemodchecksum.Dummy[0] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[0]);
  playerinfo.gamemodchecksum.Dummy[1] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[1]);
  playerinfo.gamemodchecksum.Dummy[2] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[2]);
  playerinfo.gamemodchecksum.Dummy[3] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[3]);
  playerinfo.gamemodchecksum.Dummy[4] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[4]);
  playerinfo.custommodchecksum = GS::GetGame().GetCustomModChecksum();

  gGlobalStateNetworkClient.GetNetwork()->SendData(&playerinfo, sizeof(playerinfo), true);
  gGlobalStateNetworkClient.clientplayersent = true;
  gGlobalStateNetworkClient.clientplayerreceivedcounter = clientplayerrecieved_time;
}

void clientdisconnect(INetwork &client)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_playerdisconnect playermsg;

  if (sprite_system.IsPlayerSpriteValid())
  { // send disconnection info to server
    playermsg.header.id = msgid_playerdisconnect;
    playermsg.num = gGlobalStateClient.mysprite;

    client.SendData(&playermsg, sizeof(playermsg), true);

    GS::GetConsoleLogFile().Log(string("Client Disconnect from ") +
                                gGlobalStateNetworkClient.GetNetwork()->GetStringAddress(true));
    client.ProcessLoop();
    client.Disconnect(false);
  }
  else
  {
    client.Disconnect(true);
    gGlobalStateClient.exittomenu();
  }
}

void ClientPongMsg::send(const std::uint8_t pingnum)
{
  tmsg_pong pongmsg(pingnum);
  mNetwork.SendData(pongmsg);
}

void clientpong(INetwork& network, const std::uint8_t pingnum)
{
  tmsg_pong pongmsg(pingnum);
  network.SendData(pongmsg);
}

void clienthandleplayerslist(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_playerslist *playerslistmsg;
  tvector2 pos;
  tvector2 vel;
  std::string downloadurl;
  bool forcegraphicsreload = false;
  std::string modname;
  std::string mapname;

  if (!verifypacket(sizeof(*playerslistmsg), netmessage->size, msgid_playerslist))
  {
    return;
  }

  if (!(gGlobalStateNetworkClient.requestinggame || gGlobalStateDemo.demoplayer.active()))
  {
    return;
  }

  gGlobalStateNetworkClient.requestinggame = false;

  playerslistmsg = reinterpret_cast<pmsg_playerslist>(netmessage->packet);

  if (!gGlobalStateDemo.demoplayer.active())
  {
    GS::GetMainConsole().console(_("Connection accepted to") + ' ' +
                                   (gGlobalStateNetworkClient.GetNetwork()->GetStringAddress(true)),
                                 client_message_color);
  }

  gGlobalStateClient.gClientServerIP =
    gGlobalStateNetworkClient.GetNetwork()->GetStringAddress(false);
  NotImplemented("network", "no sha1 checks");
#if 1
  mapname = trim(playerslistmsg->mapname.data());
  modname = trim(playerslistmsg->modname.data());
  NotImplemented("network", "string replace");
#if 0
    mapname = ansireplacestr(trim(playerslistmsg->mapname.data()))., "..", "");
    modname = ansireplacestr(trim(playerslistmsg->modname.data()), "..", "");
#endif
  NotImplemented("network", "download url");
#if 0
    if (CVar::sv_downloadurl != "")
        downloadurl = includetrailingpathdelimiter(CVar::sv_downloadurl);
    else
        downloadurl = string("http://") + gClientServerIP + ':' + inttostr(gClientServerPort + 10) + '/';
#endif

  if (CVar::cl_servermods)
  {
    if (!modname.empty())
    {
      if (const tsha1digest checksum =
            sha1file(GS::GetGame().GetUserDirectory() + "mods/" + modname + ".smod");
          playerslistmsg->modchecksum == checksum)
      {
        if (PHYSFS_mount((pchar)(GS::GetGame().GetUserDirectory() + "mods/" + modname + ".smod"),
                         (pchar)(string("mods/") + modname + '/'), 0) == 0)
        {
          gGlobalStateClient.showmessage(
            _(string("Could not load mod archive (") + modname + ")."));
          return;
        }
        gGlobalStateClient.moddir = string("/mods/") + modname + '/';
        GS::GetGame().SetCustomModChecksum(checksum);
        AnimationSystem::Get().LoadAnimObjects(gGlobalStateClient.moddir);
        gGlobalStateSound.loadsounds(gGlobalStateClient.moddir);
        forcegraphicsreload = true;
        gGlobalStateClient.usesservermod = true;
        GS::GetMainConsole().console(_(string("Loading server mod: ") + modname),
                                     mode_message_color);
      }
      else
      {
        NotImplemented("network", "no download thread");
#if 0
                downloadthread = tdownloadthread.create(downloadurl + "mods/" + modname + ".smod",
                                                        GS::GetGame().GetUserDirectory() + "mods/" + modname + ".smod",
                                                        tsha1digest(playerslistmsg->modchecksum));
                return;
#endif
      }
    }
    else
    {
      if (gGlobalStateClient.usesservermod) // reset to original mod
      {
        gGlobalStateClient.moddir = CVar::fs_mod;
        AnimationSystem::Get().LoadAnimObjects(gGlobalStateClient.moddir);
        gGlobalStateSound.loadsounds(gGlobalStateClient.moddir);
        forcegraphicsreload = true;
        gGlobalStateClient.usesservermod = false;
      }
    }
  }

  // Initialize Map
  auto &map = GS::GetGame().GetMap();
  auto &fs = GS::GetFileSystem();
  if (tmapinfo mapstatus ;getmapinfo(fs, mapname, GS::GetGame().GetUserDirectory(), mapstatus) /*&&
        verifymapchecksum(fs, mapstatus, playerslistmsg->mapchecksum)*/)
  {
    if (!map.loadmap(fs, mapstatus, CVar::r_forcebg, CVar::r_forcebg_color1,
                     CVar::r_forcebg_color2))
    {
      gGlobalStateGameRendering.rendergameinfo(_("Could not load map: ") + (mapname));
      gGlobalStateClient.exittomenu();
      return;
    }
  }
  else
  {
    {
      NotImplemented("network", "no download thread");
#if 0
            downloadthread = tdownloadthread.create(downloadurl + "maps/" + mapname + ".smap",
                                                    GS::GetGame().GetUserDirectory() + "maps/" + mapname + ".smap",
                                                    playerslistmsg->mapchecksum);
#endif
      return;
    }
  }
  NotImplemented("network", "no download thread");
#if 0
    downloadretry = 0;
#endif

  if (forcegraphicsreload)
  {
    gGlobalStateGameRendering.reloadgraphics();
    forcegraphicsreload = false;
  }

  // Sync cvars
  GS::GetGame().SetPlayersNum(playerslistmsg->players);
  GS::GetGame().SetTimelimitcounter(playerslistmsg->currenttime);

  sPreprocessSprites(sprite_system, playerslistmsg, pos, vel);

  if (!gGlobalStateDemo.demoplayer.active())
  {
    gGlobalStateGameRendering.rendergameinfo(_("Waiting to join game..."));
  }

  gGlobalStateClient.mysprite = 0;
  gGlobalStateClient.camerafollowsprite = 0;
  gGlobalStateClient.gamethingtarget = 0;
  gGlobalStateClient.selteam = 0;
  gGlobalStateClientGame.menutimer = 0;
  GS::GetGame().SetSurvivalendround(false);
  gGlobalStateClient.camerax = 0;
  gGlobalStateClient.cameray = 0;

  if (!gGlobalStateDemo.demoplayer.active())
  {
    GS::GetGame().ResetGoalTicks();
    gGlobalStateClient.notexts = 0;
  }

  gGlobalStateNetworkClient.clientvarsrecieved = false;
  GS::GetGame().ResetMainTickCounter();
  gGlobalStateNetworkClient.clienttickcount = playerslistmsg->serverticks;
  gGlobalStateNetworkClient.noheartbeattime = 0;
  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
  gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.escmenu, false);
  gGlobalStateClient.limbolock = false;

  if (GS::GetGame().IsVoteActive())
  {
    GS::GetGame().stopvote();
  }

  gGlobalStateClientGame.resetweaponstats();

  gGlobalStateNetworkClient.clientplayerreceived = false;
  gGlobalStateNetworkClient.clientplayersent = false;
  gGlobalStateNetworkClient.clientplayerreceivedcounter = clientplayerrecieved_time;

  // Begin rendering so that the team menu selection is visible
  if (!gGlobalStateDemo.demoplayer.active() || (gGlobalStateDemo.demoplayer.skipto() != -1))
  {
    gGlobalStateClientGame.shouldrenderframes = true;
  }

  if (CVar::cl_player_team > 0)
  {
    // Bypass Team Select Menu if team cvar is set
    gGlobalStateClient.selteam = CVar::cl_player_team;
    clientsendplayerinfo();
  }
  else if (gGlobalStateClient.spectator == 1)
  {
    clientsendplayerinfo();
  }
  else
  {
    if ((CVar::sv_gamemode == gamestyle_deathmatch) ||
        (CVar::sv_gamemode == gamestyle_pointmatch) || (CVar::sv_gamemode == gamestyle_rambo))
    {
      clientsendplayerinfo();
    }

    if (CVar::sv_gamemode == gamestyle_teammatch)
    {
      gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu);
    }

    if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
        (CVar::sv_gamemode == gamestyle_htf))
    {
      gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu);
      if (CVar::sv_balanceteams)
      {
        gGlobalStateClient.selteam = 0;
        if (GS::GetGame().GetPlayersTeamNum(1) < GS::GetGame().GetPlayersTeamNum(2))
        {
          gGlobalStateClient.selteam = 1;
        }
        if (GS::GetGame().GetPlayersTeamNum(2) < GS::GetGame().GetPlayersTeamNum(1))
        {
          gGlobalStateClient.selteam = 2;
        }
        if (gGlobalStateClient.selteam > 0)
        {
          clientsendplayerinfo();
        }
      }
    }
  }

  GS::GetGame().SetStarthealth(default_health);
  if (CVar::sv_realisticmode)
  {
    GS::GetGame().SetStarthealth(floatistic_health);
    GS::GetMainConsole().console(_("Realistic Mode ON"), mode_message_color);
  }
  if (CVar::sv_survivalmode)
  {
    GS::GetMainConsole().console(_("Survival Mode ON"), mode_message_color);
  }
  if (CVar::sv_advancemode)
  {
    GS::GetMainConsole().console(_("Advance Mode ON"), mode_message_color);
  }

  // newby stuff
  if (CVar::cl_runs < 3)
  {
    if (Random(3) == 0)
    {
      GS::GetMainConsole().console(
        _("(!) Jet fuel is map specific. There can be more or less on certain maps."),
        info_message_color);
    }
    else
    {
      GS::GetMainConsole().console(_("(!) To leave your current weapon after respawn"),
                                   info_message_color);
      GS::GetMainConsole().console(string("    ") + _("click anywhere outside the weapons menu."),
                                   info_message_color);
    }

    if (CVar::sv_realisticmode)
    {
      GS::GetMainConsole().console(
        _("(!) To prevent weapon recoil fire float shots or short bursts."), info_message_color);
    }
  }

  gGlobalStateClientGame.mx = gGlobalStateGame.gamewidthhalf;
  gGlobalStateClientGame.my = gGlobalStateGame.gameheighthalf;
  gGlobalStateClientGame.mouseprev.x = gGlobalStateClientGame.mx;
  gGlobalStateClientGame.mouseprev.y = gGlobalStateClientGame.my;
#endif
}

void clienthandleunaccepted(NetworkContext *netmessage)
{
  pmsg_unaccepted unacceptedmsg;
  std::string text;

  if (!verifypacketlargerorequal(sizeof(unacceptedmsg), netmessage->size, msgid_unaccepted))
  {
    return;
  }

  unacceptedmsg = reinterpret_cast<pmsg_unaccepted>(netmessage->packet);
  std::int32_t textlen = netmessage->size - sizeof(tmsg_unaccepted);

  if ((textlen > 0) && (unacceptedmsg->text[textlen - 1] == '\0'))
  {
    text = unacceptedmsg->text.data();
  }
  else
  {
    text = "";
  }

  GS::GetConsoleLogFile().Log(string("*UA ") + inttostr(unacceptedmsg->state));

  switch (unacceptedmsg->state)
  {
  case wrong_version:
    NotImplemented("network", "No soldat version");
#if 0
        rendergameinfo(_("Wrong game versions. Your version:") + ' ' + soldat_version + ' ' +
                       _("Server Version:") + ' ' + unacceptedmsg->version);
#endif
    break;

  case wrong_password:
    gGlobalStateGameRendering.rendergameinfo(_("Wrong server password"));
    break;

  case banned_ip:
    gGlobalStateGameRendering.rendergameinfo(_("You have been banned on this server. Reason:") +
                                             ' ' + text);
    break;

  case server_full:
    gGlobalStateGameRendering.rendergameinfo(_("Server is full"));
    break;

  case invalid_handshake:
    gGlobalStateGameRendering.rendergameinfo(_("Unspecified internal protocol error"));
    break;

  case wrong_checksum:
    gGlobalStateGameRendering.rendergameinfo(_("This server requires a different smod file."));
    break;

  case anticheat_rejected:
    gGlobalStateGameRendering.rendergameinfo(_("Rejected by Anti-Cheat:") + ' ' + text);
    break;
  }

  clientdisconnect(*gGlobalStateNetworkClient.GetNetwork());
  gGlobalStateClient.exittomenu();
}

void clienthandleserverdisconnect(NetworkContext *netmessage)
{
  if (!verifypacket(sizeof(tmsg_serverdisconnect), netmessage->size, msgid_serverdisconnect))
  {
    return;
  }

  GS::GetGame().showmapchangescoreboard("");

  if (!gGlobalStateDemo.demoplayer.active())
  {
    GS::GetMainConsole().console(_("Server disconnected"), server_message_color);
  }
  else
  {
    gGlobalStateDemo.demoplayer.stopdemo();
  }
}

void clienthandleping(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  if (!verifypacket(sizeof(tmsg_ping), netmessage->size, msgid_ping))
  {
    return;
  }

  if (gGlobalStateDemo.demoplayer.active())
  {
    return;
  }

  if (sprite_system.IsPlayerSpriteValid())
  {
    auto &player = sprite_system.GetPlayerSprite().player;
    player->pingticks = pmsg_ping(netmessage->packet)->pingticks;
    player->pingtime = player->pingticks * 1000 / 60;
  }

  clientpong(netmessage->networkClient, pmsg_ping(netmessage->packet)->pingnum);

  gGlobalStateClientGame.clientstopmovingcounter = clientstopmove_retrys;
  gGlobalStateNetworkClient.noheartbeattime = 0;
}

void clienthandleservervars(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  if (!verifypacket(sizeof(tmsg_servervars), netmessage->size, msgid_servervars))
  {
    return;
  }

  const auto *varsmsg = pmsg_servervars(netmessage->packet);

  gGlobalStateNetworkClient.clientvarsrecieved = true;

  auto &weaponSystem = GS::GetWeaponSystem();

  for (std::int32_t i = 1; i <= main_weapons; i++)
  {
    weaponSystem.EnableWeapon(i, varsmsg->weaponactive[i - 1] == 1);
    gGlobalStateGameMenus.limbomenu->button[i - 1].active = weaponSystem.IsEnabled(i);
  }

  if (sprite_system.IsPlayerSpriteValid())
  {
    selectdefaultweapons(gGlobalStateClient.mysprite);
    newplayerweapon();
  }

  createdefaultweapons(CVar::sv_realisticmode, GS::GetWeaponSystem().GetGuns(),
                       GS::GetWeaponSystem().GetDefaultGuns());
  GS::GetWeaponSystem().SetDefaultWMChecksum(createwmchecksum(GS::GetWeaponSystem().GetGuns()));

  for (std::int32_t weaponindex = 0; weaponindex < original_weapons; weaponindex++)
  {
    auto &gun = GS::GetWeaponSystem().GetGuns()[weaponindex + 1];
    gun.hitmultiply = varsmsg->damage[weaponindex];
    gun.ammo = varsmsg->ammo[weaponindex];
    gun.reloadtime = varsmsg->reloadtime[weaponindex];
    gun.speed = varsmsg->speed[weaponindex];
    gun.bulletstyle = varsmsg->bulletstyle[weaponindex];
    gun.startuptime = varsmsg->startuptime[weaponindex];
    gun.bink = varsmsg->bink[weaponindex];
    gun.fireinterval = varsmsg->fireinterval[weaponindex];
    gun.movementacc = varsmsg->movementacc[weaponindex];
    gun.bulletspread = varsmsg->bulletspread[weaponindex];
    gun.recoil = varsmsg->recoil[weaponindex];
    gun.push = varsmsg->push[weaponindex];
    gun.inheritedvelocity = varsmsg->inheritedvelocity[weaponindex];
    gun.modifierhead = varsmsg->modifierhead[weaponindex];
    gun.modifierchest = varsmsg->modifierchest[weaponindex];
    gun.modifierlegs = varsmsg->modifierlegs[weaponindex];
    gun.nocollision = varsmsg->nocollision[weaponindex];
  }

  buildweapons(GS::GetWeaponSystem().GetGuns());

  if (sprite_system.IsPlayerSpriteValid())
  {
    sprite_system.GetPlayerSprite().applyweaponbynum(sprite_system.GetPlayerSprite().weapon.num, 1);
    sprite_system.GetPlayerSprite().applyweaponbynum(
      sprite_system.GetPlayerSprite().secondaryweapon.num, 2);
    if (!sprite_system.GetPlayerSprite().deadmeat)
    {
      clientspritesnapshot();
    }
  }

  GS::GetWeaponSystem().SetLoadedWMChecksum(createwmchecksum(GS::GetWeaponSystem().GetGuns()));

  if (GS::GetWeaponSystem().GetLoadedWMChecksum() != GS::GetWeaponSystem().GetDefaultWMChecksum())
  {
    if (!gGlobalStateDemo.demoplayer.active())
    {
      GS::GetMainConsole().console(_("Server uses weapon mod (checksum") + ' ' +
                                     (inttostr(GS::GetWeaponSystem().GetLoadedWMChecksum())) + ')',
                                   server_message_color);
    }
  }
}

template <typename T>
auto ReadAndSetValue(BitStream &bs, std::uint8_t cvarid) -> bool
{
  auto &cvi = CVarBase<T>::Find(cvarid);
  if (!cvi.IsValid())
  {
    return false;
  }
  T v = {};
  bs.Read(v);
  cvi = v;
  LogDebug("net_msg", "{} id: {} value: {}", cvi.GetName(), cvi.GetId(), v);
  return true;
}

void clienthandlesynccvars(NetworkContext *netmessage)
{
  if (!verifypacketlargerorequal(sizeof(tmsg_serversynccvars), netmessage->size, msgid_synccvars))
  {
    return;
  }

  auto *varsmsg = reinterpret_cast<pmsg_serversynccvars>(netmessage->packet);
  std::int32_t size = netmessage->size - (sizeof(varsmsg->header) + sizeof(varsmsg->itemcount));
  auto *data = reinterpret_cast<std::uint8_t *>(&varsmsg->data);
  BitStream bs(data, size);

  LogDebug("net_msg", "Read sync variables. Count {}", varsmsg->itemcount);
  for (auto i = 0; i < varsmsg->itemcount; i++)
  {
    std::uint8_t cvarid = 0;
    bs.Read(cvarid);
    if (ReadAndSetValue<std::int32_t>(bs, cvarid))
    {
      continue;
    }
    if (ReadAndSetValue<bool>(bs, cvarid))
    {
      continue;
    }
    if (ReadAndSetValue<float>(bs, cvarid))
    {
      continue;
    }
    if (ReadAndSetValue<std::string>(bs, cvarid))
    {
      continue;
    }

    LogError("net_msg", "Cannot read sync variable {} of id {}", i, cvarid);
    // No support for cvar
    SoldatAssert(false);
  }
}

#pragma region tests

#include <doctest/doctest.h>
#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/scoped.hpp>
#include <boost/di/extension/scopes/session.hpp>
#include <boost/di/extension/scopes/shared.hpp>

namespace
{
  
class NetworkTestClient : public INetwork
{
public:
  template <typename T>
  [[nodiscard]] auto GetData() const -> T &
  {
    SoldatAssert(GetSize() >= sizeof(T));
    return *reinterpret_cast<T *>(mMessage.get());
  }
  [[nodiscard]] auto GetSize() const -> std::int32_t { return mSize; }
  [[nodiscard]] auto GetReliable() const -> bool { return mReliable; }

  void ProcessLoop() override {}
  bool Connect(const std::string_view host, std::uint32_t port) override { return true; }
  bool Disconnect(bool now) override { return true; }
protected:
  auto SendDataImpl(const std::byte *data, const std::int32_t size, const bool reliable,
                    const source_location& /*unused*/) -> bool override final
  {
    mMessage = std::make_unique<std::byte[]>(size);
    std::memcpy(mMessage.get(), data, size);
    mSize = size;
    mReliable = reliable;
    return true;
  }

private:
  std::unique_ptr<std::byte[]> mMessage;
  std::int32_t mSize = 0;
  bool mReliable = false;
};

constexpr auto make_injector()
{
  return boost::di::make_injector<boost::di::extension::shared_config>(
    boost::di::bind<INetwork>.to<NetworkTestClient>().in(boost::di::extension::shared)
  );
}

class NetworkClientConnectionFixture
{
public:
  NetworkClientConnectionFixture()
  {
    GlobalSystems<Config::CLIENT_MODULE>::Init();
    AnimationSystem::Get().LoadAnimObjects("");
    for (auto &s : SpriteSystem::Get().GetSprites())
    {
      s.player = std::make_shared<tplayer>();
    }
  }
  ~NetworkClientConnectionFixture()
  {
    GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  }
  NetworkClientConnectionFixture(const NetworkClientConnectionFixture &) = delete;

protected:
  decltype(make_injector()) injector = make_injector();
};

// Add helper functions
auto verifyPlayer(const tsprite &sprite, const std::string &name, std::uint32_t shirtcolor,
                  std::uint32_t pantscolor, std::uint32_t skincolor, std::uint32_t haircolor,
                  std::uint32_t jetcolor, std::int32_t team, std::int32_t hairstyle,
                  std::int32_t headcap, std::int32_t chain) -> bool
{
  if (!sprite.player)
  {
    return false;
  }
  // Check each player attribute separately
  CHECK_EQ(sprite.player->name, name);
  CHECK_EQ(sprite.player->shirtcolor, shirtcolor);
  CHECK_EQ(sprite.player->pantscolor, pantscolor);
  CHECK_EQ(sprite.player->skincolor, skincolor);
  CHECK_EQ(sprite.player->haircolor, haircolor);
  CHECK_EQ(sprite.player->jetcolor, jetcolor);
  CHECK_EQ(sprite.player->team, team);
  CHECK_EQ(sprite.player->hairstyle, hairstyle);
  CHECK_EQ(sprite.player->headcap, headcap);
  CHECK_EQ(sprite.player->chain, chain);

  // Return true if all checks pass
  return true;
}

TEST_SUITE("NetworkClientConnection")
{
  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "Send requestgame")
  {
    auto& tc = injector.create<NetworkTestClient&>();
    clientrequestgame(tc, "");
    auto msg = tc.GetData<tmsg_requestgame>();
    CHECK_EQ("1.8.0", doctest::String(msg.version.data()));
    CHECK_EQ(msgid_requestgame, msg.header.id);
    CHECK_EQ(46, tc.GetSize());
    CHECK_EQ(true, tc.GetReliable());
  }

  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "Check weird computation of requestgame message size")
  {
    auto& tc = injector.create<NetworkTestClient&>();
    clientrequestgame(tc, "asdf");
    auto msg = tc.GetData<tmsg_requestgame>();
    CHECK_EQ("1.8.0", doctest::String(msg.version.data()));
    CHECK_EQ(msgid_requestgame, msg.header.id);
    CHECK_EQ(50, tc.GetSize());
    CHECK_EQ(true, tc.GetReliable());
  }

  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "Send disconnection event")
  {
    auto& tc = injector.create<NetworkTestClient&>();
    clientrequestgame(tc, "asdf");
    auto msg = tc.GetData<tmsg_requestgame>();
    CHECK_EQ("1.8.0", doctest::String(msg.version.data()));
    CHECK_EQ(msgid_requestgame, msg.header.id);
    CHECK_EQ(50, tc.GetSize());
    CHECK_EQ(true, tc.GetReliable());
  }

  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "Send pong")
  {
    auto& tc = injector.create<NetworkTestClient&>();
    auto pong = injector.create<ClientPongMsg>();
    pong.send(12);
    auto msg = tc.GetData<tmsg_pong>();
    CHECK_EQ(msgid_pong, msg.header.id);
    CHECK_EQ(12, msg.pingnum);
  }

  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "sPreprocessSprites empty list")
  {
    auto &ss = SpriteSystem::Get();
    tmsg_playerslist msg;
    tvector2 pos;
    tvector2 vel;

    // Initialize empty list
    for (auto &name : msg.name)
    {
      std::strcpy(name.data(), "0 ");
    }

    sPreprocessSprites(ss, &msg, pos, vel);

    // Verify no sprites were created (all are inactive)
    for (int i = 1; i <= max_sprites; i++)
    {
      CHECK_FALSE(ss.GetSprite(i).active);
    }
  }

  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "sPreprocessSprites single player")
  {
    auto &ss = SpriteSystem::Get();
    tmsg_playerslist msg;
    tvector2 pos{1.0f, 2.0f};
    tvector2 vel{3.0f, 4.0f};
    for (auto &name : msg.name)
    {
      std::strcpy(name.data(), "0 ");
    }

    // Set first player
    std::strcpy(msg.name[0].data(), "TestPlayer");
    msg.shirtcolor[0] = 0x123456;
    msg.pantscolor[0] = 0x234567;
    msg.skincolor[0] = 0x345678;
    msg.haircolor[0] = 0x456789;
    msg.jetcolor[0] = 0x567890;
    msg.look[0] = 0;
    msg.team[0] = 1;
    msg.pos[0] = pos;
    msg.vel[0] = vel;

    sPreprocessSprites(ss, &msg, pos, vel);

    // Verify sprite 1 was created correctly
    const auto &sprite = ss.GetSprite(1);
    CHECK(sprite.active);
    CHECK(verifyPlayer(sprite, "TestPlayer", 0x123456 | 0xff000000, 0x234567 | 0xff000000,
                       0x345678 | 0xff000000, 0x456789 | 0xff000000, 0x567890, 1, 0, 0, 0));

    // Verify position and velocity
    const auto &velocity = ss.GetVelocity(1);
    CHECK_EQ(3.0f, velocity.x);
    CHECK_EQ(4.0f, velocity.y);
  }

  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "sPreprocessSprites player appearance")
  {
    auto &ss = SpriteSystem::Get();
    tmsg_playerslist msg;
    tvector2 pos;
    tvector2 vel;

    for (auto &name : msg.name)
    {
      std::strcpy(name.data(), "0 ");
    }

    struct TestCase
    {
      std::uint8_t look;
      int expectedHair;
      int expectedHead;
      int expectedChain;
    };

    const TestCase testCases[] = {
      {B1, 1, 0, 0},                     // Hairstyle 1
      {B2, 2, 0, 0},                     // Hairstyle 2
      {B3, 3, 0, 0},                     // Hairstyle 3
      {B4, 4, 0, 0},                     // Hairstyle 4
      {B5, 0, GFX::GOSTEK_HELM, 0},      // Helmet
      {B6, 0, GFX::GOSTEK_KAP, 0},       // Hat
      {B7, 0, 0, 1},                     // Chain 1
      {B8, 0, 0, 2},                     // Chain 2
      {B1 | B5, 1, GFX::GOSTEK_HELM, 0}, // Combined hair + helm
      {B2 | B7, 2, 0, 1},                // Combined hair + chain
    };

    for (const auto &tc : testCases)
    {
      // Reset sprite system state
      for (int i = 1; i <= max_sprites; i++)
      {
        ss.GetSprite(i).active = false;
      }

      std::strcpy(msg.name[0].data(), "TestPlayer");
      msg.team[0] = 1;
      msg.look[0] = tc.look;

      sPreprocessSprites(ss, &msg, pos, vel);

      const auto &sprite = ss.GetSprite(1);
      CHECK(sprite.active);
      CHECK_MESSAGE(sprite.player->hairstyle == tc.expectedHair,
                    "Expected hairstyle " << tc.expectedHair << " for look " << (int)tc.look);
      CHECK_MESSAGE(sprite.player->headcap == tc.expectedHead,
                    "Expected headcap " << tc.expectedHead << " for look " << (int)tc.look);
      CHECK_MESSAGE(sprite.player->chain == tc.expectedChain,
                    "Expected chain " << tc.expectedChain << " for look " << (int)tc.look);
    }
  }

  TEST_CASE_FIXTURE(NetworkClientConnectionFixture, "sPreprocessSprites handles 32 players")
  {
    auto &ss = SpriteSystem::Get();
    tmsg_playerslist msg;
    tvector2 pos{1.0f, 2.0f};
    tvector2 vel{3.0f, 4.0f};

    // Reset sprite system
    for (int i = 1; i <= max_sprites; i++)
    {
      ss.GetSprite(i).active = false;
    }

    // Setup 32 players with unique values
    for (int i = 0; i < max_sprites; i++)
    {
      std::string name = "Player" + std::to_string(i);
      std::strcpy(msg.name[i].data(), name.c_str());
      msg.shirtcolor[i] = 0x100000 + i;
      msg.pantscolor[i] = 0x200000 + i;
      msg.skincolor[i] = 0x300000 + i;
      msg.haircolor[i] = 0x400000 + i;
      msg.jetcolor[i] = 0x500000 + i;
      msg.team[i] = (i % 4) + 1; // Teams 1-4
      msg.look[i] = (1 << (i % 4)) | ( (i + 1) % 3) << 5 | ((i + 2) % 3) << 7; // All possible looks
      msg.pos[i] = tvector2(float(i), float(i * 2));
      msg.vel[i] = tvector2(float(i / 2), float(i / 3));
      msg.predduration[i] = i % 2; // Every other player is predator
    }

    sPreprocessSprites(ss, &msg, pos, vel);

    // Verify all 32 sprites were created correctly
    for (int i = 1; i <= max_sprites; i++)
    {
      const auto &sprite = ss.GetSprite(i);
      CHECK(sprite.active);

      int idx = i - 1; // Convert to 0-based for message array access
      std::string expectedName = "Player" + std::to_string(idx);

      CHECK(verifyPlayer(sprite, expectedName, (0x100000 + idx) | 0xff000000,
                         (0x200000 + idx) | 0xff000000, (0x300000 + idx) | 0xff000000,
                         (0x400000 + idx) | 0xff000000, 0x500000 + idx, (idx % 4) + 1,
                         // Calculate expected appearance based on look flags
                         ((msg.look[idx] & B1)   ? 1
                          : (msg.look[idx] & B2) ? 2
                          : (msg.look[idx] & B3) ? 3
                          : (msg.look[idx] & B4) ? 4
                                                 : 0),
                         ((msg.look[idx] & B5)   ? GFX::GOSTEK_HELM
                          : (msg.look[idx] & B6) ? GFX::GOSTEK_KAP
                                                 : 0),
                         ((msg.look[idx] & B7)   ? 1
                          : (msg.look[idx] & B8) ? 2
                                                 : 0)));

      // Verify velocity
      const auto &velocity = ss.GetVelocity(i);
      CHECK_EQ(float(idx / 2), velocity.x);
      CHECK_EQ(float(idx / 3), velocity.y);

      // Verify predator status
      if (idx % 2 == 1)
      {
        CHECK_EQ(predatoralpha, sprite.alpha);
        CHECK_EQ(60, sprite.bonustime); // predduration * 60
        CHECK_EQ(bonus_predator, sprite.bonusstyle);
      }
    }
  }
} // TEST_SUITE

} // namespace

#pragma endregion tests