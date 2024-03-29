// automatically converted

#include "Game.hpp"

#ifndef SERVER
#include "../client/Client.hpp"
#include "../client/ClientGame.hpp"
#include "../client/GameMenus.hpp"
#include "../client/GameRendering.hpp"
#include "../client/InterfaceGraphics.hpp"
#include "../client/Sound.hpp"
#else
#include "../server/Server.hpp"
#include "../server/ServerHelper.hpp"
#include "common/Logging.hpp"
#include "shared/network/NetworkServerGame.hpp"
#include "shared/network/NetworkServerSprite.hpp"
#endif
#include "Cvar.hpp"
#include "Demo.hpp"
#include "common/Util.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <chrono>

#ifndef SERVER
std::int32_t gamewidth = default_width;
std::int32_t gameheight = default_height;

float gamewidthhalf = default_width;  // / 2;
float gameheighthalf = default_width; // / 2;
#endif

#ifndef SERVER
PascalArray<tkillsort, 1, max_sprites> sortedteamscore;

std::int32_t heartbeattime, heartbeattimewarnings;
#endif

#ifndef SERVER
PascalArray<tspark, 1, max_sparks> spark; // spark game handling sprite
#endif

namespace
{
// NUMBER27's TIMING ROUTINES
std::chrono::steady_clock::time_point timeinmil,
  timeinmillast; // time in Milliseconds the computer has
// been running
std::chrono::milliseconds timepassed;      // Time in Milliseconds the program has been running
std::chrono::seconds seconds, secondslast; // Seconds the program has been running
} // namespace

using string = std::string;

// Timing routine
template <Config::Module M>
void Game<M>::number27timing()
{
  using namespace std::literals;
  timeinmillast = timeinmil;
  timeinmil = std::chrono::steady_clock::now();
  if (timeinmil - timeinmillast > 2000ms)
  {
    timeinmillast = timeinmil; // safety precaution
  }

  timepassed =
    timepassed + std::chrono::duration_cast<std::chrono::milliseconds>(timeinmil - timeinmillast);
  secondslast = seconds;
  seconds = std::chrono::duration_cast<std::chrono::seconds>(timepassed);

  ticktimelast = ticktime;

  ticktime = timepassed / std::chrono::milliseconds((1000) / goalticks);
}

template <Config::Module M>
void Game<M>::updategamestats()
{
  std::int32_t i;
  tstringlist s;

  // Game Stats save
  if (CVar::log_enable)
  {
    //    try

    s.add("In-Game Statistics");
    s.add(string("Players: ") + inttostr(playersnum));
    s.add(string("Map: ") + map.name);
    switch (CVar::sv_gamemode)
    {
    case gamestyle_deathmatch:
      s.add("Gamemode: Deathmatch");
      break;
    case gamestyle_pointmatch:
      s.add("Gamemode: Pointmatch");
      break;
    case gamestyle_teammatch:
      s.add("Gamemode: Teammatch");
      break;
    case gamestyle_ctf:
      s.add("Gamemode: Capture the Flag");
      break;
    case gamestyle_rambo:
      s.add("Gamemode: Rambomatch");
      break;
    case gamestyle_inf:
      s.add("Gamemode: Infiltration");
      break;
    case gamestyle_htf:
      s.add("Gamemode: Hold the Flag");
      break;
    }
    s.add(string("Timeleft: ") + inttostr(timeleftmin) + ':' + inttostr(timeleftsec));
    if (isteamgame())
    {
      s.add(string("Team 1: ") + inttostr(teamscore[1]));
      s.add(string("Team 2: ") + inttostr(teamscore[2]));
      s.add(string("Team 3: ") + inttostr(teamscore[3]));
      s.add(string("Team 4: ") + inttostr(teamscore[4]));
    }

    s.add("Players list: (name/kills/deaths/team/ping)");
    if (playersnum > 0)
      for (i = 1; i <= playersnum; i++)
      {
        s.add(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player->name);
        s.add(inttostr(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player->kills));
        s.add(inttostr(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player->deaths));
        s.add(inttostr(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player->team));
        s.add(inttostr(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player->realping));
      }
#ifndef SERVER
    s.add("");
    s.add("Server:");
    s.add(joinip + ':' + joinport);
#endif

    s.savetofile(UserDirectory + "logs/gamestat.txt");
    //    finally
    //      FreeAndNil(S);
    //    end;
  }
}

