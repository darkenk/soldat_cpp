#pragma once

#include <string>
#include <cinttypes>

/*#include "Windows.h"*/
/*#include "SDL2.h"*/
#include "common/Vector.hpp"
#include "common/Constants.hpp"

struct GlobalStateClientGame
{
  float getcurrenttime();
  std::int32_t getgamefps();
  std::uint8_t getcameratarget(bool backwards = false);
  void bigmessage(const std::string &text, std::int32_t delay, std::uint32_t col);
  void gameloop();
  void resetframetiming();
  void resetweaponstats();
  void tabcomplete();
  tvector2 mouseprev{};
  float mx = {};
  float my = {};
  bool mapchanged = false;
  bool chatchanged = true;
  bool shouldrenderframes = true;
  std::uint8_t actionsnap = 1;
  bool actionsnaptaken = false;
  std::int32_t capscreen = 255;
  std::uint8_t showscreen = 0u;
  std::uint8_t screencounter = 255;
  bool isfullscreen = {};
  std::int32_t screenwidth = Constants::DEFAULT_WIDTH;
  std::int32_t screenheight = Constants::DEFAULT_HEIGHT;
  std::int32_t renderwidth = 0;
  std::int32_t renderheight = 0;
  std::int32_t windowwidth = 0;
  std::int32_t windowheight = 0;
  std::string chattext = {};
  std::string lastchattext = {};
  std::string firechattext = {};
  std::uint8_t chattype = {};
  std::string completionbase = {};
  std::int32_t completionbaseseparator = {};
  std::uint8_t currenttabcompleteplayer = 0;
  std::uint8_t cursorposition = 0;
  bool tabcompletepressed = {};
  std::int32_t chattimecounter = {};
  std::int32_t clientstopmovingcounter = 99999;
  bool forceclientspritesnapshotmov = {};
  std::uint32_t lastforceclientspritesnapshotmovtick = {};
  std::int32_t menutimer = {};

private:
  struct tframetiming
  {
    std::int64_t frequency;
    std::int64_t starttime;
    double prevtime;
    double prevrendertime;
    double accumulator;
    double mindeltatime;
    double elapsed;
    std::int32_t counter;
    std::int32_t fps;
    double fpsaccum;
  };
  tframetiming frametiming;
};

extern GlobalStateClientGame gGlobalStateClientGame;

// used for blinking chat input
// false during game request phase

// us std::uint8_t  action snap

// resolution

// cha std::uint8_t f
