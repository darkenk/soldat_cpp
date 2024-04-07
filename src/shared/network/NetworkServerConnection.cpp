// automatically converted
#include "NetworkServerConnection.hpp"
#include "NetworkServer.hpp"
#include "../../server/BanSystem.hpp"
#include "../../server/Server.hpp"
#include "../../server/ServerHelper.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../LogFile.hpp"
#include "../mechanics/Sprites.hpp"
#include "../mechanics/Things.hpp"
#include "../misc/BitStream.hpp"
#include "NetworkServerGame.hpp"
#include "NetworkServerMessages.hpp"
#include "NetworkServerThing.hpp"
#include "NetworkUtils.hpp"
#include "common/Logging.hpp"
#include "common/gfx.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

auto constexpr minsperhour = 60;
auto constexpr minsperday = 24 * minsperhour;

#ifdef SERVER
std::array<std::array<std::int32_t, max_players>, 10> pingtime;
std::array<std::uint8_t, max_players> pingsendcount;
#endif

#ifdef SERVER
void serverhandlerequestgame(SteamNetworkingMessage_t *netmessage)
{
  TServerPlayer *player;
  tmsg_requestgame *requestmsg;
  std::uint32_t state;
  std::int32_t banindex;
  std::int32_t id;
  std::string banreason;
  bool banhw;

  if (!verifypacketlargerorequal(sizeof(tmsg_requestgame), netmessage->m_cbSize, msgid_requestgame))
    return;

  // Player := TPlayer(NetMessage^.m_pData);

  player = GetServerNetwork()->GetPlayer(netmessage);
  banindex = 0;
  banhw = false;
  banreason = "";

  // Fancy packet filter
  id = updateantiflood(player->ip);
  if (isfloodid(id))
    return;

  requestmsg = pmsg_requestgame(netmessage->m_pData);

  // Begin by checking the client's version
  if (iswronggameversion(requestmsg->version.data()))
  {
    state = wrong_version;
    serversendunaccepted(netmessage->m_conn, state);
    return;
  }

#ifdef ENABLE_FAE
  // Reject clients without anti-cheat if this server requires it
  if ((CVar::ac_enable) && (requestmsg.haveanticheat != actype_fae))
  {
    state = anticheat_required;
    serversendunaccepted(netmessage->m_conn, state);
    return;
  }
#endif

  if (isservertotallyfull())
    state = server_full;
  else if (isremoteadminip(player->ip))
    state = ok;
  else if (isadminpassword((pmsg_requestgame(netmessage->m_pData)->password.data())))
  {
    if (addiptoremoteadmins(player->ip))
      GS::GetMainConsole().console(player->ip + " added to Game Admins via Request password",
                                     server_message_color);
    state = ok;
  }
  else
  {
    banindex = findban(player->ip);

    if (banindex > 0)
    {
      state = banned_ip;
      banreason = std::string(" (") + bannediplist[banindex].reason + ')';
    }
    else
    {
#ifdef STEAM
      banindex = findbanhw(inttostr(tsteamid(player.steamid).getaccountid));
#else
      banindex = findbanhw(requestmsg->hardwareid.data());
#endif
      if (banindex > 0)
      {
        state = banned_ip;
        banhw = true;
        banreason = std::string(" (") + bannedhwlist[banindex].reason + ')';
      }
      else if (iswronggamepassword((pmsg_requestgame(netmessage->m_pData)->password.data())))
        state = wrong_password;
      else if (isserverfull())
        state = server_full;
      else
        state = ok;
    }
  }

#ifdef STEAM
  if (CVar::sv_steamonly)
    if (uint64(player.steamid) == 0)
      state = steam_only;
#endif

  GS::GetMainConsole().console(
    player->ip + ':' + inttostr(player->port) +
      //    '|' + {$IFDEF
      //    STEAM}TSteamID(Player.SteamID).GetAsString{$ELSE}RequestMsg.HardwareID{$ENDIF} +
      " requesting game" + banreason + "...",
    server_message_color);

#ifdef SCRIPT
  if (CVar::sc_enable)
  {
    state =
      scrptdispatcher.onrequestgame(player.ip,
                                    //      {$IFDEF STEAM}
                                    //      TSteamID(Player.SteamID).GetAsString
                                    //      {$ELSE}
                                    //      RequestMsg.HardwareID
                                    //      {$ENDIF},
                                    player.port, state, (bool)(requestmsg.forwarded),
                                    (pchar)(&pmsg_requestgame(netmessage->m_pData)->password));
  }
#endif

  if (state == ok)
  {
    player->gamerequested = true;
#ifdef ENABLE_FAE
    if (CVar::ac_enable)
      serversendfaechallenge(netmessage->m_conn, true);
#endif
    serversynccvars(0, netmessage->m_conn, true);
    serversendplaylist(netmessage->m_conn);
  }
  else
  {
    LogInfoG("Request rejected: {}", state);
    serversendunaccepted(netmessage->m_conn, state, getbanstrforindex(banindex, banhw));
  }
}

