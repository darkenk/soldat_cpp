// automatically converted

#include "NetworkClientConnection.hpp"
#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/GameRendering.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../GameStrings.hpp"
#include "../misc/BitStream.hpp"
#include "NetworkClientSprite.hpp"
#include "NetworkUtils.hpp"
#include "common/Logging.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <physfs.h>
#include <shared/Cvar.hpp>

using string = std::string;

namespace
{
} // namespace

// REQUEST GAME FROM SERVER
void clientrequestgame(tclientnetwork& network)
{
  pmsg_requestgame requestmsg;
  std::int32_t size;
  std::vector<std::uint8_t> sendbuffer;

  size = sizeof(tmsg_requestgame) + length(joinpassword) + 1;

  setlength(sendbuffer, size);

  requestmsg = pmsg_requestgame(sendbuffer.data());

  requestmsg->header.id = msgid_requestgame;
  std::strcpy(requestmsg->version.data(), soldat_version);

  requestmsg->haveanticheat = actype_none;

#ifdef ENABLE_FAE
  if (faeisenabled)
    requestmsg.haveanticheat = actype_fae;
#endif

  if (redirectip != "")
  {
    requestmsg->forwarded = 1;
    redirectip = "";
    redirectport = 0;
    redirectmsg = "";
  }
  std::strcpy(requestmsg->hardwareid.data(), hwid.data());

  std::strcpy(requestmsg->password.data(), joinpassword.data());
  network.senddata((std::byte *)(requestmsg), size, k_nSteamNetworkingSend_Reliable);
  // udp->senddata(requestmsg, size, k_nSteamNetworkingSend_Reliable);

  requestinggame = true;
}

// SEND INFO ABOUT NAME, COLOR, PASS etc. TO SERVER OR CHANGE TEAM
void clientsendplayerinfo()
{
  tmsg_playerinfo playerinfo;
  tmsg_changeteam changemsg;

  if ((spectator == 1) && (selteam == 0))
    selteam = team_spectator;
  spectator = 0; // allow joining other teams after first join
  if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
      (CVar::sv_gamemode == gamestyle_htf))
    if ((selteam < team_alpha) || ((selteam > team_bravo) && (selteam < team_spectator)))
    {
      gamemenushow(teammenu);
      return;
    }
  if (CVar::sv_gamemode == gamestyle_teammatch)
    if (selteam < team_alpha)
    {
      gamemenushow(teammenu);
      return;
    }

  // sent a team change request instead if we're already ingame
  if (mysprite > 0)
  {
    changemsg.header.id = msgid_changeteam;
    changemsg.team = selteam;
    GetNetwork()->senddata(&changemsg, sizeof(changemsg), k_nSteamNetworkingSend_Reliable);
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
  playerinfo.team = selteam;
  playerinfo.look = 0;
  if (CVar::cl_player_hairstyle == 1)
    playerinfo.look = playerinfo.look | B1;
  if (CVar::cl_player_hairstyle == 2)
    playerinfo.look = playerinfo.look | B2;
  if (CVar::cl_player_hairstyle == 3)
    playerinfo.look = playerinfo.look | B3;
  if (CVar::cl_player_hairstyle == 4)
    playerinfo.look = playerinfo.look | B4;
  if (CVar::cl_player_headstyle == headstyle_helmet)
    playerinfo.look = playerinfo.look | B5;
  if (CVar::cl_player_headstyle == headstyle_hat)
    playerinfo.look = playerinfo.look | B6;
  if (CVar::cl_player_chainstyle == 1)
    playerinfo.look = playerinfo.look | B7;
  if (CVar::cl_player_chainstyle == 2)
    playerinfo.look = playerinfo.look | B8;

  playerinfo.gamemodchecksum.Dummy[0] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[0]);
  playerinfo.gamemodchecksum.Dummy[1] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[1]);
  playerinfo.gamemodchecksum.Dummy[2] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[2]);
  playerinfo.gamemodchecksum.Dummy[3] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[3]);
  playerinfo.gamemodchecksum.Dummy[4] = htobe32(GS::GetGame().GetGameModChecksum().Dummy[4]);
  playerinfo.custommodchecksum = GS::GetGame().GetCustomModChecksum();

  GetNetwork()->senddata(&playerinfo, sizeof(playerinfo), k_nSteamNetworkingSend_Reliable);
  clientplayersent = true;
  clientplayerreceivedcounter = clientplayerrecieved_time;
}

