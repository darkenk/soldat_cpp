#pragma once

#include <string>
#include <cstdint>

/*#include "Windows.h"*/
/*#include "SDL2.h"*/
#include "common/Vector.hpp"
#include "common/Constants.hpp"

void resetframetiming();
void gameloop();
std::int32_t getgamefps();
float getcurrenttime();
void tabcomplete();
void resetweaponstats();
void bigmessage(const std::string &text, std::int32_t delay, std::uint32_t col);
std::uint8_t getcameratarget(bool backwards = false);

struct GlobalStateClientGame
{
  tvector2 mouseprev;
  float mx;
  float my;
  bool mapchanged;
  bool chatchanged;
  bool shouldrenderframes;
  std::uint8_t actionsnap;
  bool actionsnaptaken;
  std::int32_t capscreen;
  std::uint8_t showscreen;
  std::uint8_t screencounter;
  bool isfullscreen;
  std::int32_t screenwidth;
  std::int32_t screenheight;
  std::int32_t renderwidth;
  std::int32_t renderheight;
  std::int32_t windowwidth;
  std::int32_t windowheight;
  std::string chattext;
  std::string lastchattext;
  std::string firechattext;
  std::uint8_t chattype;
  std::string completionbase;
  std::int32_t completionbaseseparator;
  std::uint8_t currenttabcompleteplayer;
  std::uint8_t cursorposition;
  bool tabcompletepressed;
  std::int32_t chattimecounter;
  std::int32_t clientstopmovingcounter;
  bool forceclientspritesnapshotmov;
  std::uint32_t lastforceclientspritesnapshotmovtick;
  std::int32_t menutimer;
};

extern GlobalStateClientGame gGlobalStateClientGame;

// used for blinking chat input
// false during game request phase

// us std::uint8_t  action snap

// resolution

// cha std::uint8_t f
