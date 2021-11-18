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

template <Config::Module M = Config::GetModule()>
void number27timing();
template <Config::Module M = Config::GetModule()>
void togglebullettime(bool turnon, std::int32_t duration = 30);
template <Config::Module M = Config::GetModule()>
void updategamestats();
template <Config::Module M = Config::GetModule()>
bool pointvisible(float x, float y, std::int32_t i);
template <Config::Module M = Config::GetModule()>
bool pointvisible2(float x, float y, std::int32_t i);
template <Config::Module M = Config::GetModule()>
void startvote(std::uint8_t startervote, std::uint8_t typevote, std::string targetvote,
               std::string reasonvote);

template <Config::Module M = Config::GetModule()>
void stopvote();
template <Config::Module M = Config::GetModule()>
void timervote();
#ifdef SERVER
void countvote(std::uint8_t voter);
#endif
template <Config::Module M = Config::GetModule()>
void showmapchangescoreboard();
template <Config::Module M = Config::GetModule()>
void showmapchangescoreboard(const std::string nextmap);
template <Config::Module M = Config::GetModule()>
bool isteamgame();
#ifndef SERVER
bool ispointonscreen(tvector2 point);
#endif
template <Config::Module M = Config::GetModule()>
void changemap();
template <Config::Module M = Config::GetModule()>
void sortplayers();