void serverhandleplayerinfo(SteamNetworkingMessage_t *netmessage)
{
  pmsg_playerinfo playerinfomsg;
  TServerPlayer *player;
  std::string fixedplayername, finalplayername;
  bool playernameunused;
  std::int32_t suffixlen;
  std::int32_t i, j;
  tvector2 a;
#ifdef SCRIPT
  std::int8_t newteam;
#endif

  if (!verifypacket(sizeof(tmsg_playerinfo), netmessage->m_cbSize, msgid_playerinfo))
    return;

  auto &things = GS::GetThingSystem().GetThings();

  playerinfomsg = pmsg_playerinfo(netmessage->m_pData);
  player = GetServerNetwork()->GetPlayer(netmessage);
  SoldatAssert(player->spritenum == 0);

#ifdef ENABLE_FAE
  if (player.faekicked)
  {
    debug("Rejecting, FaeKicked flag is set");
    return; // already sent unaccept message in NetworkServerFae
  }

  // We expect the client to send a Fae response prior to sending its player info.
  // Note that in case of Fae being disabled that bool is always false, so no need to check the
  // cvar. This is guaranteed to arrive in order due to GameNetworkingSockets magic, thus we can
  // just check the pending bool:
  if (player.faeresponsepending)
  {
    debug("Rejecting, no Fae response");
    serversendunaccepted(event.peer, invalid_handshake);
    return;
  }
#endif

  NotImplemented("no sha1match");
#if 0
    if ((CVar::sv_pure and
         (!sha1match(tsha1digest(playerinfomsg->gamemodchecksum), GS::GetGame().GetGameModChecksum()))) or
        (!sha1match(tsha1digest(playerinfomsg->custommodchecksum), GS::GetGame().GetCustommodchecksum())))
    {
        serversendunaccepted(player.peer, wrong_checksum);
        return;
    }
#endif

  if (isservertotallyfull() ||
      (((GS::GetGame().GetPlayersNum() - GS::GetGame().GetBotsNum()) >= CVar::sv_maxplayers) &&
       (!isremoteadminip(player->ip))))
  {
    serversendunaccepted(player->peer, server_full);
    return;
  }

  if (playerinfomsg->team > team_spectator)
  {
    serversendunaccepted(player->peer, invalid_handshake);
    return;
  }

  // change name if is already taken
  fixedplayername = fixplayername(playerinfomsg->name.data());
  finalplayername = fixedplayername;
  j = 0;
  do
  {
    playernameunused = true;
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if (sprite.player->name == finalplayername)
      {
        auto jstring = inttostr(j);
        suffixlen = length(jstring) + 2;
        j += 1;

        // Truncate nick too long to append (NN) for duplicate names
        if (length(finalplayername) + suffixlen <= 24)
          finalplayername = fixedplayername + '(' + jstring + ')';
        else
        {
#if 0
                    leftstr(fixedplayername, 22 - length(inttostr(j)))
#endif
          finalplayername = fixedplayername.substr(0, 22) + '(' + jstring + ')';
        }
        playernameunused = false;
      }
    }
  } while (!playernameunused);

  GS::GetMainConsole().console(finalplayername + " joining game (" + player->ip + ':' +
                                   inttostr(player->port) + ") HWID:" +
#ifdef STEAM
                                   Player.SteamID.GetAsString()
#else
                                   player->hwid
#endif
                                   ,
                                 server_message_color);

  // Set a network name for debugging purposes
  GetServerNetwork()->SetConnectionName(player->peer, finalplayername);

  player->name = finalplayername;
  player->shirtcolor = playerinfomsg->shirtcolor & 0xffffff;
  player->pantscolor = playerinfomsg->pantscolor & 0xffffff;
  player->skincolor = playerinfomsg->skincolor & 0xffffff;
  player->haircolor = playerinfomsg->haircolor;
  player->jetcolor = playerinfomsg->jetcolor;
  player->controlmethod = human;

  // select team and fix an impossible team selection
  player->team = playerinfomsg->team;
  switch (CVar::sv_gamemode)
  {
    // no teams
  case gamestyle_deathmatch:
  case gamestyle_pointmatch:
  case gamestyle_rambo:
    if ((player->team != team_none) && (player->team != team_spectator))
      player->team = team_spectator;
    break;
    // two teams
  case gamestyle_ctf:
  case gamestyle_inf:
  case gamestyle_htf:
    if ((player->team < team_alpha) || (player->team > team_bravo))
      player->team = team_spectator;
    break;
    // four teams
  case gamestyle_teammatch:
    if ((player->team < team_alpha) || (player->team > team_delta))
      player->team = team_spectator;
    break;
  }

  // enforce spectator limit
  if ((playerinfomsg->team == team_spectator) &&
      (GS::GetGame().GetSpectatorsNum() >= CVar::sv_maxspectators))
  {
    if (!isremoteadminip(player->ip))
    {
      serversendunaccepted(player->peer, server_full);
      return;
    }
  }

  // restore warnings across sessions
  for (j = 1; j <= max_players; j++)
    if (trim(mutelist[j]) == player->ip)
    {
      player->muted = 1;
      player->name = mutename[j];
      break;
    }
  player->tkwarnings = 0;
  for (j = 1; j <= max_players; j++)
    if (trim(tklist[j]) == player->ip)
    {
      player->tkwarnings = tklistkills[j];
      break;
    }

  player->playtime = 0;

  player->hairstyle = 0;
  if ((playerinfomsg->look & B1) == B1)
    player->hairstyle = 1;
  if ((playerinfomsg->look & B2) == B2)
    player->hairstyle = 2;
  if ((playerinfomsg->look & B3) == B3)
    player->hairstyle = 3;
  if ((playerinfomsg->look & B4) == B4)
    player->hairstyle = 4;

  player->headcap = 0;
  if ((playerinfomsg->look & B5) == B5)
    player->headcap = GFX::GOSTEK_HELM;
  if ((playerinfomsg->look & B6) == B6)
    player->headcap = GFX::GOSTEK_KAP;

  player->chain = 0;
  if ((playerinfomsg->look & B7) == B7)
    player->chain = 1;
  if ((playerinfomsg->look & B8) == B8)
    player->chain = 2;