template <Config::Module M>
void Game<M>::togglebullettime(bool turnon, std::int32_t duration)
{
#ifdef SERVER
  LogTraceG("ToggleBulletTime");
#endif

  if (turnon)
  {
    bullettimetimer = duration;
    goalticks = default_goalticks / 3;
  }
  else
    goalticks = default_goalticks;

  number27timing();
}

template <Config::Module M>
bool Game<M>::pointvisible(float x, float y, const std::int32_t i)
{
#ifdef SERVER
  // TODO: check why numbers differ on server and client
  const std::int32_t game_width = max_game_width;
  const std::int32_t game_height = 480;
#else
  std::int32_t game_width;
  std::int32_t game_height;
#endif
  float sx, sy;

#ifdef SERVER
  LogTraceG("PointVisible");
#else
  // workaround because of variables instead of constants
  game_width = gamewidth;
  game_height = gameheight;
#endif

  bool result = false;

  if ((i > max_players) || (i < 1))
    return result;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);

  sx = spritePartsPos.x -
       ((float)((spritePartsPos.x - SpriteSystem::Get().GetSprite(i).control.mouseaimx)) / 2);
  sy = spritePartsPos.y -
       ((float)((spritePartsPos.y - SpriteSystem::Get().GetSprite(i).control.mouseaimy)) / 2);

  if ((x > (sx - game_width)) && (x < (sx + game_width)) && (y > (sy - game_height)) &&
      (y < (sy + game_height)))
    result = true;
  return result;
}

template <Config::Module M>
bool Game<M>::pointvisible2(float x, float y, const std::int32_t i)
{
// TODO: check why numbers differ on server and client
#ifdef SERVER
  const std::int32_t game_width = max_game_width;
  const std::int32_t game_height = 480;
#else
  const std::int32_t game_width = 600;
  const std::int32_t game_height = 440;
#endif
  float sx, sy;

#ifdef SERVER
  LogTraceG("PointVisible2");
#endif

  bool result = false;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);

  sx = spritePartsPos.x;
  sy = spritePartsPos.y;

  if ((x > (sx - game_width)) && (x < (sx + game_width)) && (y > (sy - game_height)) &&
      (y < (sy + game_height)))
    result = true;
  return result;
}

#ifndef SERVER
template <Config::Module M>
bool Game<M>::ispointonscreen(const tvector2 &point)
{
  float p1, p2;

  bool result;
  result = true;
  p1 = gamewidthhalf - (camerax - point.x);
  p2 = gameheighthalf - (cameray - point.y);
  if ((p1 < 0) || (p1 > gamewidth))
    result = false;
  if ((p2 < 0) || (p2 > gameheight))
    result = false;
  return result;
}
#endif

template <Config::Module M>
void Game<M>::startvote(std::uint8_t startervote, std::uint8_t typevote, std::string targetvote,
                        std::string reasonvote)
{
  VoteActive = true;
  if ((startervote < 1) || (startervote > max_players))
    VoteStarter = "Server";
  else
  {
    VoteStarter = SpriteSystem::Get().GetSprite(startervote).player->name;
    VoteCooldown[startervote] = default_vote_time;
#ifndef SERVER
    if (startervote == mysprite)
      if (VoteType == vote_kick)
      {
        GS::GetMainConsole().console(("You have voted to kick ") +
                                   (SpriteSystem::Get().GetSprite(kickmenuindex).player->name) +
                                   (" from the game"),
                                 vote_message_color);
        VoteActive = false;
        NotImplemented("network", "No clientvotekick");
#if 0
                clientvotekick(strtoint(targetvote), true, "");
#endif
      }
#endif
  }
  VoteType = typevote;
  VoteTarget = targetvote;
  VoteReason = reasonvote;
  VoteTimeRemaining = default_voting_time;
  VoteNumVotes = 0;
  VoteMaxVotes = 0;
  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      VoteMaxVotes = VoteMaxVotes + 1;
    }
  }
}

