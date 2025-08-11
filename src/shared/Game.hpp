#pragma once

#include <array>
#include <cstdint>

#include "Constants.hpp"
#include "common/Vector.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include "mechanics/Bullets.hpp"
#include "mechanics/Sparks.hpp"
#include "mechanics/Things.hpp"

struct tkillsort
{
  std::int32_t kills, deaths;
  std::uint8_t flags;
  std::int32_t playernum;
  std::uint32_t color;
};

#ifndef SERVER
struct GlobalStateGame
{
  std::int32_t gamewidth;
  std::int32_t gameheight;
  float gamewidthhalf;
  float gameheighthalf;
  PascalArray<tkillsort, 1, Constants::MAX_PLAYERS> sortedteamscore;
  std::int32_t heartbeattime;
  std::int32_t heartbeattimewarnings;
  PascalArray<tspark, 1, Constants::MAX_SPARKS> spark;
};

extern GlobalStateGame gGlobalStateGame;

// / 2;
// / 2;
#endif

#ifndef SERVER

#endif

#ifndef SERVER
// spark game handling sprite
#endif

template <Config::Module M = Config::GetModule()>
class Game : public GlobalSubsystem<Game<M>>
{
public:
  void number27timing();
  void togglebullettime(bool turnon, std::int32_t duration = 30);
  void updategamestats();
  auto pointvisible(float x, float y, const int32_t i) -> bool;
  auto pointvisible2(float x, float y, const int32_t i) -> bool;
  void startvote(std::uint8_t startervote, std::uint8_t typevote, std::string targetvote,
                 std::string reasonvote);

  void stopvote();
  void timervote();
#ifdef SERVER
  void countvote(std::uint8_t voter);
#endif
  void showmapchangescoreboard();
  void showmapchangescoreboard(const std::string nextmap);
  auto isteamgame() -> bool;
#ifndef SERVER
  auto ispointonscreen(const tvector2 &point) -> bool;
#endif
  void changemap();
  void sortplayers();

  bool IsVoteActive() const
  {
    return VoteActive;
  }
  void SetVoteActive(bool active)
  {
    VoteActive = active;
  }

  std::uint8_t GetVoteType() const
  {
    return VoteType;
  }
  void SetVoteType(std::uint8_t voteType)
  {
    VoteType = voteType;
  }

  const std::string &GetVoteTarget() const
  {
    return VoteTarget;
  }
  void SetVoteTarget(const std::string &voteTarget)
  {
    VoteTarget = voteTarget;
  }

  const std::string &GetVoteStarter() const
  {
    return VoteStarter;
  }
  const std::string &GetVoteReason() const
  {
    return VoteReason;
  }
  std::int32_t GetVoteTimeRemaining() const
  {
    return VoteTimeRemaining;
  }

  bool HasVoted(std::int32_t spriteId) const
  {
    return VoteHasVoted[spriteId];
  }

  void ResetVoteCooldown(std::int32_t spriteId)
  {
    VoteCooldown[spriteId] = default_vote_time;
  }
  bool CanVote(std::int32_t spriteId) const
  {
    return VoteCooldown[spriteId] < 0;
  }
  void TickVote();

  void SetUserDirectory(const std::string &userDirectory)
  {
    UserDirectory = userDirectory;
  }
  const std::string &GetUserDirectory() const
  {
    return UserDirectory;
  }

  Polymap &GetMap()
  {
    return map;
  }
  twaypoints &GetBotPath()
  {
    return botpath;
  }

  auto GetTickTime() const
  {
    return ticktime;
  }
  void SetTickTime(const auto time)
  {
    ticktime = time;
  }

  auto GetTickTimeLast() const
  {
    return ticktimelast;
  }
  void SetTickTimeLast(const auto time)
  {
    ticktimelast = time;
  }

  auto GetGoalTicks() const
  {
    return goalticks;
  }
  void SetGoalTicks(const auto ticks)
  {
    goalticks = ticks;
  }
  bool IsDefaultGoalTicks()
  {
    return goalticks == default_goalticks;
  }
  void ResetGoalTicks()
  {
    goalticks = default_goalticks;
  };

  auto GetBulletTimeTimer() const
  {
    return bullettimetimer;
  }
  void TickBulletTimeTimer()
  {
    if (bullettimetimer > -1)
    {
      bullettimetimer -= 1;
    }
  }
  void SetBulletTimeTimer(auto _Bullettimetimer)
  {
    bullettimetimer = _Bullettimetimer;
  }

  std::uint8_t GetAlivenum() const
  {
    return alivenum;
  }
  void SetAlivenum(std::uint8_t _Alivenum)
  {
    alivenum = _Alivenum;
  }

  std::int8_t GetTeamAliveNum(std::int32_t idx) const
  {
    return teamalivenum[idx];
  }

  bool GetSurvivalEndRound() const
  {
    return survivalendround;
  }
  void SetSurvivalendround(bool _survivalendround)
  {
    survivalendround = _survivalendround;
  }

  std::int8_t GetTeamplayersnum(std::int32_t idx) const
  {
    return teamplayersnum[idx];
  }

  void CalculateTeamAliveNum(std::int32_t player);

  std::int32_t GetCeasefiretime() const
  {
    return ceasefiretime;
  }

  std::int32_t GetMapchangecounter() const
  {
    return mapchangecounter;
  }

  void SetMapchangecounter(std::int32_t _mapchangecounter)
  {
    mapchangecounter = _mapchangecounter;
  }

  std::int32_t GetStarthealth() const
  {
    return starthealth;
  }
  void SetStarthealth(std::int32_t _starthealth)
  {
    starthealth = _starthealth;
  }

  const std::string &GetMapchangename() const
  {
    return mapchangename;
  }