#ifdef SCRIPT
  newteam = scrptdispatcher.onbeforejointeam(player.spritenum, player.team, 255);
  if ((newteam >= team_none) && (newteam <= team_spectator))
    player.team = newteam;
  else
  {
    SpriteSystem::Get().GetSprite(player.spritenum).kill();
    serversendunaccepted(player.peer, server_full);
    return;
  }
#endif

  player->applyshirtcolorfromteam();

  // ------------------------
  // NOTE we're done fixing up the Player structure now -- don't change it below this comment
  // next the player's sprite is initialized

  // create sprite and assign it our player object
  randomizestart(a, playerinfomsg->team);
  auto players = GetServerNetwork()->GetPlayers();
  auto it = std::find_if(players.begin(), players.end(),
                         [&player](const auto &v) { return v.get() == player; });
  SoldatAssert(it != players.end());
  createsprite(a, 255, *it); // assigns Player.SpriteNum

  // respawn holded thing if is
  // FIXME english
  // TODO do a good bit of these things in CreateSprite instead?
  if (SpriteSystem::Get().GetSprite(player->spritenum).holdedthing > 0)
    if (things[SpriteSystem::Get().GetSprite(player->spritenum).holdedthing].style < object_ussocom)
    {
      things[SpriteSystem::Get().GetSprite(player->spritenum).holdedthing].respawn();
      SpriteSystem::Get().GetSprite(player->spritenum).holdedthing = 0;
    }
  for (i = 1; i <= max_things; i++)
    if (things[player->spritenum].holdingsprite == player->spritenum)
      things[player->spritenum].respawn();
  SpriteSystem::Get().GetSprite(player->spritenum).haspack = false;
  SpriteSystem::Get().GetSprite(player->spritenum).respawn(); // FIXME do this later?

  if (CVar::sv_survivalmode)
  {
    SpriteSystem::Get().GetSprite(player->spritenum).healthhit(150, player->spritenum, 1, -1, a);
    SpriteSystem::Get().GetSprite(player->spritenum).player->deaths -= 1;
  }

  // reset legacy-ish counters that are stored under the sprite ID
  // TODO: most of this can be moved to TPlayer
  noclientupdatetime[player->spritenum] = 0;
  messagesasecnum[player->spritenum] = 0;
  floodwarnings[player->spritenum] = 0;
  pingwarnings[player->spritenum] = 0;
  bullettime[player->spritenum] = -1000;
  grenadetime[player->spritenum] = -1000;
  knifecan[player->spritenum] = true;
  bulletwarningcount[player->spritenum] = 0;
  cheattag[player->spritenum] = 0;
  GS::GetGame().ResetVoteCooldown(player->spritenum);
  lastplayer = player->spritenum; // for /kicklast command

  serversendnewplayerinfo(player->spritenum, join_normal);
  serverthingmustsnapshotonconnect(player->spritenum);
  servervars(player->spritenum);
  serversyncmsg(player->spritenum);

  // greetings message
  if (length((std::string)CVar::sv_greeting) > 0)
    serversendstringmessage((CVar::sv_greeting), player->spritenum, 255, msgtype_pub);
  if (length((std::string)CVar::sv_greeting2) > 0)
    serversendstringmessage((CVar::sv_greeting2), player->spritenum, 255, msgtype_pub);
  if (length((std::string)CVar::sv_greeting3) > 0)
    serversendstringmessage((CVar::sv_greeting3), player->spritenum, 255, msgtype_pub);

  switch (player->team)
  {
  case team_none:
    GS::GetMainConsole().console(player->name + " has joined the game.", enter_message_color);
    break;
  case team_alpha:
    GS::GetMainConsole().console(player->name + " has joined alpha team.", alphaj_message_color);
    break;
  case team_bravo:
    GS::GetMainConsole().console(player->name + " has joined bravo team.", bravoj_message_color);
    break;
  case team_charlie:
    GS::GetMainConsole().console(player->name + " has joined charlie team.",
                                   charliej_message_color);
    break;
  case team_delta:
    GS::GetMainConsole().console(player->name + " has joined delta team.", deltaj_message_color);
    break;
  case team_spectator:
    GS::GetMainConsole().console(player->name + " has joined as spectator.",
                                   deltaj_message_color);
    break;
  }

  // check if map change is in progress
  if ((GS::GetGame().GetMapchangecounter() > -60) &&
      (GS::GetGame().GetMapchangecounter() < 99999999))
    servermapchange(player->spritenum);