template <Config::Module M>
void Game<M>::stopvote()
{
  VoteActive = false;
  VoteNumVotes = 0;
  VoteMaxVotes = 0;
  VoteType = 0;
  VoteTarget = "";
  VoteStarter = "";
  VoteReason = "";
  VoteTimeRemaining = -1;
  std::fill(std::begin(VoteHasVoted), std::end(VoteHasVoted), false);
}

template <Config::Module M>
void Game<M>::timervote()
{
#ifdef SERVER
  if (VoteActive)
  {
#endif
    if (VoteTimeRemaining > -1)
      VoteTimeRemaining = VoteTimeRemaining - 1;

    if (VoteTimeRemaining == 0)
    {
      if (VoteType == vote_map)
        GS::GetMainConsole().console(
#ifdef SERVER
          "No map has been voted",
#else
        ("No map has been voted"),
#endif
          vote_message_color);
      stopvote();
    }
#ifdef SERVER
  }
#endif
}

#ifdef SERVER
template <Config::Module M>
void Game<M>::countvote(std::uint8_t voter)
{
  std::int32_t i;
  float edge;
  // Status: TMapInfo;

  if (VoteActive && !VoteHasVoted[voter])
  {
    VoteNumVotes = VoteNumVotes + 1;
    VoteHasVoted[voter] = true;
    edge = (float)(VoteNumVotes) / VoteMaxVotes;
    if (edge >= ((float)(CVar::sv_votepercent) / 100))
    {
      if (VoteType == vote_kick)
      {
        i = strtoint(VoteTarget);
        // There should be no permanent bans by votes. Reduced to 1 day.
        if (cheattag[i] == 0)
          kickplayer(i, true, kick_voted, hour, "Vote Kicked");
        else
          kickplayer(i, true, kick_voted, day, "Vote Kicked by Server");
        dobalancebots(1, SpriteSystem::Get().GetSprite(i).player->team);
      }
      else if (VoteType == vote_map)
      {
        if (!preparemapchange(VoteTarget))
        {
          GS::GetMainConsole().console(string("Map not found (") + VoteTarget + ')',
                                   warning_message_color);
          GS::GetMainConsole().console("No map has been voted", vote_message_color);
        }
      }
      stopvote();
      serversendvoteoff();
    }
  }
}
#endif

template <Config::Module M>
void Game<M>::showmapchangescoreboard()
{
  showmapchangescoreboard("EXIT*!*");
}

template <Config::Module M>
void Game<M>::showmapchangescoreboard(const std::string nextmap)
{
  mapchangename = nextmap;
  mapchangecounter = mapchangetime;
#ifndef SERVER
  gamemenushow(limbomenu, false);
  fragsmenushow = true;
  statsmenushow = false;
  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    stopsound(sprite.reloadsoundchannel);
    stopsound(sprite.jetssoundchannel);
    stopsound(sprite.gattlingsoundchannel);
    stopsound(sprite.gattlingsoundchannel2);
  }
#endif
}

template <Config::Module M>
bool Game<M>::isteamgame()
{
  bool isteamgame_result;
  switch (CVar::sv_gamemode)
  {
  case gamestyle_teammatch:
  case gamestyle_ctf:
  case gamestyle_inf:
  case gamestyle_htf:
    isteamgame_result = true;
    break;
  default:
    isteamgame_result = false;
  }
  return isteamgame_result;
}

