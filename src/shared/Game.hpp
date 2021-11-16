#pragma once

#include <array>
#include <cstdint>

#include "Constants.hpp"
#include "PolyMap.hpp"
#include "Vector.hpp"
#include "mechanics/Bullets.hpp"
#include "mechanics/Sparks.hpp"
#include "mechanics/Sprites.hpp"
#include "mechanics/Things.hpp"

struct tkillsort
{
    std::int32_t kills, deaths;
    std::uint8_t flags;
    std::int32_t playernum;
    std::uint32_t color;
};

// TODO: move all dependent functions in this unit
extern std::int32_t ticks, tickspersecond; // Tick counter and Ticks persecond counter
// Ticks counter etc
extern std::int32_t frames, framespersecond, ticktime, ticktimelast;
extern std::int32_t goalticks;

extern std::int32_t bullettimetimer;

#ifndef SERVER
extern std::int32_t gamewidth;
extern std::int32_t gameheight;

extern float gamewidthhalf;  // / 2;
extern float gameheighthalf; // / 2;
#endif
// Ping Impr - vars
extern PascalArray<PascalArray<tvector2, 0, max_oldpos>, 1, max_sprites> oldspritepos;

// survival vars
extern std::uint8_t alivenum;
extern std::array<std::int8_t, 6> teamalivenum;
extern PascalArray<std::int8_t, 0, 4> teamplayersnum;
extern bool survivalendround;
extern bool weaponscleaned;

extern std::int32_t ceasefiretime;
extern std::int32_t mapchangetime;
extern std::int32_t mapchangecounter;
extern std::string mapchangename;
extern tmapinfo mapchange;
extern std::uint64_t mapchangeitemid;
extern tsha1digest mapchangechecksum;
extern std::int32_t timelimitcounter;
extern std::int32_t starthealth;
extern std::int32_t timeleftsec, timeleftmin;
extern PascalArray<PascalArray<std::uint8_t, 1, main_weapons>, 1, max_sprites> weaponsel;

extern std::array<std::int32_t, 5> teamscore;
extern std::array<std::int32_t, 4> teamflag;

extern float sinuscounter;

extern tpolymap map;

extern tsha1digest gamemodchecksum;
extern tsha1digest custommodchecksum;
extern tsha1digest mapchecksum;

extern std::int32_t mapindex;

extern PascalArray<tkillsort, 1, max_sprites> sortedplayers;
#ifndef SERVER
extern PascalArray<tkillsort, 1, max_sprites> sortedteamscore;

extern std::int32_t heartbeattime, heartbeattimewarnings;
#endif

// Sprites
// FIXME: client has frozen bullets when Sprite array position is "bad"
// if happens again change Sprite array to 0..MAX_SPRITES to "fix" it
// possible cause: out of range array read (index 0 instead of 1)
extern PascalArray<tsprite, 1, max_sprites> sprite; // player, game handling sprite
extern PascalArray<tbullet, 1, max_bullets> bullet; // bullet game handling sprite
#ifndef SERVER
extern PascalArray<tspark, 1, max_sparks> spark; // spark game handling sprite
#endif
extern PascalArray<tthing, 1, max_things> thing; // thing game handling sprite

// voting
extern bool voteactive;
extern std::uint8_t votetype; // VOTE_MAP or VOTE_KICK
extern std::string votetarget;
extern std::string votestarter;
extern std::string votereason;
extern std::int32_t votetimeremaining;
extern std::uint8_t votenumvotes;
extern std::uint8_t votemaxvotes;
extern PascalArray<bool, 1, max_sprites> votehasvoted;
extern PascalArray<std::int32_t, 1, max_sprites> votecooldown;
extern bool votekickreasontype;

void number27timing();
void togglebullettime(bool turnon, std::int32_t duration = 30);
void updategamestats();
bool pointvisible(float x, float y, std::int32_t i);
bool pointvisible2(float x, float y, std::int32_t i);
void startvote(std::uint8_t startervote, std::uint8_t typevote, std::string targetvote,
               std::string reasonvote);
void stopvote();
template <Config::Module M = Config::GetModule()>
void timervote();
#ifdef SERVER
void countvote(std::uint8_t voter);
#endif
void showmapchangescoreboard();
void showmapchangescoreboard(const std::string nextmap);
bool isteamgame();
#ifndef SERVER
bool ispointonscreen(tvector2 point);
#endif
void changemap();
void sortplayers();
