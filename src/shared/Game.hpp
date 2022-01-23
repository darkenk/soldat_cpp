#pragma once

#include <array>
#include <cstdint>

#include "Constants.hpp"
#include "common/Vector.hpp"
#include "mechanics/Bullets.hpp"
#include "mechanics/Sparks.hpp"
#include "mechanics/Sprites.hpp"
#include "mechanics/Things.hpp"
#include "common/misc/GlobalSubsystem.hpp"

struct tkillsort
{
    std::int32_t kills, deaths;
    std::uint8_t flags;
    std::int32_t playernum;
    std::uint32_t color;
};

#ifndef SERVER
extern std::int32_t gamewidth;
extern std::int32_t gameheight;

extern float gamewidthhalf;  // / 2;
extern float gameheighthalf; // / 2;
#endif

#ifndef SERVER
extern PascalArray<tkillsort, 1, max_sprites> sortedteamscore;

extern std::int32_t heartbeattime, heartbeattimewarnings;
#endif

#ifndef SERVER
extern PascalArray<tspark, 1, max_sparks> spark; // spark game handling sprite
#endif

template<Config::Module M = Config::GetModule()>
class Game : public GlobalSubsystem<Game<M>>
{
  public:
    void number27timing();
    void togglebullettime(bool turnon, std::int32_t duration = 30);
    void updategamestats();
    bool pointvisible(float x, float y, const int32_t i);
    bool pointvisible2(float x, float y, const int32_t i);
    void startvote(std::uint8_t startervote, std::uint8_t typevote, std::string targetvote,
               std::string reasonvote);

    void stopvote();
    void timervote();
#ifdef SERVER
    void countvote(std::uint8_t voter);
#endif
    void showmapchangescoreboard();
    void showmapchangescoreboard(const std::string nextmap);
    bool isteamgame();
#ifndef SERVER
    bool ispointonscreen(const tvector2& point);
#endif
    void changemap();
    void sortplayers();

    bool IsVoteActive() const { return VoteActive; }
    void SetVoteActive(bool active) { VoteActive = active; }

    std::uint8_t GetVoteType() const { return VoteType; }
    void SetVoteType(std::uint8_t voteType) { VoteType = voteType; }

    const std::string &GetVoteTarget() const { return VoteTarget; }
    void SetVoteTarget(const std::string &voteTarget) { VoteTarget = voteTarget; }

    const std::string& GetVoteStarter() const { return VoteStarter; }
    const std::string& GetVoteReason() const { return VoteReason; }
    std::int32_t GetVoteTimeRemaining() const { return VoteTimeRemaining; }

    bool HasVoted(std::int32_t spriteId) const { return VoteHasVoted[spriteId]; }

    void ResetVoteCooldown(std::int32_t spriteId) { VoteCooldown[spriteId] = default_vote_time; }
    bool CanVote(std::int32_t spriteId) const { return VoteCooldown[spriteId] < 0; }
    void TickVote();

    void SetUserDirectory(const std::string& userDirectory) { UserDirectory = userDirectory; }
    const std::string& GetUserDirectory() const { return UserDirectory; }


    Polymap& GetMap() { return map; }
    twaypoints& GetBotPath() { return botpath; }

    auto GetTickTime() const { return ticktime; }
    void SetTickTime(const auto time) { ticktime = time; }

    auto GetTickTimeLast() const { return ticktimelast; }
    void SetTickTimeLast(const auto time) { ticktimelast = time; }

    auto GetGoalTicks() const { return goalticks; }
    void SetGoalTicks(const auto ticks) { goalticks = ticks; }
    bool IsDefaultGoalTicks() { return goalticks == default_goalticks; }
    void ResetGoalTicks() { goalticks = default_goalticks; };

    auto GetBulletTimeTimer() const { return bullettimetimer; }
    void TickBulletTimeTimer() { if (bullettimetimer > -1) { bullettimetimer -= 1; } }
    void SetBulletTimeTimer(auto _Bullettimetimer) { bullettimetimer = _Bullettimetimer; }

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
    PascalArray<bool, 1, max_sprites> VoteHasVoted;
    PascalArray<bool, 1, max_sprites> VoteCooldown;
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
    std::int32_t mapchangecounter;
    std::string mapchangename;
    tmapinfo mapchange ;
    std::uint64_t mapchangeitemid;
    tsha1digest mapchangechecksum;
    std::int32_t timelimitcounter = 3600;
    std::int32_t starthealth = 150;
    std::int32_t timeleftsec;
    std::int32_t timeleftmin;

    friend GlobalSubsystem<Game<M>>;
};