template <Config::Module M>
void Game<M>::changemap()
{
#ifdef SERVER
  tvector2 a;
#endif
  std::int32_t j;
  std::int32_t secwep;
#ifndef SERVER
  tmapinfo mapchangestatus;
#endif

#ifdef SERVER
  //  try
  LogDebugG("ChangeMap");
  mapslist.clear();

  if (fileexists(UserDirectory + std::string(CVar::sv_maplist)))
  {
    mapslist.loadfromfile(UserDirectory + std::string(CVar::sv_maplist));
    auto it = std::remove(mapslist.begin(), mapslist.end(), "");
    mapslist.erase(it, mapslist.end());
  }

  for (auto &w : botpath.waypoint)
  {
    w.active = false;
    w.id = 0;
    w.pathnum = 0;
  }

  if (!map.loadmap(GS::GetFileSystem(), mapchange))
  {
    GS::GetMainConsole().console(string("Error: Could not load map (") + mapchange.name + ')',
                             debug_message_color);
    nextmap();
    return;
  }
#endif
#ifndef SERVER
  mapchanged = true;
  GS::GetDemoRecorder().stoprecord();
  auto& fs = GS::GetFileSystem();

  if (getmapinfo(fs, mapchangename, UserDirectory, mapchangestatus) &&
      verifymapchecksum(fs, mapchangestatus, mapchangechecksum, gamemodchecksum))
  {
    if (!map.loadmap(fs, mapchangestatus, CVar::r_forcebg, CVar::r_forcebg_color1,
                     CVar::r_forcebg_color2))
    {
      rendergameinfo(("Could not load map: ") + (mapchangename));
      exittomenu();
      return;
    }
    // Map.Name := MapChangeName;
  }
  else
  {
    exittomenu();
    joinserver();
    return;
  }
#endif

  GS::GetBulletSystem().KillAll();
  GS::GetThingSystem().KillAll();
#ifndef SERVER
  for (auto &s : spark)
  {
    s.kill();
  }
#endif

  {
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if (sprite.isnotspectator())
      {
        auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(sprite.num);
        randomizestart(spritePartsPos, sprite.player->team);
        sprite.respawn();
        sprite.player->kills = 0;
        sprite.player->deaths = 0;
        sprite.player->flags = 0;
        sprite.bonustime = 0;
        sprite.bonusstyle = bonus_none;
#ifndef SERVER
        sprite.selweapon = 0;
#endif
        sprite.freecontrols();
        sprite.SetFirstWeapon(GS::GetWeaponSystem().GetGuns()[noweapon]);

        secwep = sprite.player->secwep + 1;

        auto &weaponSystem = GS::GetWeaponSystem();
        if ((secwep >= 1) && (secwep <= secondary_weapons) &&
            (weaponSystem.IsEnabled(primary_weapons + secwep)))
        {
          sprite.SetSecondWeapon(GS::GetWeaponSystem().GetGuns()[primary_weapons + secwep]);
        }
        else
        {
          sprite.SetSecondWeapon(GS::GetWeaponSystem().GetGuns()[noweapon]);
        }

        sprite.respawncounter = 0;
      }
    }
  }

#ifndef SERVER
  for (auto &w : weaponsel)
  {
    for (auto i = 1; i <= primary_weapons; i++)
    {
      w[i] = 1;
    }
  }
#endif

  if (CVar::sv_advancemode)
  {
    for (auto &w : weaponsel)
    {
      for (auto i = 1; i <= primary_weapons; i++)
      {
        w[i] = 0;
      }
    }

#ifndef SERVER
    if (mysprite > 0)
      for (auto i = 1; i <= main_weapons; i++)
        limbomenu->button[i - 1].active = (bool)(weaponsel[mysprite][i]);
#endif
  }

  for (auto i = 1; i <= 4; i++)
  {
    teamscore[i] = 0;
  }

  for (auto i = 1; i <= 2; i++)
  {
    teamflag[i] = 0;
  }

#ifndef SERVER
  fragsmenushow = false;
  statsmenushow = false;

  if (mysprite > 0)
    gamemenushow(limbomenu);