#ifdef SCRIPT
  scrptdispatcher.onjointeam(SpriteSystem::Get().GetSprite(player.spritenum).num, player.team,
                             player.team, true);
#endif

#ifdef STEAMSTATS
  requestuserstats(player.steamid);
#endif

  if (SpriteSystem::Get()
        .GetSprite(player->spritenum)
        .active) // FIXME like above, it's always active is it not?
  {
    dobalancebots(0, player->team);
  }

  // flood from IP prevention
  // TODO eek, this should rather be done with enet (reject on connect, or using a built-in
  // feature)
  j = 0;
  for (i = 1; i <= max_floodips; i++)
    if (floodip[i] == player->ip)
    {
      j = i;
      break;
    }
  if (j > 0)
  {
    floodnum[j] += 1;
    if (floodnum[j] > floodip_max)
      kickplayer(player->spritenum, true, kick_flooding, twenty_minutes, "Join game flood");
  }
  if (j == 0)
    for (i = 1; i <= max_floodips; i++)
      if (floodip[i] == " ")
      {
        floodip[i] = player->ip;
        break;
      }
}
#endif

void serversendplaylist(HSteamNetConnection peer)
{
  tmsg_playerslist playerslist;

  playerslist.header.id = msgid_playerslist;

  stringtoarray(playerslist.modname.data(), CVar::fs_mod);
  playerslist.modchecksum = GS::GetGame().GetCustomModChecksum();
  auto &map = GS::GetGame().GetMap();

  stringtoarray(playerslist.mapname.data(), map.name);
  playerslist.mapchecksum = GS::GetGame().GetMapChecksum();

  playerslist.players = GS::GetGame().GetPlayersNum();
  playerslist.currenttime = GS::GetGame().GetTimelimitcounter();

#ifdef SERVER
  playerslist.serverticks = servertickcounter;
#else
  playerslist.serverticks = GS::GetGame().GetMainTickCounter();
#endif

#ifdef ENABLE_FAE
  playerslist.anticheatrequired = CVar::ac_enable;
#else
  playerslist.anticheatrequired = false;
#endif

  auto i = 0;
  for (auto &s : SpriteSystem::Get().GetSprites())
  {
    if (s.IsActive() and !s.player->demoplayer)
    {
      stringtoarray(playerslist.name[i].data(), s.player->name);
      playerslist.shirtcolor[i] = 0xff000000 | s.player->shirtcolor;
      playerslist.pantscolor[i] = 0xff000000 | s.player->pantscolor;
      playerslist.skincolor[i] = 0xff000000 | s.player->skincolor;
      playerslist.haircolor[i] = 0xff000000 | s.player->haircolor;
      playerslist.jetcolor[i] = s.player->jetcolor;
      playerslist.team[i] = s.player->team;
      playerslist.predduration[i] =
        iif(s.bonusstyle == bonus_predator, ((float)(s.bonustime) / 60), 0.f);

      playerslist.look[i] = 0;
      if (s.player->hairstyle == 1)
        playerslist.look[i] = playerslist.look[i] | B1;
      if (s.player->hairstyle == 2)
        playerslist.look[i] = playerslist.look[i] | B2;
      if (s.player->hairstyle == 3)
        playerslist.look[i] = playerslist.look[i] | B3;
      if (s.player->hairstyle == 4)
        playerslist.look[i] = playerslist.look[i] | B4;
      if (s.player->headcap == GFX::GOSTEK_HELM)
        playerslist.look[i] = playerslist.look[i] | B5;
      if (s.player->headcap == GFX::GOSTEK_KAP)
        playerslist.look[i] = playerslist.look[i] | B6;
      if (s.player->chain == 1)
        playerslist.look[i] = playerslist.look[i] | B7;
      if (s.player->chain == 2)
        playerslist.look[i] = playerslist.look[i] | B8;

      const auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(s.num);
      const auto &spriteVelocity = SpriteSystem::Get().GetVelocity(s.num);
      playerslist.pos[i] = spritePartsPos;
      playerslist.vel[i] = spriteVelocity;
      playerslist.steamid[i] = 0;
    }
    else
    {
      stringtoarray(playerslist.name[i].data(), "0 ");
      playerslist.shirtcolor[i] = 0;
      playerslist.pantscolor[i] = 0;
      playerslist.skincolor[i] = 0;
      playerslist.haircolor[i] = 0;
      playerslist.jetcolor[i] = 0;
      playerslist.team[i] = team_none;
      playerslist.predduration[i] = 0;
      playerslist.look[i] = 0;
      playerslist.pos[i] = vector2(0, 0);
      playerslist.vel[i] = vector2(0, 0);
      playerslist.steamid[i] = 0;
    }
    i++;
  }

#ifdef SERVER
  GetServerNetwork()->SendData(&playerslist, sizeof(playerslist), peer,
                               k_nSteamNetworkingSend_Reliable);
#else
  GS::GetDemoRecorder().saverecord(playerslist, sizeof(playerslist));
#endif
}