  void SetMapchangename(const std::string &_mapchangename)
  {
    mapchangename = _mapchangename;
  }

  std::int32_t GetTimelimitcounter() const
  {
    return timelimitcounter;
  }
  void SetTimelimitcounter(std::int32_t _timelimitcounter)
  {
    timelimitcounter = _timelimitcounter;
  }

  std::int32_t GetTimeleftmin() const
  {
    return timeleftmin;
  }
  void SetTimeleftmin(std::int32_t _timeleftmin)
  {
    timeleftmin = _timeleftmin;
  }

  std::int32_t GetTimeleftsec() const
  {
    return timeleftsec;
  }
  void SetTimeleftsec(std::int32_t _timeleftsec)
  {
    timeleftsec = _timeleftsec;
  }

  void SetMapchange(const tmapinfo &_mapchange)
  {
    mapchange = _mapchange;
  }

  const tmapinfo &GetMapchange() const
  {
    return mapchange;
  }

  void SetMapchangechecksum(const tsha1digest &_mapchangechecksum)
  {
    mapchangechecksum = _mapchangechecksum;
  }

  std::int32_t GetMapchangetime() const
  {
    return mapchangetime;
  }

  std::int32_t GetMainTickCounter() const
  {
    return maintickcounter;
  }

  void TickMainTickCounter()
  {
    maintickcounter += 1;
  }

  void ResetMainTickCounter()
  {
    maintickcounter = 0;
  }

  std::int32_t GetPlayersNum() const
  {
    return playersnum;
  }

  void SetPlayersNum(std::int32_t num)
  {
    playersnum = num;
  }

  std::int32_t GetBotsNum() const
  {
    return botsnum;
  }

  std::int32_t GetSpectatorsNum() const
  {
    return spectatorsnum;
  }

  std::int32_t GetPlayersTeamNum(std::int32_t idx) const
  {
    return playersteamnum[idx];
  }

  std::int32_t GetTeamScore(std::int32_t idx) const
  {
    return teamscore[idx];
  }

  void SetTeamScore(std::int32_t idx, std::int32_t value)
  {
    teamscore[idx] = value;
  }

  std::int32_t GetTeamFlag(std::int32_t idx) const
  {
    return teamflag[idx];
  }

  void SetTeamFlag(std::int32_t idx, std::int32_t value)
  {
    teamflag[idx] = value;
  }

  float GetSinusCounter() const
  {
    return sinuscounter;
  }
  void SetSinusCounter(float _sinuscounter)
  {
    sinuscounter = _sinuscounter;
  }

  const tsha1digest &GetCustomModChecksum() const
  {
    return custommodchecksum;
  }
  void SetCustomModChecksum(const tsha1digest &_custommodchecksum)
  {
    custommodchecksum = _custommodchecksum;
  }

  const tsha1digest &GetGameModChecksum() const
  {
    return gamemodchecksum;
  }
  void SetGameModChecksum(const tsha1digest &_gamemodchecksum)
  {
    gamemodchecksum = _gamemodchecksum;
  }

  const tsha1digest &GetMapChecksum() const
  {
    return mapchecksum;
  }
  void SetMapChecksum(const tsha1digest &_mapchecksum)
  {
    mapchecksum = _mapchecksum;
  }

  const tkillsort &GetSortedPlayers(std::int32_t idx) const
  {
    return sortedplayers[idx];
  }

  PascalArray<PascalArray<std::uint8_t, 1, main_weapons>, 1, Constants::MAX_PLAYERS> &GetWeaponsel()
  {
    return weaponsel;
  }

protected:
  Game();

private:
  bool VoteActive = false;
  std::uint8_t VoteType = vote_map;
  std::string VoteTarget;
  std::string VoteStarter;
  std::string VoteReason;
  std::int32_t VoteTimeRemaining = -1;
  std::uint8_t VoteNumVotes = 0;
  std::uint8_t VoteMaxVotes = 0;
  PascalArray<bool, 1, Constants::MAX_PLAYERS> VoteHasVoted;
  PascalArray<std::int32_t, 1, Constants::MAX_PLAYERS> VoteCooldown;
  std::string UserDirectory;
  twaypoints botpath;
  Polymap map;

  std::int32_t ticktime;
  std::int32_t ticktimelast;
  std::int32_t goalticks = default_goalticks;
  std::int32_t bullettimetimer;

  std::uint8_t alivenum;
  std::array<std::int8_t, 6> teamalivenum;
  PascalArray<std::int8_t, 0, 4> teamplayersnum;
  bool survivalendround = false;

  std::int32_t ceasefiretime = default_ceasefire_time;
  std::int32_t mapchangetime = default_mapchange_time;
  std::int32_t mapchangecounter = 0;
  std::string mapchangename;
  tmapinfo mapchange;
  std::uint64_t mapchangeitemid;
  tsha1digest mapchangechecksum;
  std::int32_t timelimitcounter = 3600;
  std::int32_t starthealth = 150;
  std::int32_t timeleftsec;
  std::int32_t timeleftmin;

  std::int32_t maintickcounter;
  std::int32_t playersnum = 0;
  std::int32_t botsnum;
  std::int32_t spectatorsnum;
  PascalArray<std::int32_t, 1, 4> playersteamnum;

  std::array<std::int32_t, 5> teamscore = {0};
  std::array<std::int32_t, 4> teamflag = {0};
  float sinuscounter = 0.0f;
  tsha1digest custommodchecksum;
  tsha1digest gamemodchecksum;
  tsha1digest mapchecksum;
  PascalArray<tkillsort, 1, Constants::MAX_PLAYERS> sortedplayers;
  PascalArray<PascalArray<std::uint8_t, 1, main_weapons>, 1, Constants::MAX_PLAYERS> weaponsel;

  friend GlobalSubsystem<Game<M>>;
};
