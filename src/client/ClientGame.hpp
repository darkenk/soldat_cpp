#pragma once

/*#include "Windows.h"*/
/*#include "SDL2.h"*/
#include "common/Vector.hpp"
#include "shared/Constants.hpp"
#include <string>

void resetframetiming();
void gameloop();
std::int32_t getgamefps();
float getcurrenttime();
void tabcomplete();
void resetweaponstats();
void bigmessage(const std::string &text, std::int32_t delay, std::uint32_t col);
std::uint8_t getcameratarget(bool backwards = false);
#ifdef STEAM
void getmicdata();
#endif

extern tvector2 mouseprev;
extern float mx, my;
extern bool mapchanged;
extern bool chatchanged;        // used for blinking chat input
extern bool shouldrenderframes; // false during game request phase

// us std::uint8_t  action snap
extern std::uint8_t actionsnap;
extern bool actionsnaptaken;
extern std::int32_t capscreen;
extern std::uint8_t showscreen;
extern std::uint8_t screencounter;

// resolution
extern bool isfullscreen;
extern std::int32_t screenwidth;
extern std::int32_t screenheight;
extern std::int32_t renderwidth;
extern std::int32_t renderheight;
extern std::int32_t windowwidth;
extern std::int32_t windowheight;

// cha std::uint8_t f
extern std::string chattext, lastchattext, firechattext;
extern std::uint8_t chattype;
extern std::string completionbase;
extern std::int32_t completionbaseseparator;
extern std::uint8_t currenttabcompleteplayer;
extern std::uint8_t cursorposition;
extern bool tabcompletepressed;
extern std::int32_t chattimecounter;

extern std::int32_t clientstopmovingcounter;
extern bool forceclientspritesnapshotmov;
extern std::uint32_t lastforceclientspritesnapshotmovtick;
extern std::int32_t menutimer;