#ifdef SERVER
std::string getbanstrforindex(std::int32_t banindex, bool banhw)
{
  std::string bantimestr;

  // check if has ban description
  std::string result;
  if (banindex < 0)
  {
    bantimestr = "";
  }
  else
  {
    if (banhw)
    {
      // get ban time
      if ((bannedhwlist[banindex].time + 1) / 3600 >= minsperday)
      {
        bantimestr = "24+ Hrs";
      }
      else
      {
        if ((bannedhwlist[banindex].time / 3600) >= minsperhour)
          bantimestr = inttostr((bannedhwlist[banindex].time + 1) / 3600 / minsperhour) + 'h';
        else
          bantimestr = inttostr((bannedhwlist[banindex].time + 1) / 3600) + 'm';
      }
      // add ban reason
      bantimestr = bannedhwlist[banindex].reason + " (" + bantimestr + ')';
    }
    else
    {
      // get ban time
      if ((bannediplist[banindex].time + 1) / 3600 >= minsperday)
      {
        bantimestr = "24+ Hrs";
      }
      else
      {
        if ((bannediplist[banindex].time / 3600) >= minsperhour)
          bantimestr = inttostr((bannediplist[banindex].time + 1) / 3600 / minsperhour) + 'h';
        else
          bantimestr = inttostr((bannediplist[banindex].time + 1) / 3600) + 'm';
      }
      // add ban reason
      bantimestr = bannediplist[banindex].reason + " (" + bantimestr + ')';
    }
  }
  result = bantimestr;
  return result;
}

