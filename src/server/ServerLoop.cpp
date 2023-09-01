// automatically converted

#include "ServerLoop.hpp"
#include "BanSystem.hpp"
#include "Server.hpp"
#include "ServerHelper.hpp"
#include "common/Logging.hpp"
#include "shared/Console.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/LogFile.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkServer.hpp"
#include "shared/network/NetworkServerConnection.hpp"
#include "shared/network/NetworkServerGame.hpp"
#include "shared/network/NetworkServerHeartbeat.hpp"
#include "shared/network/NetworkServerSprite.hpp"
#include "shared/network/NetworkServerThing.hpp"
#include <Tracy.hpp>

using string = std::string;

void apponidle()
{
  ZoneScopedN("AppOnIdle");
  std::int32_t maincontrol;
  std::int32_t j;
  std::int32_t heavysendersnum;
  float adjust;

  // LogTraceG("AppOnIdle");

  auto &game = GS::GetGame();
  game.number27timing(); // makes the program go and do the timing calculations

  // NET RECEIVE
  GetServerNetwork()->ProcessLoop();

#ifdef RCON
  if (adminserver != nullptr)
    adminserver.processcommands(0);
#endif

  for (maincontrol = 1; maincontrol <= (game.GetTickTime() - game.GetTickTimeLast()); maincontrol++)
  { // frame rate independant code

    servertickcounter += 1;
    // Update main tick counter
    GS::GetGame().TickMainTickCounter();
    if (GS::GetGame().GetMainTickCounter() == 2147483640)
    {
      GS::GetGame().ResetMainTickCounter();
    }

#ifdef SCRIPT
    scrptdispatcher.onclocktick(0);
#endif

#ifdef STEAM
    runmanualcallbacks(0);
#endif
    // Flood Nums Cancel
    if (GS::GetGame().GetMainTickCounter() % 1000 == 0)
    {
      std::fill(std::begin(floodnum), std::end(floodnum), 0);
    }

    // clear last admin connect flood list every 3 seconds
    if (GS::GetGame().GetMainTickCounter() % (second * 3) == 0)
    {
      std::fill(std::begin(lastadminips), std::end(lastadminips), "");
    }

    // Warnings Cancel
    if (GS::GetGame().GetMainTickCounter() % (minute * 5) == 0)
    {
      for (auto &p : pingwarnings)
      {
        if (p > 0)
        {
          p -= 1;
        }
      }
      for (auto &f : floodwarnings)
      {
        if (f > 0)
        {
          f -= 1;
        }
      }
    }

    if (GS::GetGame().GetMainTickCounter() % 1000 == 0)
    {
      for (auto &s : SpriteSystem::Get().GetSprites())
      {
        s.player->knifewarnings = 0;
      }
    }
    NotImplemented("network");
#if 0
        // sync changed cvars to all players
        if (cvarsneedsyncing)
            serversynccvars(0, 0, false);
#endif

    // General game updating
    updateframe();

    if (GS::GetGame().GetMapchangecounter() < 0)
    {
      if (GS::GetDemoRecorder().active())
        GS::GetDemoRecorder().savenextframe();
    }

    LogTraceG("AppOnIdle 2");

    // Network updating
    if (GS::GetGame().GetMainTickCounter() % second == 0)
    {
      // Player Ping Warning
      if (GS::GetGame().GetMapchangecounter() < 0)
      {
        if (GS::GetGame().GetMainTickCounter() % (second * 6) == 0)
        {
          for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
          {
            if ((sprite.player->controlmethod == human) &&
                ((sprite.player->realping > (CVar::sv_maxping)) ||
                 ((sprite.player->realping < CVar::sv_minping) && (sprite.player->pingtime > 0))))
            {
              GS::GetMainConsole().console(sprite.player->name + " gets a ping warning",
                                             warning_message_color);
              pingwarnings[sprite.num] += 1;
              if (pingwarnings[sprite.num] > CVar::sv_warnings_ping)
              {
                kickplayer(sprite.num, true, kick_ping, sixty_minutes / 4, "Ping Kick");
              }
            }
          }
        }
      }

      // Player Packet Flooding
      for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
      {
        j = sprite.num;
        if (((CVar::net_lan == LAN) && (messagesasecnum[j] > CVar::net_floodingpacketslan)) ||
            ((CVar::net_lan == INTERNET) &&
             (messagesasecnum[j] > CVar::net_floodingpacketsinternet)))
        {
          GS::GetMainConsole().console(sprite.player->name + " is flooding the server",
                                         warning_message_color);
          floodwarnings[j] += 1;
          if (floodwarnings[j] > CVar::sv_warnings_flood)
            kickplayer(j, true, kick_flooding, sixty_minutes / 4, "Flood Kicked");
        }
      }

      std::fill(std::begin(messagesasecnum), std::end(messagesasecnum), 0);
    }

    if (GS::GetGame().GetMainTickCounter() % (second * 10) == 0)
    {
      for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
      {
        GetServerNetwork()->UpdateNetworkStats(sprite.num);
      }
    }

    // Packet rate send adjusting
    heavysendersnum = GS::GetGame().GetPlayersNum() - GS::GetGame().GetSpectatorsNum();

    if (heavysendersnum < 5)
      adjust = 0.66;
    else if (heavysendersnum < 9)
      adjust = 0.75;
    else
      adjust = 1.0;

    // Send Bundled packets
    if (CVar::net_lan == LAN)
    {
      if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(30 * adjust) == 0)
        serverspritesnapshot(netw);

      if ((GS::GetGame().GetMainTickCounter() % (std::int32_t)round(15 * adjust) == 0) &&
          (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(30 * adjust) != 0))
        serverspritesnapshotmajor(netw);

      if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(20 * adjust) == 0)
        serverskeletonsnapshot(netw);

      if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(59 * adjust) == 0)
        serverheartbeat();

      if ((GS::GetGame().GetMainTickCounter() % (std::int32_t)round(4 * adjust) == 0) &&
          (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(30 * adjust) != 0) &&
          (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(60 * adjust) != 0))
      {
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {
          if (sprite.player->controlmethod == bot)
          {
            serverspritedeltas(sprite.num);
          }
        }
      }
    }
    else if (CVar::net_lan == INTERNET)
    {
      if (GS::GetGame().GetMainTickCounter() %
            (std::int32_t)round(CVar::net_t1_snapshot * adjust) ==
          0)
        serverspritesnapshot(netw);

      if ((GS::GetGame().GetMainTickCounter() %
             (std::int32_t)round(CVar::net_t1_majorsnapshot * adjust) ==
           0) &&
          (GS::GetGame().GetMainTickCounter() %
             (std::int32_t)round(CVar::net_t1_snapshot * adjust) !=
           0))
        serverspritesnapshotmajor(netw);

      if (GS::GetGame().GetMainTickCounter() %
            (std::int32_t)round(CVar::net_t1_deadsnapshot * adjust) ==
          0)
        serverskeletonsnapshot(netw);

      if (GS::GetGame().GetMainTickCounter() %
            (std::int32_t)round(CVar::net_t1_heartbeat * adjust) ==
          0)
        serverheartbeat();

      if ((GS::GetGame().GetMainTickCounter() % (std::int32_t)round(CVar::net_t1_delta * adjust) ==
           0) &&
          (GS::GetGame().GetMainTickCounter() %
             (std::int32_t)round(CVar::net_t1_snapshot * adjust) !=
           0) &&
          (GS::GetGame().GetMainTickCounter() %
             (std::int32_t)round(CVar::net_t1_majorsnapshot * adjust) !=
           0))
      {
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {
          if (sprite.player->controlmethod == bot)
          {
            serverspritedeltas(sprite.num);
          }
        }
      }
    }

    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if ((sprite.player->controlmethod == human) && (sprite.player->port > 0))
      {
        j = sprite.num;
        // connection problems
        if (GS::GetGame().GetMapchangecounter() < 0)
          noclientupdatetime[j] = noclientupdatetime[j] + 1;
        if (noclientupdatetime[j] > disconnection_time)
        {
          serverplayerdisconnect(j, kick_noresponse);
          GS::GetMainConsole().console(sprite.player->name + " could not respond",
                                         warning_message_color);
#ifdef SCRIPT
          scrptdispatcher.onleavegame(j, false);
#endif
          sprite.kill();
          dobalancebots(1, sprite.player->team);
          continue;
        }
        if (noclientupdatetime[j] < 0)
          noclientupdatetime[j] = 0;

#ifdef ENABLE_FAE
        if (CVar::ac_enable)
        {
          // Monotonically increment the anti-cheat ticks counter. A valid response resets
          // it.
          SpriteSystem::Get().GetSprite(j).player->faeticks += 1;
          if (SpriteSystem::Get().GetSprite(j).player->faeticks > (second * 20))
          {
            // Timeout reached; no valid response for 20 seconds. Boot the player.
            GS::GetMainConsole().console(SpriteSystem::Get().GetSprite(j).player->name +
                                             " no anti-cheat response",
                                           warning_message_color);
            kickplayer(j, false, kick_ac, 0, "No Anti-Cheat Response");
            continue;
          }
          else if ((GS::GetGame().GetMainTickCounter() % (second * 3) == 0) and
                   (!SpriteSystem::Get().GetSprite(j).player->faeresponsepending))
          {
            // Send periodic anti-cheat challenge.
            serversendfaechallenge(SpriteSystem::Get().GetSprite(j).player->peer, false);
          }
        }
#endif

        if (GS::GetGame().GetMainTickCounter() % minute == 0)
          serversyncmsg();

        if (CVar::net_lan == LAN)
        {
          if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(21 * adjust) == 0)
            serverping(j);

          if (GS::GetGame().GetMainTickCounter() % (std::int32_t)round(12 * adjust) == 0)
            serverthingsnapshot(j);
        }
        else if (CVar::net_lan == INTERNET)
        {
          if (GS::GetGame().GetMainTickCounter() %
                (std::int32_t)round(CVar::net_t1_ping * adjust) ==
              0)
            serverping(j);

          if (GS::GetGame().GetMainTickCounter() %
                (std::int32_t)round(CVar::net_t1_thingsnapshot * adjust) ==
              0)
            serverthingsnapshot(j);
        }
      }
    }
    // UDP.FlushMsg;
  }
}