#endif

#ifdef SERVER
  // add yellow flag
  if ((CVar::sv_gamemode == gamestyle_pointmatch) || (CVar::sv_gamemode == gamestyle_htf))
  {
    randomizestart(a, 14);
    teamflag[1] = creatething(a, 255, object_pointmatch_flag, 255);
  }

  if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf))
  {
    // red flag
    if (randomizestart(a, 5))
      teamflag[1] = creatething(a, 255, object_alpha_flag, 255);

    // blue flag
    if (randomizestart(a, 6))
      teamflag[2] = creatething(a, 255, object_bravo_flag, 255);
  }

  if (CVar::sv_gamemode == gamestyle_rambo)
  {
    randomizestart(a, 15);
    creatething(a, 255, object_rambo_bow, 255);
  }

  if (!CVar::sv_survivalmode)
  {
    // spawn medikits
    spawnthings(object_medical_kit, map.medikits);

    // spawn grenadekits
    if (CVar::sv_maxgrenades > 0)
      spawnthings(object_grenade_kit, map.grenades);
  }

  // stat gun
  if (CVar::sv_stationaryguns)
  {
    for (auto &s : map.spawnpoints)
    {
      if (!s.active)
      {
        continue;
      }
      if (s.team != 16)
      {
        continue;
      }
      a.x = s.x;
      a.y = s.y;
      creatething(a, 255, object_stationary_gun, 255);
    }
  }
#endif
#ifndef SERVER
  heartbeattime = maintickcounter;
  heartbeattimewarnings = 0;

  if ((mysprite > 0) && SpriteSystem::Get().GetSprite(mysprite).isnotspectator())
  {
    camerafollowsprite = mysprite;
  }
  else
  {
    // If in freecam or the previous followee is gone, then find a new followee
    if ((camerafollowsprite == 0) or !SpriteSystem::Get().GetSprite(camerafollowsprite).IsActive())
    {
      camerafollowsprite = getcameratarget(0);
      // If no appropriate player found, then just center the camera
      if (camerafollowsprite == 0)
      {
        camerax = 0;
        cameray = 0;
      }
    }
  }

  if (!escmenu->active)
  {
    mx = gamewidthhalf;
    my = gameheighthalf;
    mouseprev.x = mx;
    mouseprev.y = my;
  }

  // Spawn sound
  if (mysprite > 0)
  {
    auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(mysprite);
    playsound(SfxEffect::spawn, spritePartsPos);
  }

#endif
  // DEMO
  if (CVar::demo_autorecord)
  {
    if (GS::GetDemoRecorder().active())
      GS::GetDemoRecorder().stoprecord();

    NotImplemented("no current time function");
#if 0
        GS::GetDemoRecorder().startrecord(userdirectory + "demos/" +
                                 formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + ".sdm");
#endif
  }

  sortplayers();

  mapchangecounter = -60;

  timelimitcounter = CVar::sv_timelimit;

#ifdef SCRIPT
  scrptdispatcher.onaftermapchange(map.name);
#endif
#ifdef SERVER
  serverspritesnapshotmajor(netw);
  //  except
  //    on e: exception do
  //      WriteLn('Error changing map ' + e.message);
  //  end;
#endif
}

