// automatically converted

#include "NetworkServerHeartbeat.hpp"
#include "NetworkServer.hpp"
#include "../Demo.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

template<class TSprite, Config::Module M>
void serverheartbeat(NetworkServer& transport, TSpriteSystem<TSprite>& spriteSystem, Game<M>& game)
{
  tmsg_heartbeat heartbeatmsg; // NOLINT
#if SOLDAT_UTBOT
  auto fill = [](auto &arr, auto val) { std::for_each(std::begin(arr), std::end(arr), [val](auto &v) { v = val; }); };
  fill(heartbeatmsg.active, false);
  fill(heartbeatmsg.kills, 0);
  fill(heartbeatmsg.caps, 0);
  fill(heartbeatmsg.team, 0);
  fill(heartbeatmsg.deaths, 0);
  fill(heartbeatmsg.flags, 0);
  fill(heartbeatmsg.ping, 255);
#else
  std::ranges::fill(heartbeatmsg.active, false);
  std::ranges::fill(heartbeatmsg.kills, 0);
  std::ranges::fill(heartbeatmsg.caps, 0);
  std::ranges::fill(heartbeatmsg.team, 0);
  std::ranges::fill(heartbeatmsg.deaths, 0);
  std::ranges::fill(heartbeatmsg.flags, 0);
  std::ranges::fill(heartbeatmsg.ping, 255);
#endif // SOLDAT_UTBOT

  for (auto c = 0; auto &s : spriteSystem.GetActiveSprites())
  {
    auto& player = *s.player;
    heartbeatmsg.active[c] = s.active;
    heartbeatmsg.kills[c] = player.kills;
    heartbeatmsg.caps[c] = player.flags;
    heartbeatmsg.deaths[c] = player.deaths;
    heartbeatmsg.team[c] = player.team;
    heartbeatmsg.flags[c] = player.flags;
    heartbeatmsg.ping[c] = player.pingticks;
    heartbeatmsg.realping[c] = player.realping;
    heartbeatmsg.connectionquality[c] = player.connectionquality;
    c++;
  }
  heartbeatmsg.teamscore[0] = game.GetTeamScore(team_alpha);
  heartbeatmsg.teamscore[1] = game.GetTeamScore(team_bravo);
  heartbeatmsg.teamscore[2] = game.GetTeamScore(team_charlie);
  heartbeatmsg.teamscore[3] = game.GetTeamScore(team_delta);

  auto &map = game.GetMap();
  heartbeatmsg.mapid = map.mapid;
  if (game.GetMapchangecounter() > 0)
  {
    heartbeatmsg.mapid = 0;
  }
  if ((CVar::sv_timelimit - game.GetTimelimitcounter()) < 600)
  {
    heartbeatmsg.mapid = 0;
  }
  if (game.GetTimelimitcounter() < 600)
  {
    heartbeatmsg.mapid = 0;
  }

  for (auto &s : spriteSystem.GetActiveSprites())
  {
    if (s.player->controlmethod != human)
    {
      continue;
    }
    [[maybe_unused]] auto ret = transport.SendData(heartbeatmsg, s.player->peer);
    SoldatAssert(ret == true);
  }

}
// tests
#include <doctest/doctest.h>
#include "NetworkClient.hpp"

namespace
{

class LogBumper
{
public:
  LogBumper(std::string_view logger, spdlog::level::level_enum lv = spdlog::level::trace)
    : Logger(logger)
  {
    PreviousLevel = spdlog::get(Logger)->level();
    spdlog::get(Logger)->set_level(lv);
  }
  ~LogBumper() { spdlog::get(Logger)->set_level(PreviousLevel); }

private:
  std::string Logger;
  spdlog::level::level_enum PreviousLevel;
};

class NetworkServerHeartbeatFixture
{
public:
  NetworkServerHeartbeatFixture(): LogBumperNetMsg("net_msg"), LogBumperNetworkMsg("network")
  {
    GlobalSystems<Config::CLIENT_MODULE>::Init();
    GlobalSystems<Config::SERVER_MODULE>::Init();
    AnimationSystem::Get().LoadAnimObjects("");
  }
  NetworkServerHeartbeatFixture(const NetworkServerHeartbeatFixture &) = delete;
  ~NetworkServerHeartbeatFixture()
  {
    GlobalSystems<Config::SERVER_MODULE>::Deinit();
    GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  }

protected:
  LogBumper LogBumperNetMsg;
  LogBumper LogBumperNetworkMsg;
};

TEST_CASE_FIXTURE(NetworkServerHeartbeatFixture, "Initial test for heartbeat" * doctest::skip(true))
{
  auto &spriteSystem = SpriteSystem::Get();
  auto &game = GS::GetGame();
  auto server = std::make_unique<NetworkServer>("0.0.0.0", 23073);
  auto client = std::make_unique<NetworkClientImpl>();
  client->Connect("127.0.0.1", 23073);
  while(server->GetPlayers().empty())
  {
    client->FlushMsg();
    server->FlushMsg();
    client->ProcessLoop();
    server->ProcessLoop();
  }
  SoldatAssert(server->GetPlayers().size() == 1);
  auto player = server->GetPlayers().at(0);
  player->controlmethod = human;
  tvector2 spos; // out
  std::uint8_t spriteId = 255;
  createsprite(spos, spriteId, player);

  serverheartbeat(*server, spriteSystem, game);

}

} // namespace