void updateframe()
{
  ZoneScopedN("UpdateFrame");
  std::int32_t i, j;
  tvector2 m;
  std::uint32_t _x;

  LogTraceG("UpdateFrame");
  auto &things = GS::GetThingSystem().GetThings();

  if (GS::GetGame().GetMapchangecounter() < 0)
  {
    {
      ZoneScopedN("SpriteParts");
      SpriteSystem::Get().UpdateSpriteParts();
    }

    {
      ZoneScopedN("UpdateSprites");
      auto &activeSprites = SpriteSystem::Get().GetActiveSprites();
      std::for_each(std::begin(activeSprites), std::end(activeSprites),
                    [](auto &sprite) { sprite.update(); });
    }

    {
      ZoneScopedN("Bullets");
      // Bullets update
      auto &bullet = GS::GetBulletSystem().GetBullets();
      for (j = 1; j <= max_bullets; j++)
        if (bullet[j].active)
          bullet[j].update();

      GetBulletParts().doeulertimestep();
    }

    {
      ZoneScopedN("Things");
      // update Things
      for (j = 1; j <= max_things; j++)
        if (things[j].active)
          things[j].update();
    }

    // Bonuses spawn
    if ((!CVar::sv_survivalmode) && (!CVar::sv_realisticmode))
    {
      if (CVar::sv_bonus_frequency > 0)
      {
        switch (CVar::sv_bonus_frequency)
        {
        case 1:
          bonusfreq = 7400;
          break;
        case 2:
          bonusfreq = 4300;
          break;
        case 3:
          bonusfreq = 2500;
          break;
        case 4:
          bonusfreq = 1600;
          break;
        case 5:
          bonusfreq = 800;
          break;
        }

        if (CVar::sv_bonus_berserker)
          if (GS::GetGame().GetMainTickCounter() % bonusfreq == 0)
            if (Random(berserkerbonus_random) == 0)
              spawnthings(object_berserk_kit, 1);

        j = flamerbonus_random;
        if (CVar::sv_bonus_flamer)
          if (GS::GetGame().GetMainTickCounter() % 444 == 0)
            if (Random(j) == 0)
              spawnthings(object_flamer_kit, 1);

        if (CVar::sv_bonus_predator)
          if (GS::GetGame().GetMainTickCounter() % bonusfreq == 0)
            if (Random(predatorbonus_random) == 0)
              spawnthings(object_predator_kit, 1);

        if (CVar::sv_bonus_vest)
          if (GS::GetGame().GetMainTickCounter() % (bonusfreq / 2) == 0)
            if (Random(vestbonus_random) == 0)
              spawnthings(object_vest_kit, 1);

        j = clusterbonus_random;
        if (CVar::sv_gamemode == gamestyle_ctf)
          j = round(clusterbonus_random * 0.75);
        if (CVar::sv_gamemode == gamestyle_inf)
          j = round(clusterbonus_random * 0.75);
        if (CVar::sv_gamemode == gamestyle_htf)
          j = round(clusterbonus_random * 0.75);
        if (CVar::sv_bonus_cluster)
          if (GS::GetGame().GetMainTickCounter() % (bonusfreq / 2) == 0)
            if (Random(j) == 0)
              spawnthings(object_cluster_kit, 1);
      }
    }
  }

  // bullet timer
  GS::GetGame().TickBulletTimeTimer();

  if (GS::GetGame().GetBulletTimeTimer() == 0)
  {
    GS::GetGame().togglebullettime(false);
    GS::GetGame().TickBulletTimeTimer();
  }
  else if (GS::GetGame().GetBulletTimeTimer() < 1)
  {
    // MapChange counter update
    if ((GS::GetGame().GetMapchangecounter() > -60) &&
        (GS::GetGame().GetMapchangecounter() < 99999999))
      GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 1);
    if ((GS::GetGame().GetMapchangecounter() < 0) && (GS::GetGame().GetMapchangecounter() > -59))
      GS::GetGame().changemap();

    // Game Stats save
    if (GS::GetGame().GetMainTickCounter() % CVar::log_filesupdate == 0)
    {
      if (CVar::log_enable)
      {
        GS::GetGame().updategamestats();
        auto& fs = GS::GetFileSystem();

        writelogfile(fs, &GetKillLog(), GetKillLogFilename());
        writelogfile(fs, GetGameLog(), GetGameLogFilename());

        if ((fs.Size(GetKillLogFilename()) > max_logfilesize) ||
            (fs.Size(GetGameLogFilename()) > max_logfilesize))
        {
          newlogfiles(fs);
        }
      }
    }

    // Anti-Hack for Mass-Flag Cheat
    if (GS::GetGame().GetMainTickCounter() % second == 0)
    {
      if (CVar::sv_antimassflag)
      {
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {

          if ((sprite.player->grabspersecond > 0) && (sprite.player->scorespersecond > 0) &&
              (sprite.player->grabbedinbase))
          {
            j = sprite.num;
            cheattag[j] = 1;
#ifdef SCRIPT
            if (!scrptdispatcher.onvotekickstart(255, j, "Server: Possible cheating"))
            {
#endif
              GS::GetGame().startvote(255, vote_kick, inttostr(j), "Server: Possible cheating");
              serversendvoteon(GS::GetGame().GetVoteType(), 255, inttostr(j),
                               "Server: Possible cheating");
#ifdef SCRIPT
            }
#endif
            GS::GetMainConsole().console(string("** Detected possible Mass-Flag cheating from ") +
                                             sprite.player->name,
                                           warning_message_color);
          }
          sprite.player->grabspersecond = 0;
          sprite.player->scorespersecond = 0;
          sprite.player->grabbedinbase = false;
        }
      }
    }
    if (CVar::sv_healthcooldown > 0)
    {
      if (GS::GetGame().GetMainTickCounter() % (CVar::sv_healthcooldown * second) == 0)
      {
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {
          if (sprite.haspack)
          {
            sprite.haspack = false;
          }
        }
      }
    }

    // Anti-Chat Flood
    if (GS::GetGame().GetMainTickCounter() % second == 0)
    {
      for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
      {
        if (sprite.player->chatwarnings > 5)
        {
          // 20 Minutes is too harsh
          kickplayer(sprite.num, true, kick_flooding, five_minutes, "Chat Flood");
        }
        if (sprite.player->chatwarnings > 0)
        {
          sprite.player->chatwarnings -= 1;
        }
      }
    }

    if (GS::GetGame().GetMainTickCounter() % second == 0)
      if ((lastreqip[0] != "") && (lastreqip[0] == lastreqip[1]) &&
          (lastreqip[1] == lastreqip[2]) && (lastreqip[2] == lastreqip[3]))
      {
        dropip = lastreqip[0];
        GS::GetMainConsole().console(string("Firewalled IP ") + dropip, 0);
      }

    if (GS::GetGame().GetMainTickCounter() % (second * 3) == 0)
      for (j = 0; j <= 3; j++)
        lastreqip[j] = ""; // Reset last 4 IP requests in 3 seconds

    if (GS::GetGame().GetMainTickCounter() % (second * 30) == 0)
      dropip = ""; // Clear temporary firewall IP

    if (GS::GetGame().GetMainTickCounter() % minute == 0)
    {
      NotImplemented("network");
#if 0
            if (CVar::sv_lobby)
                lobbythread = tlobbythread.create;
#endif
    }

    // *BAN*
    // Ban Timers v2
    if (GS::GetGame().GetMainTickCounter() % minute == 0)
    {
      updateipbanlist();
      updatehwbanlist();
    }

    if (GS::GetGame().GetMainTickCounter() % minute == 0)
    {
      auto &activeSprites = SpriteSystem::Get().GetActiveSprites();
      std::for_each(std::begin(activeSprites), std::end(activeSprites),
                    [](auto &sprite) { sprite.player->playtime += 1; });
    }

    // Leftover from old Ban Timers code
    if (GS::GetGame().GetMainTickCounter() % (second * 10) == 0)
      if (GS::GetGame().GetPlayersNum() == 0)
        if (GS::GetGame().GetMapchangecounter() > 99999999)
          GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);

    {
      auto v = GS::GetGame().GetSinusCounter() + iluminatespeed;
      GS::GetGame().SetSinusCounter(v);
    }

    // Wave respawn count
    waverespawncounter = waverespawncounter - 1;
    if (waverespawncounter < 1)
      waverespawncounter = waverespawntime;

    GS::GetGame().TickVote();

    // Time Limit decrease
    if (GS::GetGame().GetMapchangecounter() < 99999999)
      // if (MapChangeCounter<0) then
      if (GS::GetGame().GetTimelimitcounter() > 0)
        GS::GetGame().SetTimelimitcounter(GS::GetGame().GetTimelimitcounter() - 1);
    if (GS::GetGame().GetTimelimitcounter() == 1)
    {
      nextmap();
    }

    GS::GetGame().SetTimeleftmin(GS::GetGame().GetTimelimitcounter() / minute);
    GS::GetGame().SetTimeleftsec(
      (GS::GetGame().GetTimelimitcounter() - GS::GetGame().GetTimeleftmin() * minute) / 60);

    if (GS::GetGame().GetTimelimitcounter() > 0)
    {
      if (GS::GetGame().GetTimelimitcounter() < five_minutes + 1)
      {
        if (GS::GetGame().GetTimelimitcounter() % minute == 0)
          GS::GetMainConsole().console(string("Time Left: ") +
                                           inttostr(GS::GetGame().GetTimelimitcounter() / minute) +
                                           " minutes",
                                         game_message_color);
      }
      else if (GS::GetGame().GetTimelimitcounter() % five_minutes == 0)
      {
        GS::GetMainConsole().console(string("Time Left: ") +
                                         inttostr(GS::GetGame().GetTimelimitcounter() / minute) +
                                         " minutes",
                                       game_message_color);
      }
    }

    LogTraceG("UpdateFrame 2");

    // voting timer
    GS::GetGame().timervote();

    // Consoles Update
    GS::GetMainConsole().scrolltick = GS::GetMainConsole().scrolltick + 1;
    if (GS::GetMainConsole().scrolltick == GS::GetMainConsole().scrolltickmax)
      GS::GetMainConsole().scrollconsole();

    if (GS::GetMainConsole().alphacount > 0)
      GS::GetMainConsole().alphacount = GS::GetMainConsole().alphacount - 1;

    if (!CVar::sv_advancemode)
      for (j = 1; j <= max_sprites; j++)
        for (i = 1; i <= 10; i++)
          GS::GetGame().GetWeaponsel()[j][i] = 1;
  } // bullettime off

  LogTraceG("UpdateFrame 3");

  // Infiltration mode blue team score point
  j = CVar::sv_inf_bluelimit * second;
  if (GS::GetGame().GetPlayersTeamNum(1) < GS::GetGame().GetPlayersTeamNum(2))
    j = CVar::sv_inf_bluelimit * second +
        2 * second * (GS::GetGame().GetPlayersTeamNum(2) - GS::GetGame().GetPlayersTeamNum(1));

  if (CVar::sv_gamemode == gamestyle_inf)
    if (GS::GetGame().GetMapchangecounter() < 0)
      if (things[GS::GetGame().GetTeamFlag(2)].inbase)
        if ((GS::GetGame().GetPlayersTeamNum(1) > 0) && (GS::GetGame().GetPlayersTeamNum(2) > 0))
          if (GS::GetGame().GetMainTickCounter() % j == 0)
          {
            GS::GetGame().SetTeamScore(2, GS::GetGame().GetTeamScore(2) + 1);
            GS::GetGame().sortplayers();
          }

  // HTF mode team score point
  if (GS::GetGame().GetPlayersTeamNum(2) == GS::GetGame().GetPlayersTeamNum(1))
    htftime = CVar::sv_htf_pointstime * 60;

  if (CVar::sv_gamemode == gamestyle_htf)
  {
    if (GS::GetGame().GetMapchangecounter() < 0)
    {
      if ((GS::GetGame().GetPlayersTeamNum(1) > 0) && (GS::GetGame().GetPlayersTeamNum(2) > 0))
      {
        if (GS::GetGame().GetMainTickCounter() % htftime == 0)
        {
          for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
          {
            if ((sprite.holdedthing > 0))
            {
              if (things[sprite.holdedthing].style == object_pointmatch_flag)
              {
                GS::GetGame().SetTeamScore(sprite.player->team,
                                           GS::GetGame().GetTeamScore(sprite.player->team) + 1);

                if (sprite.player->team == team_alpha)
                  htftime = htf_sec_point + 2 * second *
                                              (GS::GetGame().GetPlayersTeamNum(1) -
                                               GS::GetGame().GetPlayersTeamNum(2));

                if (sprite.player->team == team_bravo)
                  htftime = htf_sec_point + 2 * second *
                                              (GS::GetGame().GetPlayersTeamNum(2) -
                                               GS::GetGame().GetPlayersTeamNum(1));

                if (htftime < htf_sec_point)
                  htftime = htf_sec_point;

                GS::GetGame().sortplayers();
              }
            }
          }
        }
      }
    }
  }

  // Spawn Rambo bow if nobody has it and not on map
  if (CVar::sv_gamemode == gamestyle_rambo)
    if (GS::GetGame().GetMainTickCounter() % second == 0)
    {
      _x = 0;

      for (j = 1; j <= max_things; j++)
        if (things[j].active)
          if (things[j].style == object_rambo_bow)
            _x = 1;

      for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
      {
        if ((sprite.weapon.num == bow_num) || (sprite.weapon.num == bow2_num))
        {
          _x = 1;
        }
      }

      if (_x == 0)
      {
        randomizestart(m, 15);

        j = creatething(m, 255, object_rambo_bow, 255);
      }
    }

  // Destroy flags if > 1
  if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf))
    if (GS::GetGame().GetMainTickCounter() % (second * 2) == 0)
    {
      _x = 0;

      for (j = 1; j <= max_things; j++)
        if (things[j].active)
          if (things[j].style == object_alpha_flag)
            _x += 1;

      if (_x > 1)
        for (j = max_things; j >= 1; j--)
          if (things[j].active)
            if (things[j].style == object_alpha_flag)
            {
              things[j].kill();
              break;
            }

      if (_x == 0)
      {
        if (randomizestart(m, 5))
        {
          auto v = creatething(m, 255, object_alpha_flag, 255);
          GS::GetGame().SetTeamFlag(1, v);
        }
      }

      _x = 0;

      for (j = 1; j <= max_things; j++)
        if (things[j].active)
          if (things[j].style == object_bravo_flag)
            _x += 1;

      if (_x > 1)
        for (j = max_things; j >= 1; j--)
          if (things[j].active)
            if (things[j].style == object_bravo_flag)
            {
              things[j].kill();
              break;
            }

      if (_x == 0)
      {
        if (randomizestart(m, 6))
        {
          auto v = creatething(m, 255, object_bravo_flag, 255);
          GS::GetGame().SetTeamFlag(2, v);
        }
      }
    }

  if ((CVar::sv_gamemode == gamestyle_pointmatch) || (CVar::sv_gamemode == gamestyle_htf))
    if (GS::GetGame().GetMainTickCounter() % (second * 2) == 0)
    {
      _x = 0;

      for (j = 1; j <= max_things; j++)
        if (things[j].active)
          if (things[j].style == object_pointmatch_flag)
            _x += 1;

      if (_x > 1)
        for (j = max_things; j >= 1; j--)
          if (things[j].active)
            if (things[j].style == object_pointmatch_flag)
            {
              things[j].kill();
              break;
            }

      if (_x == 0)
        if (randomizestart(m, 14))
        {
          auto v = creatething(m, 255, object_pointmatch_flag, 255);
          GS::GetGame().SetTeamFlag(1, v);
        }
    }
  auto &map = GS::GetGame().GetMap();
  if ((CVar::demo_autorecord) && (GS::GetDemoRecorder().active() == false) && (map.name != ""))
  {
    NotImplemented();
#if 0
        GS::GetDemoRecorder().startrecord(GS::GetGame().GetUserDirectory() + "demos/" +
                                 formatdatetime("yyyy-mm-dd_hh-nn-ss_", now(0)) + map.name +
                                 ".sdm");
#endif
  }
}