template <Config::Module M>
void Game<M>::sortplayers()
{
  std::int32_t j;
  tkillsort temp;

  playersnum = 0;
  botsnum = 0;
  spectatorsnum = 0;
  std::fill(std::begin(playersteamnum), std::end(playersteamnum), 0);

  for (auto &sp : sortedplayers)
  {
    sp.kills = 0;
    sp.deaths = 0;
    sp.flags = 0;
    sp.playernum = 0;
  }

  {
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if (!sprite.player->demoplayer)
      {
        playersnum += 1;
        if (sprite.player->controlmethod == bot)
          botsnum += 1;

        if (sprite.isspectator())
          spectatorsnum += 1;

        if (sprite.isnotsolo() && sprite.isnotspectator())
          playersteamnum[sprite.player->team] += 1;

        if (sprite.isnotspectator())
        {
          sortedplayers[playersnum].kills = sprite.player->kills;
          sortedplayers[playersnum].deaths = sprite.player->deaths;
          sortedplayers[playersnum].flags = sprite.player->flags;
          sortedplayers[playersnum].playernum = sprite.num;
        }
        else
        {
          sortedplayers[playersnum].kills = 0;
          sortedplayers[playersnum].deaths = std::numeric_limits<std::int32_t>::max();
          sortedplayers[playersnum].flags = 0;
          sortedplayers[playersnum].playernum = sprite.num;
        }

        // Kill Limit
        if (mapchangecounter < 1)
          if (!isteamgame())
            if (sprite.player->kills >= CVar::sv_killlimit)
            {
#ifndef SERVER
              camerafollowsprite = sprite.num;
              if (escmenu->active)
              {
                mx = gamewidthhalf;
                my = gameheighthalf;
                mouseprev.x = mx;
                mouseprev.y = my;
              }
#else
              nextmap();
#endif
            }
      }
    }
  }

  // sort by caps first if new score board
  if (playersnum > 0)
    for (auto i = 1; i <= playersnum; i++)
    {
      for (j = i + 1; j <= playersnum; j++)
      {
        if (sortedplayers[j].flags > sortedplayers[i].flags)
        {
          temp = sortedplayers[i];
          sortedplayers[i] = sortedplayers[j];
          sortedplayers[j] = temp;
        }
      }
    }

  // sort by kills
  if (playersnum > 0)
    for (auto i = 1; i <= playersnum; i++)
      for (j = i + 1; j <= playersnum; j++)
        if (sortedplayers[j].flags == sortedplayers[i].flags)
          if (sortedplayers[j].kills > sortedplayers[i].kills)
          {
            temp = sortedplayers[i];
            sortedplayers[i] = sortedplayers[j];
            sortedplayers[j] = temp;
          } // if

  // final sort by deaths
  if (playersnum > 0)
    for (auto i = 1; i <= playersnum; i++)
      for (j = i + 1; j <= playersnum; j++)
        if (sortedplayers[j].flags == sortedplayers[i].flags)
          if (sortedplayers[j].kills == sortedplayers[i].kills)
            if (sortedplayers[j].deaths < sortedplayers[i].deaths)
            {
              temp = sortedplayers[i];
              sortedplayers[i] = sortedplayers[j];
              sortedplayers[j] = temp;
            }

#ifndef SERVER
  // Sort Team Score
  for (auto i = 1; i <= 4; i++)
  {
    sortedteamscore[i].kills = teamscore[i];
    sortedteamscore[i].playernum = i;
  }

  sortedteamscore[1].color = (std::uint32_t(CVar::ui_status_transparency) << 24) | 0xd20f05; // ARGB
  sortedteamscore[2].color = (std::uint32_t(CVar::ui_status_transparency) << 24) | 0x50fd2;
  sortedteamscore[3].color = (std::uint32_t(CVar::ui_status_transparency) << 24) | 0xd2d205;
  sortedteamscore[4].color = (std::uint32_t(CVar::ui_status_transparency) << 24) | 0x5d205;

  for (auto i = 1; i <= 4; i++)
    for (j = i + 1; j <= 4; j++)
      if (sortedteamscore[j].kills > sortedteamscore[i].kills)
      {
        temp = sortedteamscore[i];
        sortedteamscore[i] = sortedteamscore[j];
        sortedteamscore[j] = temp;
      }
#endif

#ifdef SERVER
  // Team - Kill Limit
  if (mapchangecounter < 1)
  {
    for (auto i = 1; i <= 4; i++)
    {
      if (teamscore[i] >= CVar::sv_killlimit)
      {
        nextmap();
        break;
      }
    }
  }
  // Wave respawn time
  updatewaverespawntime();