void serversendunaccepted(HSteamNetConnection peer, std::uint8_t state, std::string message)
{
  pmsg_unaccepted unaccepted;
  std::int32_t size;
  std::vector<std::uint8_t> sendbuffer;

  // request memory
  size = sizeof(tmsg_unaccepted) + length(message) + 1;
  setlength(sendbuffer, size); // can throw out of memory exception
  unaccepted = pmsg_unaccepted(sendbuffer.data());

  // fill memory
  unaccepted->header.id = msgid_unaccepted;
  unaccepted->state = state;
  std::strcpy(unaccepted->version.data(), soldat_version);
  strcpy(unaccepted->text.data(), message.data());

  GetServerNetwork()->SendData(&unaccepted, size, peer, k_nSteamNetworkingSend_Reliable);

  GetServerNetwork()->NetworkingSocket().CloseConnection(peer, 0, "", true);
}
#endif

void serversendnewplayerinfo(std::uint8_t num, std::uint8_t jointype)
{
  tmsg_newplayer newplayer;

  newplayer.header.id = msgid_newplayer;
  newplayer.num = num;
  newplayer.jointype = jointype;

  stringtoarray(newplayer.name.data(), SpriteSystem::Get().GetSprite(num).player->name);
  newplayer.shirtcolor = SpriteSystem::Get().GetSprite(num).player->shirtcolor & 0xffffff;
  newplayer.pantscolor = SpriteSystem::Get().GetSprite(num).player->pantscolor & 0xffffff;
  newplayer.skincolor = SpriteSystem::Get().GetSprite(num).player->skincolor & 0xffffff;
  newplayer.haircolor = SpriteSystem::Get().GetSprite(num).player->haircolor & 0xffffff;
  newplayer.jetcolor = SpriteSystem::Get().GetSprite(num).player->jetcolor;
  newplayer.team = SpriteSystem::Get().GetSprite(num).player->team;
  newplayer.pos = SpriteSystem::Get().GetSpritePartsPos(num);
  //  NewPlayer.SteamID := {$IFDEF
  //  STEAM}UInt64(SpriteSystem::Get().GetSprite(Num).Player.SteamID){$ELSE}0{$ENDIF};

  if (newplayer.team == team_spectator)
    newplayer.shirtcolor = colortohex(0xffffff);

  newplayer.look = 0;
  if (SpriteSystem::Get().GetSprite(num).player->hairstyle == 1)
    newplayer.look = newplayer.look | B1;
  if (SpriteSystem::Get().GetSprite(num).player->hairstyle == 2)
    newplayer.look = newplayer.look | B2;
  if (SpriteSystem::Get().GetSprite(num).player->hairstyle == 3)
    newplayer.look = newplayer.look | B3;
  if (SpriteSystem::Get().GetSprite(num).player->hairstyle == 4)
    newplayer.look = newplayer.look | B4;
  if (SpriteSystem::Get().GetSprite(num).player->headcap == GFX::GOSTEK_HELM)
    newplayer.look = newplayer.look | B5;
  if (SpriteSystem::Get().GetSprite(num).player->headcap == GFX::GOSTEK_KAP)
    newplayer.look = newplayer.look | B6;
  if (SpriteSystem::Get().GetSprite(num).player->chain == 1)
    newplayer.look = newplayer.look | B7;
  if (SpriteSystem::Get().GetSprite(num).player->chain == 2)
    newplayer.look = newplayer.look | B8;

#ifdef SERVER
  // NOTE we also send to pending players to avoid desynchronization of the players list
  if (!SpriteSystem::Get().GetSprite(num).player->demoplayer)
  {
    auto players = GetServerNetwork()->GetPlayers();
    for (auto &player : players)
    {
      newplayer.adoptspriteid = num == player->spritenum;
      GetServerNetwork()->SendData(&newplayer, sizeof(newplayer), player->peer,
                                   k_nSteamNetworkingSend_Reliable);
    }
  }
  else
  {
    if (GS::GetDemoRecorder().active())
    {
      newplayer.adoptspriteid =
        (std::uint8_t)(SpriteSystem::Get().GetSprite(num).player->demoplayer == true);
      GS::GetDemoRecorder().saverecord(&newplayer, sizeof(newplayer));
    }
  }
  NotImplemented("No time functions");
#if 0
    addlinetologfile(gamelog,
                     std::string(" Net - ") + SpriteSystem::Get().GetSprite(num).player->name + " connected " +
                         datetostr(get_date()) + ' ' + timetostr(get_time()),
                     consolelogfilename);
#endif
#else
  newplayer.adoptspriteid = 1;
  GS::GetDemoRecorder().saverecord(newplayer, sizeof(newplayer));
#endif
}

