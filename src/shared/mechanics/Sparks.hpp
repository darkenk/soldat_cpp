#pragma once
#include "common/Vector.hpp"
#include "shared/misc/SoldatConfig.hpp"
#include <cstdint>

class Polymap;
class particlesystem;

class tspark
{
public:
  bool active;
  std::int16_t num;
  float lifefloat;
  std::uint8_t life, lifeprev;
  std::uint8_t style, owner;
  std::uint8_t collidecount;

public:
  void update();
  void render() const;
  bool checkmapcollision(float x, float y);
  void kill();
  void checkoutofbounds();
};
std::int32_t createspark(tvector2 spos, tvector2 svelocity, std::uint8_t sstyle,
                         std::uint8_t sowner, std::int32_t life);

extern std::int32_t sparkscount;

template <Config::Module M = Config::GetModule()>
particlesystem &GetSparkParts();