#endif

#ifndef SERVER
  teamplayersnum[0] = 0;
  teamplayersnum[1] = 0;
  teamplayersnum[2] = 0;
  teamplayersnum[3] = 0;
  teamplayersnum[4] = 0;
#else
  teamalivenum[1] = 0;
  teamalivenum[2] = 0;
  teamalivenum[3] = 0;
  teamalivenum[4] = 0;
#endif

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (team_alpha >= sprite.player->team && sprite.player->team <= team_delta)
#ifdef SERVER
      teamalivenum[sprite.player->team] += 1;
#else
      teamplayersnum[sprite.player->team] += 1;
#endif
  }
}

template <Config::Module M>
void Game<M>::TickVote()
{
  for (auto j = 1; j <= max_sprites; j++)
  {
    if (VoteCooldown[j] > -1)
    {
      VoteCooldown[j] = VoteCooldown[j] - 1;
    }
  }
}

template <Config::Module M>
void Game<M>::CalculateTeamAliveNum(int32_t player)
{
  teamalivenum[1] = 0;
  teamalivenum[2] = 0;
  teamalivenum[3] = 0;
  teamalivenum[4] = 0;

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (!sprite.deadmeat and (sprite.player->team == team_alpha))
      teamalivenum[team_alpha] += 1;
    if (!sprite.deadmeat and (sprite.player->team == team_bravo))
      teamalivenum[team_bravo] += 1;
    if (!sprite.deadmeat and (sprite.player->team == team_charlie))
      teamalivenum[team_charlie] += 1;
    if (!sprite.deadmeat and (sprite.player->team == team_delta))
      teamalivenum[team_delta] += 1;
  }

  teamalivenum[player] -= 1;

  alivenum = teamalivenum[1] + teamalivenum[2] + teamalivenum[3] + teamalivenum[4];

  if (((teamalivenum[1] > 0) && (teamalivenum[2] < 1) && (teamalivenum[3] < 1) &&
       (teamalivenum[4] < 1)) ||
      ((teamalivenum[2] > 0) && (teamalivenum[1] < 1) && (teamalivenum[3] < 1) &&
       (teamalivenum[4] < 1)) ||
      ((teamalivenum[3] > 0) && (teamalivenum[1] < 1) && (teamalivenum[2] < 1) &&
       (teamalivenum[4] < 1)) ||
      ((teamalivenum[4] > 0) && (teamalivenum[1] < 1) && (teamalivenum[2] < 1) &&
       (teamalivenum[3] < 1)) ||
      ((teamalivenum[1] < 1) && (teamalivenum[2] < 1) && (teamalivenum[3] < 1) &&
       (teamalivenum[4] < 1)))
  {
    auto &activeSprites = SpriteSystem::Get().GetActiveSprites();
    std::for_each(std::begin(activeSprites), std::end(activeSprites),
                  [](auto &sprite) { sprite.respawncounter = survival_respawntime; });

    if (!survivalendround)
      if (CVar::sv_gamemode == gamestyle_ctf)
      {
        if (teamalivenum[1] > 0)
          teamscore[1] += 1;
        if (teamalivenum[2] > 0)
          teamscore[2] += 1;
      }
    if (!survivalendround)
      if (CVar::sv_gamemode == gamestyle_inf)
      {
        if (teamalivenum[1] > 0)
          teamscore[1] += CVar::sv_inf_redaward;

        // penalty
        if (playersteamnum[1] > playersteamnum[2])
          teamscore[1] -= 5 * (playersteamnum[1] - playersteamnum[2]);
        if (teamscore[1] < 0)
          teamscore[1] = 0;
      }

    survivalendround = true;

    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if (!sprite.deadmeat)
      {
        sprite.idlerandom = 5;
        sprite.idletime = 1;
      }
    }
  }
}

template <Config::Module M>
Game<M>::Game() : map(botpath)
{
}

template class Game<>;