#ifdef SERVER
void serverdisconnect()
{
  tmsg_serverdisconnect servermsg;

  servermsg.header.id = msgid_serverdisconnect;

  // NOTE send to pending like above
  auto players = GetServerNetwork()->GetPlayers();
  for (const auto &dstplayer : players)
  {
    GetServerNetwork()->SendData(&servermsg, sizeof(servermsg), dstplayer->peer,
                                 k_nSteamNetworkingSend_Reliable);
  }

  for (auto &s : SpriteSystem::Get().GetActiveSprites())
  {
    if (s.player->controlmethod == human)
    {
      s.kill();
    }
  }
}

void serverplayerdisconnect(std::uint8_t num, std::uint8_t why)
{
  tmsg_playerdisconnect playermsg;
  tplayer dstplayer;

  playermsg.header.id = msgid_playerdisconnect;
  playermsg.num = num;
  playermsg.why = why;

  // NOTE send to pending like above
  auto players = GetServerNetwork()->GetPlayers();
  for (const auto &dstplayer : players)
  {
    GetServerNetwork()->SendData(&playermsg, sizeof(playermsg), dstplayer->peer,
                                 k_nSteamNetworkingSend_Reliable);
  }

  NotImplemented("No time functions");
#if 0
    addlinetologfile(gamelog,
                     string(" Net - ") + SpriteSystem::Get().GetSprite(num).player->name + " disconnected " +
                         datetostr(get_date()) + ' ' + timetostr(get_time()),
                     consolelogfilename);
#endif
}

void serverping(std::uint8_t tonum)
{
  tmsg_ping pingmsg;

  pingmsg.header.id = msgid_ping;
  pingmsg.pingticks = SpriteSystem::Get().GetSprite(tonum).player->pingticks;

  if (pingsendcount[tonum] < 8)
    pingsendcount[tonum] += 1;
  else
    pingsendcount[tonum] = 1;
  pingtime[tonum][pingsendcount[tonum]] = GS::GetGame().GetMainTickCounter();

  pingmsg.pingnum = pingsendcount[tonum];

  GetServerNetwork()->SendData(&pingmsg, sizeof(pingmsg),
                               SpriteSystem::Get().GetSprite(tonum).player->peer,
                               k_nSteamNetworkingSend_Reliable);
}
#endif

template <typename T>
std::uint8_t CopyCVarsToBuffer(BitStream &bs, bool fullsync)
{
  std::uint8_t fieldcount = 0;
  // TODO: always sync all variables, make use of fullsync
  for (const auto &v : CVarBase<T>::GetAllCVars())
  {
    if (!v.IsSyncable())
    {
      continue;
    }
    LogDebug("net_msg", "{} id: {} value:{}", v.GetName(), v.GetId(), T(v));
    bs.Write(v.GetId());
    bs.Write(T(v));
    fieldcount++;
  }
  return fieldcount;
}

