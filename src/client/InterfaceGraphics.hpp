#pragma once

#include "common/Vector.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Constants.hpp"

#include <array>
#include <cstdint>
#include <string>

auto constexpr MAX_CHAT_SIZE = 32;

// Chat stuff
extern PascalArray<std::string, 1, MAX_CHAT_SIZE> chatmessage;
extern PascalArray<bool, 1, MAX_CHAT_SIZE> chatteam;
extern PascalArray<std::int32_t, 1, MAX_CHAT_SIZE> chatdelay;

// Big Text
extern std::array<std::string, max_big_messages> bigtext;
extern std::array<std::int32_t, max_big_messages> bigdelay;
extern std::array<std::int32_t, max_big_messages> bigx;
extern std::array<float, max_big_messages> bigscale;
extern std::array<std::uint32_t, max_big_messages> bigcolor;
extern std::array<float, max_big_messages> bigposx, bigposy;

// World Text
extern std::array<std::string, max_big_messages> worldtext;
extern std::array<std::int32_t, max_big_messages> worlddelay;
extern std::array<std::int32_t, max_big_messages> worldx;
extern std::array<float, max_big_messages> worldscale;
extern std::array<std::uint32_t, max_big_messages> worldcolor;
extern std::array<float, max_big_messages> worldposx, worldposy;

extern std::string cursortext;
extern std::int32_t cursortextlength;
extern bool cursorfriendly;

extern bool fragsmenushow, statsmenushow, coninfoshow, playernamesshow, minimapshow, noobshow;

extern std::uint8_t fragsscrollmax;
extern std::uint8_t fragsscrolllev;
extern tvector2 _rscala;
extern tvector2 _iscala;
extern std::int32_t fragx, fragy;

void loadinterfacearchives(const std::string &path, bool firstonly = false);
bool loadinterfacedata(const std::string &interfacename);
void renderinterface(float timeelapsed, float width, float height);
void renderactionsnaptext(double t);
bool isdefaultinterface(const std::string& interfacename);
