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
    bool pointvisible(float x, float y, std::int32_t i);
    bool pointvisible2(float x, float y, std::int32_t i);
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

    friend GlobalSubsystem<Game<M>>;
};

