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
  double chatinputtime{};
  struct
  { // 0: left; 1: right
    std::uint8_t weapon;
    std::uint8_t bullets;
    std::uint8_t healthbar;
    std::uint8_t ammobar;
    std::uint8_t reloadbar;
    std::uint8_t firebar;
    std::uint8_t jetbar;
    std::uint8_t vestbar;
  } intalign;

  struct tinterfacerelinfo
  {
    std::int32_t healthbar_rel_x, healthbar_rel_y;
    std::int32_t jetbar_rel_x, jetbar_rel_y;
    std::int32_t ammobar_rel_x, ammobar_rel_y;
    std::int32_t firebar_rel_x, firebar_rel_y;
    std::int32_t nadesbar_rel_x, nadesbar_rel_y;
  };

  struct tinterface
  {
    std::uint8_t alpha;
    bool health, ammo, vest, jet, nades, bullets, weapon, fire, team, ping, status;
    std::int32_t healthico_x, healthico_y;
    std::int32_t healthico_rotate;
    std::int32_t healthbar_x, healthbar_y;
    std::int32_t healthbar_width, healthbar_height;
    std::uint8_t healthbar_pos;
    std::int32_t healthbar_rotate;
    std::int32_t ammoico_x, ammoico_y;
    std::int32_t ammoico_rotate;
    std::int32_t ammobar_x, ammobar_y;
    std::int32_t ammobar_width, ammobar_height;
    std::uint8_t ammobar_pos;
    std::int32_t ammobar_rotate;
    std::int32_t jetico_x, jetico_y;
    std::int32_t jetico_rotate;
    std::int32_t jetbar_x, jetbar_y;
    std::int32_t jetbar_width, jetbar_height;
    std::uint8_t jetbar_pos;
    std::int32_t jetbar_rotate;
    std::int32_t vestbar_x, vestbar_y;
    std::int32_t vestbar_width, vestbar_height;
    std::uint8_t vestbar_pos;
    std::int32_t vestbar_rotate;
    std::int32_t nades_x, nades_y;
    std::int32_t nades_width, nades_height;
    std::uint8_t nades_pos;
    std::int32_t bullets_x, bullets_y;
    std::int32_t weapon_x, weapon_y;
    std::int32_t fireico_x, fireico_y;
    std::int32_t fireico_rotate;
    std::int32_t firebar_x, firebar_y;
    std::int32_t firebar_width, firebar_height;
    std::uint8_t firebar_pos;
    std::int32_t firebar_rotate;
    std::int32_t teambox_x, teambox_y;
    std::int32_t ping_x, ping_y;
    std::int32_t status_x, status_y;
  };

  using pinterface = tinterface *;

  tinterface int_;
  tinterfacerelinfo relinfo;
  tvector2 pixelsize;
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
