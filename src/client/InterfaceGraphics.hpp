#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "common/Vector.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Constants.hpp"
#include "shared/Constants.cpp.h"
#include "Gfx.hpp"

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
  struct tattr
  {
    float cur;
    float def;
    std::string des;
  };
  auto getweaponattribdesc(tattr &attr) -> std::string;
  auto isinteractiveinterface() -> bool;
  auto pixelalignx(float x) -> float;
  auto pixelaligny(float y) -> float;
  auto tominimap(const tvector2 pos, float scale = 1) -> tvector2;
  void drawline(float x, float y, float w, tgfxcolor color);
  void getweaponattribs(std::int32_t i, std::vector<tattr> &attrs);
  void loaddefaultinterfacedata();
  void renderbar(std::int32_t t, std::uint8_t postype, std::int32_t x, std::int32_t rx,
                 std::int32_t y, std::int32_t ry, std::int32_t w, std::int32_t h, std::int32_t r,
                 float p, bool leftalign = true)

    ;
  void renderceasefirecounter();
  void renderchatinput(float w, float h, double t);
  void renderchattexts();
  void renderconsoletexts(float w);
  void renderendgametexts(float fragmenubottom);
  void renderescmenutext(float w, float h);
  void renderfragsmenutexts(float fragmenubottom);
  void rendergamemenutexts(float w, float h);
  void renderkickwindowtext();
  void renderkillconsoletexts(float w);
  void rendermapwindowtext();
  void renderplayerinterfacetexts(std::int32_t playerindex);
  void renderplayername(float width, float height, std::int32_t i, bool onlyoffscreen);
  void renderplayernames(float width, float height);
  void renderradiomenutexts();
  void renderrespawnandsurvivaltexts();
  void renderteammenutext();
  void renderteamscoretexts();
  void rendervotemenutexts();
  void renderweaponmenutext();
  void renderweaponstatstexts();
};

extern GlobalStateInterfaceGraphics gGlobalStateInterfaceGraphics;

// Big Text

// World Text