void serversynccvars(std::uint8_t tonum, HSteamNetConnection peer, bool fullsync)
{
  pmsg_serversynccvars varsmsg;
  std::uint8_t fieldcount = 0;
  std::uint32_t buffersize;

  BitStream bs;
  LogDebug("net_msg", "Write sync variables");

  fieldcount += CopyCVarsToBuffer<std::int32_t>(bs, fullsync);
  fieldcount += CopyCVarsToBuffer<bool>(bs, fullsync);
  fieldcount += CopyCVarsToBuffer<float>(bs, fullsync);
  fieldcount += CopyCVarsToBuffer<std::string>(bs, fullsync);

  buffersize = bs.Data().size();
  auto data = new uint8_t[sizeof(tmsg_serversynccvars) + buffersize];
  varsmsg = new (data) tmsg_serversynccvars();
  varsmsg->itemcount = fieldcount;
  varsmsg->header.id = msgid_synccvars;
  std::memcpy(&varsmsg->data, bs.Data().data(), buffersize);

#ifdef SERVER
  if (peer == 0)
  {
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {

      if ((tonum == 0) || (sprite.num == tonum))
        if (sprite.player->controlmethod == human)
          GetServerNetwork()->SendData(varsmsg, sizeof(tmsg_serversynccvars) + buffersize,
                                       sprite.player->peer, k_nSteamNetworkingSend_Reliable);
    }
  }
  else
  {
    GetServerNetwork()->SendData(varsmsg, sizeof(tmsg_serversynccvars) + buffersize, peer,
                                 k_nSteamNetworkingSend_Reliable);
  }
#else
  GS::GetDemoRecorder().saverecord(varsmsg, sizeof(varsmsg) + buffersize);
#endif
  varsmsg->~tmsg_serversynccvars();
  delete[] data;
}

void servervars(std::uint8_t tonum)
{
  tmsg_servervars varsmsg;
  std::int32_t i;

  std::int32_t weaponindex;

  varsmsg.header.id = msgid_servervars;
  auto &weaponSystem = GS::GetWeaponSystem();

  for (i = 1; i <= main_weapons; i++)
  {
    varsmsg.weaponactive[i - 1] = weaponSystem.IsEnabled(i);
  }

  auto &guns = GS::GetWeaponSystem().GetGuns();

  for (weaponindex = 1; weaponindex <= original_weapons; weaponindex++)
  {
    auto &gun = guns[weaponindex];
    auto weaponidxminus1 = weaponindex - 1;
    varsmsg.damage[weaponidxminus1] = gun.hitmultiply;
    varsmsg.ammo[weaponidxminus1] = gun.ammo;
    varsmsg.reloadtime[weaponidxminus1] = gun.reloadtime;
    varsmsg.speed[weaponidxminus1] = gun.speed;
    varsmsg.bulletstyle[weaponidxminus1] = gun.bulletstyle;
    varsmsg.startuptime[weaponidxminus1] = gun.startuptime;
    varsmsg.bink[weaponidxminus1] = gun.bink;
    varsmsg.fireinterval[weaponidxminus1] = gun.fireinterval;
    varsmsg.movementacc[weaponidxminus1] = gun.movementacc;
    varsmsg.bulletspread[weaponidxminus1] = gun.bulletspread;
    varsmsg.recoil[weaponidxminus1] = gun.recoil;
    varsmsg.push[weaponidxminus1] = gun.push;
    varsmsg.inheritedvelocity[weaponidxminus1] = gun.inheritedvelocity;
    varsmsg.modifierhead[weaponidxminus1] = gun.modifierhead;
    varsmsg.modifierchest[weaponidxminus1] = gun.modifierchest;
    varsmsg.modifierlegs[weaponidxminus1] = gun.modifierlegs;
    varsmsg.nocollision[weaponidxminus1] = gun.nocollision;
  }

#ifdef SERVER
  GetServerNetwork()->SendData(&varsmsg, sizeof(varsmsg),
                               SpriteSystem::Get().GetSprite(tonum).player->peer,
                               k_nSteamNetworkingSend_Reliable);
#else
  GS::GetDemoRecorder().saverecord(varsmsg, sizeof(varsmsg));
#endif
}

#ifdef SERVER
void serverhandlepong(SteamNetworkingMessage_t *netmessage)
{
  tmsg_pong *pongmsg;
  tplayer *player;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_pong), netmessage->m_cbSize, msgid_pong))
    return;

  pongmsg = pmsg_pong(netmessage->m_pData);
  player = GetServerNetwork()->GetPlayer(netmessage);
  i = player->spritenum;

  messagesasecnum[i] += 1;

  if ((pongmsg->pingnum < 1) || (pongmsg->pingnum > 8))
    return;

  SpriteSystem::Get().GetSprite(i).player->pingticks =
    GS::GetGame().GetMainTickCounter() - pingtime[i][pongmsg->pingnum];
  SpriteSystem::Get().GetSprite(i).player->pingtime =
    SpriteSystem::Get().GetSprite(i).player->pingticks * 1000 / 60;

  noclientupdatetime[i] = 0;
}
#endif
