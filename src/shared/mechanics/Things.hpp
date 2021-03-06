#pragma once

#include "BackgroundState.hpp"
#include "common/MapFile.hpp"
#include "common/Parts.hpp"
#include <array>
#include <cstdint>

class tthingcollision
{
public:
  std::uint8_t thingnum;
  std::uint32_t cooldownend;
};

template <Config::Module M = Config::GetModule()>
class Thing
{
public:
  bool active{false};
  std::uint8_t style, num{0}, owner;
  std::uint8_t holdingsprite;
  std::uint8_t ammocount;
  float radius;
  std::int32_t timeout;
  bool statictype;
  std::int32_t interest;
  bool collidewithbullets;
  bool inbase;
  std::uint8_t lastspawn, team;
  particlesystem skeleton;
  PascalArray<std::uint8_t, 1, 4> collidecount;
  PascalArray<tmappolygon, 1, 2> polys;
  tbackgroundstate bgstate;
#ifndef SERVER
  std::int32_t tex1, tex2;
  std::uint32_t texture;
  std::uint32_t color;
#endif
  //    public
  void update();
#ifndef SERVER
  void render(double timeelapsed);
  void polygonsrender();
#endif
  bool checkmapcollision(std::int32_t i, float x, float y);
  void kill();
  void checkoutofbounds();
  void respawn();
  void moveskeleton(float x1, float y1, bool fromzero);
#ifdef SERVER
  std::int32_t checkspritecollision();
#endif
#ifdef SERVER
  std::int32_t checkstationaryguncollision();
#else
  std::int32_t checkstationaryguncollision(bool clientcheck);
#endif
};

template <Config::Module M = Config::GetModule()>
std::int32_t creatething(tvector2 spos, std::uint8_t owner, std::uint8_t sstyle, std::uint8_t n);
template <Config::Module M = Config::GetModule()>
tthingcollision thingcollision(std::uint8_t thingnum, std::uint32_t cooldownend);
template <Config::Module M = Config::GetModule()>
bool spawnboxes(tvector2 &start, std::uint8_t team, std::uint8_t num);
template <Config::Module M = Config::GetModule()>
bool randomizestart(tvector2 &start, std::uint8_t team);

using tthing = Thing<Config::GetModule()>;
