#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "common/Vector.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Constants.hpp"
#include "shared/Constants.cpp.h"

auto constexpr MAX_CHAT_SIZE = 32;

// Chat stuff
struct GlobalStateInterfaceGraphics
{
  bool isdefaultinterface(const std::string &interfacename);
  bool loadinterfacedata(const std::string &interfacename);
  void loadinterfacearchives(const std::string &path, bool firstonly = false);
  void renderactionsnaptext(double t);
  void renderinterface(float timeelapsed, float width, float height);
  PascalArray<std::string, 1, MAX_CHAT_SIZE> chatmessage = {};
  PascalArray<bool, 1, MAX_CHAT_SIZE> chatteam = {};
  PascalArray<std::int32_t, 1, MAX_CHAT_SIZE> chatdelay = {};
  std::array<std::string, max_big_messages> bigtext = {};
  std::array<std::int32_t, max_big_messages> bigdelay = {};
  std::array<std::int32_t, max_big_messages> bigx = {};
  std::array<float, max_big_messages> bigscale = {};
  std::array<std::uint32_t, max_big_messages> bigcolor = {};
  std::array<float, max_big_messages> bigposx = {};
  std::array<float, max_big_messages> bigposy = {};
  std::array<std::string, max_big_messages> worldtext = {};
  std::array<std::int32_t, max_big_messages> worlddelay = {};
  std::array<std::int32_t, max_big_messages> worldx = {};
  std::array<float, max_big_messages> worldscale = {};
  std::array<std::uint32_t, max_big_messages> worldcolor = {};
  std::array<float, max_big_messages> worldposx = {};
  std::array<float, max_big_messages> worldposy = {};
  std::string cursortext = {};
  std::int32_t cursortextlength = {};
  bool cursorfriendly = {};
  bool fragsmenushow = {};
  bool statsmenushow = {};
  bool coninfoshow = {};
  bool playernamesshow = {};
  bool minimapshow = {};
  bool noobshow = {};
  std::uint8_t fragsscrollmax = 0;
  std::uint8_t fragsscrolllev = 0;
  tvector2 _rscala{};
  tvector2 _iscala{};
  std::int32_t fragx = {};
  std::int32_t fragy = {};

private:
};

extern GlobalStateInterfaceGraphics gGlobalStateInterfaceGraphics;

// Big Text

// World Text