void clientdisconnect()
{
  tmsg_playerdisconnect playermsg;

  if (mysprite > 0)
  { // send disconnection info to server
    playermsg.header.id = msgid_playerdisconnect;
    playermsg.num = mysprite;

    GetNetwork()->senddata(&playermsg, sizeof(playermsg), k_nSteamNetworkingSend_Reliable);

    auto& fs= GS::GetFileSystem();

    addlinetologfile(fs, GetGameLog(),
                     string("Client Disconnect from ") +
                       GetNetwork()->GetStringAddress(&GetNetwork()->Address(), true),
                     GetGameLogFilename());
    GetNetwork()->processloop();
    GetNetwork()->disconnect(false);
  }
  else
  {
    GetNetwork()->disconnect(true);
    exittomenu();
  }
}

void clientpong(std::uint8_t pingnum)
{
  tmsg_pong pongmsg;

  pongmsg.header.id = msgid_pong;
  pongmsg.pingnum = pingnum;

  GetNetwork()->senddata(&pongmsg, sizeof(pongmsg), k_nSteamNetworkingSend_Reliable);
}

void clienthandleplayerslist(SteamNetworkingMessage_t *netmessage)
{
  tmsg_playerslist *playerslistmsg;
  std::int32_t i;
  tvector2 pos, vel;
  std::string downloadurl;
  tsha1digest checksum;
  bool forcegraphicsreload = false;
  std::string modname;
  std::string mapname;
  tmapinfo mapstatus;

  if (!verifypacket(sizeof(*playerslistmsg), netmessage->m_cbSize, msgid_playerslist))
    return;

  if (!(requestinggame || demoplayer.active()))
    return;

  requestinggame = false;

  playerslistmsg = pmsg_playerslist(netmessage->m_pData);

  if (!demoplayer.active())
    GS::GetMainConsole().console(_("Connection accepted to") + ' ' +
                               (GetNetwork()->GetStringAddress(&GetNetwork()->Address(), true)),
                             client_message_color);

  gClientServerIP = GetNetwork()->GetStringAddress(&GetNetwork()->Address(), false);
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
    if (modname != "")
    {
      checksum = sha1file(GS::GetGame().GetUserDirectory() + "mods/" + modname + ".smod");
      if (playerslistmsg->modchecksum == checksum)
      {
        if (!PHYSFS_mount((pchar)(GS::GetGame().GetUserDirectory() + "mods/" + modname + ".smod"),
                          (pchar)(string("mods/") + modname + '/'), false))
        {
          showmessage(_(string("Could not load mod archive (") + modname + ")."));
          return;
        }
        moddir = string("/mods/") + modname + '/';
        GS::GetGame().SetCustomModChecksum(checksum);
        AnimationSystem::Get().LoadAnimObjects(moddir);
        loadsounds(moddir);
        forcegraphicsreload = true;
        usesservermod = true;
        GS::GetMainConsole().console(_(string("Loading server mod: ") + modname), mode_message_color);
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
      if (usesservermod) // reset to original mod
      {
        moddir = CVar::fs_mod;
        AnimationSystem::Get().LoadAnimObjects(moddir);
        loadsounds(moddir);
        forcegraphicsreload = true;
        usesservermod = false;
      }
    }
  }

  // Initialize Map
  auto &map = GS::GetGame().GetMap();
  auto& fs = GS::GetFileSystem();
  if (getmapinfo(fs, mapname, GS::GetGame().GetUserDirectory(), mapstatus) /*&&
        verifymapchecksum(fs, mapstatus, playerslistmsg->mapchecksum)*/)
  {
    if (!map.loadmap(fs, mapstatus, CVar::r_forcebg, CVar::r_forcebg_color1, CVar::r_forcebg_color2))
    {
      rendergameinfo(_("Could not load map: ") + (mapname));
      exittomenu();
      return;
    }
  }
  else
  {
#ifdef STEAM
    if (mapstatus.workshopid > 0)
    {
      rendergameinfo(_("Downloading workshop item: ") + (inttostr(mapstatus.workshopid)));
      steamapi.ugc.downloaditem(mapstatus.workshopid, true);
      mapchangeitemid = mapstatus.workshopid;
      forcereconnect = true;
      return;
    }
    else
#endif
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
    reloadgraphics();
    forcegraphicsreload = false;
  }

  // Sync cvars
  GS::GetGame().SetPlayersNum(playerslistmsg->players);
  GS::GetGame().SetTimelimitcounter(playerslistmsg->currenttime);

  for (i = 0; i < max_sprites; i++)
  {
    if (strcmp(playerslistmsg->name[i].data(), "0 ") != 0)
    {
      [[deprecated("conversion from 0 to 1")]] auto iplus1 =
        i + 1; // convert from indexing from 0 to indexing from 1
      auto newplayer = SpriteSystem::Get().GetSprite(iplus1).player; // reuse object
      newplayer->name = returnfixedplayername(playerslistmsg->name[i].data());
      newplayer->shirtcolor = playerslistmsg->shirtcolor[i] | 0xff000000;
      newplayer->pantscolor = playerslistmsg->pantscolor[i] | 0xff000000;
      newplayer->skincolor = playerslistmsg->skincolor[i] | 0xff000000;
      newplayer->haircolor = playerslistmsg->haircolor[i] | 0xff000000;
      newplayer->jetcolor = playerslistmsg->jetcolor[i];
      newplayer->team = playerslistmsg->team[i];

#ifdef STEAM
      newplayer.steamid = tsteamid(playerslistmsg->steamid[i]);
#endif

      newplayer->secwep = 0;
      pos = playerslistmsg->pos[i];
      vel = playerslistmsg->vel[i];

      newplayer->hairstyle = 0;
      if ((playerslistmsg->look[i] & B1) == B1)
        newplayer->hairstyle = 1;
      if ((playerslistmsg->look[i] & B2) == B2)
        newplayer->hairstyle = 2;
      if ((playerslistmsg->look[i] & B3) == B3)
        newplayer->hairstyle = 3;
      if ((playerslistmsg->look[i] & B4) == B4)
        newplayer->hairstyle = 4;

      newplayer->headcap = 0;
      if ((playerslistmsg->look[i] & B5) == B5)
        newplayer->headcap = GFX::GOSTEK_HELM;
      if ((playerslistmsg->look[i] & B6) == B6)
        newplayer->headcap = GFX::GOSTEK_KAP;

      newplayer->chain = 0;
      if ((playerslistmsg->look[i] & B7) == B7)
        newplayer->chain = 1;
      if ((playerslistmsg->look[i] & B8) == B8)
        newplayer->chain = 2;

      createsprite(pos, iplus1, newplayer);

      auto &spriteVelocity =
        SpriteSystem::Get().GetVelocity(SpriteSystem::Get().GetSprite(iplus1).num);
      spriteVelocity = vel;

      SpriteSystem::Get().GetSprite(iplus1).ceasefirecounter = 0;
      if (playerslistmsg->predduration[i] > 0)
      {
        SpriteSystem::Get().GetSprite(iplus1).alpha = predatoralpha;
        SpriteSystem::Get().GetSprite(iplus1).bonustime = playerslistmsg->predduration[i] * 60;
        SpriteSystem::Get().GetSprite(iplus1).bonusstyle = bonus_predator;
      }
    }
  }

  if (!demoplayer.active())
    rendergameinfo(_("Waiting to join game..."));

  mysprite = 0;
  camerafollowsprite = 0;
  gamethingtarget = 0;
  selteam = 0;
  menutimer = 0;
  GS::GetGame().SetSurvivalendround(false);
  camerax = 0;
  cameray = 0;

  if (!demoplayer.active())
  {
    GS::GetGame().ResetGoalTicks();
    notexts = 0;
  }

  clientvarsrecieved = false;
  GS::GetGame().ResetMainTickCounter();
  clienttickcount = playerslistmsg->serverticks;
  noheartbeattime = 0;
  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
  gamemenushow(escmenu, false);
  limbolock = false;

  if (GS::GetGame().IsVoteActive())
    GS::GetGame().stopvote();

  resetweaponstats();

  clientplayerreceived = false;
  clientplayersent = false;
  clientplayerreceivedcounter = clientplayerrecieved_time;

  // Begin rendering so that the team menu selection is visible
  if (!(demoplayer.active() && (demoplayer.skipto() == -1)))
    shouldrenderframes = true;

  if (CVar::cl_player_team > 0)
  {
    // Bypass Team Select Menu if team cvar is set
    selteam = CVar::cl_player_team;
    clientsendplayerinfo();
  }
  else if (spectator == 1)
    clientsendplayerinfo();
  else
  {
    if ((CVar::sv_gamemode == gamestyle_deathmatch) ||
        (CVar::sv_gamemode == gamestyle_pointmatch) || (CVar::sv_gamemode == gamestyle_rambo))
      clientsendplayerinfo();

    if (CVar::sv_gamemode == gamestyle_teammatch)
      gamemenushow(teammenu);

    if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
        (CVar::sv_gamemode == gamestyle_htf))
    {
      gamemenushow(teammenu);
      if (CVar::sv_balanceteams)
      {
        selteam = 0;
        if (GS::GetGame().GetPlayersTeamNum(1) < GS::GetGame().GetPlayersTeamNum(2))
          selteam = 1;
        if (GS::GetGame().GetPlayersTeamNum(2) < GS::GetGame().GetPlayersTeamNum(1))
          selteam = 2;
        if (selteam > 0)
          clientsendplayerinfo();
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
    GS::GetMainConsole().console(_("Survival Mode ON"), mode_message_color);
  if (CVar::sv_advancemode)
    GS::GetMainConsole().console(_("Advance Mode ON"), mode_message_color);

  // newby stuff
  if (CVar::cl_runs < 3)
  {
    if (Random(3) == 0)
      GS::GetMainConsole().console(
        _("(!) Jet fuel is map specific. There can be more or less on certain maps."),
        info_message_color);
    else
    {
      GS::GetMainConsole().console(_("(!) To leave your current weapon after respawn"),
                               info_message_color);
      GS::GetMainConsole().console(string("    ") + _("click anywhere outside the weapons menu."),
                               info_message_color);
    }

    if (CVar::sv_realisticmode)
      GS::GetMainConsole().console(_("(!) To prevent weapon recoil fire float shots or short bursts."),
                               info_message_color);
  }

  mx = gamewidthhalf;
  my = gameheighthalf;
  mouseprev.x = mx;
  mouseprev.y = my;
#endif
}

void clienthandleunaccepted(SteamNetworkingMessage_t *netmessage)
{
  pmsg_unaccepted unacceptedmsg;
  std::string text;
  std::int32_t textlen;

  if (!verifypacketlargerorequal(sizeof(unacceptedmsg), netmessage->m_cbSize, msgid_unaccepted))
    return;

  unacceptedmsg = pmsg_unaccepted(netmessage->m_pData);
  textlen = netmessage->m_cbSize - sizeof(tmsg_unaccepted);

  if ((textlen > 0) && (unacceptedmsg->text[textlen - 1] == '\0'))
    text = unacceptedmsg->text.data();
  else
    text = "";


  auto& fs = GS::GetFileSystem();

  addlinetologfile(fs, GetGameLog(), string("*UA ") + inttostr(unacceptedmsg->state),
                   GetGameLogFilename());

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
    rendergameinfo(_("Wrong server password"));
    break;

  case banned_ip:
    rendergameinfo(_("You have been banned on this server. Reason:") + ' ' + text);
    break;

  case server_full:
    rendergameinfo(_("Server is full"));
    break;

  case invalid_handshake:
    rendergameinfo(_("Unspecified internal protocol error"));
    break;

  case wrong_checksum:
    rendergameinfo(_("This server requires a different smod file."));
    break;

#ifdef STEAM
  case steam_only:
    rendergameinfo(_("This server accepts only Steam players."));
    break;
#endif

  case anticheat_rejected:
    rendergameinfo(_("Rejected by Anti-Cheat:") + ' ' + text);
    break;
  }

  clientdisconnect();
  exittomenu();
}

void clienthandleserverdisconnect(SteamNetworkingMessage_t *netmessage)
{
  if (!verifypacket(sizeof(tmsg_serverdisconnect), netmessage->m_cbSize, msgid_serverdisconnect))
    return;

  GS::GetGame().showmapchangescoreboard("");

  if (!demoplayer.active())
    GS::GetMainConsole().console(_("Server disconnected"), server_message_color);
  else
    demoplayer.stopdemo();
}

void clienthandleping(SteamNetworkingMessage_t *netmessage)
{
  if (!verifypacket(sizeof(tmsg_ping), netmessage->m_cbSize, msgid_ping))
    return;

  if (demoplayer.active())
    return;

  if (mysprite != 0)
  {
    SpriteSystem::Get().GetSprite(mysprite).player->pingticks =
      pmsg_ping(netmessage->m_pData)->pingticks;
    SpriteSystem::Get().GetSprite(mysprite).player->pingtime =
      SpriteSystem::Get().GetSprite(mysprite).player->pingticks * 1000 / 60;
  }

  clientpong(pmsg_ping(netmessage->m_pData)->pingnum);

  clientstopmovingcounter = clientstopmove_retrys;
  noheartbeattime = 0;
}

void clienthandleservervars(SteamNetworkingMessage_t *netmessage)
{
  tmsg_servervars *varsmsg;
  std::int32_t i;
  std::int32_t weaponindex;

  if (!verifypacket(sizeof(tmsg_servervars), netmessage->m_cbSize, msgid_servervars))
    return;

  varsmsg = pmsg_servervars(netmessage->m_pData);

  clientvarsrecieved = true;

  auto &weaponSystem = GS::GetWeaponSystem();

  for (i = 1; i <= main_weapons; i++)
  {
    weaponSystem.EnableWeapon(i, varsmsg->weaponactive[i - 1] == 1);
    limbomenu->button[i - 1].active = weaponSystem.IsEnabled(i);
  }

  if (mysprite > 0)
  {
    selectdefaultweapons(mysprite);
    newplayerweapon();
  }

  createdefaultweapons(CVar::sv_realisticmode, GS::GetWeaponSystem().GetGuns(),
                       GS::GetWeaponSystem().GetDefaultGuns());
  GS::GetWeaponSystem().SetDefaultWMChecksum(createwmchecksum(GS::GetWeaponSystem().GetGuns()));

  for (weaponindex = 0; weaponindex < original_weapons; weaponindex++)
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

  if (mysprite > 0)
  {
    SpriteSystem::Get().GetSprite(mysprite).applyweaponbynum(
      SpriteSystem::Get().GetSprite(mysprite).weapon.num, 1);
    SpriteSystem::Get().GetSprite(mysprite).applyweaponbynum(
      SpriteSystem::Get().GetSprite(mysprite).secondaryweapon.num, 2);
    if (!SpriteSystem::Get().GetSprite(mysprite).deadmeat)
      clientspritesnapshot();
  }

  GS::GetWeaponSystem().SetLoadedWMChecksum(createwmchecksum(GS::GetWeaponSystem().GetGuns()));

  if (GS::GetWeaponSystem().GetLoadedWMChecksum() != GS::GetWeaponSystem().GetDefaultWMChecksum())
  {
    if (!demoplayer.active())
    {
      GS::GetMainConsole().console(_("Server uses weapon mod (checksum") + ' ' +
                                 (inttostr(GS::GetWeaponSystem().GetLoadedWMChecksum())) + ')',
                               server_message_color);
    }
  }
}

template <typename T>
bool ReadAndSetValue(BitStream &bs, std::uint8_t cvarid)
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

void clienthandlesynccvars(SteamNetworkingMessage_t *netmessage)
{
  tmsg_serversynccvars *varsmsg;
  std::int32_t size;

  if (!verifypacketlargerorequal(sizeof(tmsg_serversynccvars), netmessage->m_cbSize,
                                 msgid_synccvars))
    return;

  varsmsg = pmsg_serversynccvars(netmessage->m_pData);
  size = netmessage->m_cbSize - (sizeof(varsmsg->header) + sizeof(varsmsg->itemcount));
  std::uint8_t *data = reinterpret_cast<std::uint8_t *>(&varsmsg->data);
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
