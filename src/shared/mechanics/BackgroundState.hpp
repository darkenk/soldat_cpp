#pragma once

#include "common/PolyMap.hpp"
#include "shared/misc/SoldatConfig.hpp"

template <Config::Module M = Config::GetModule()>
class BackgroundState
{
public:
  std::uint8_t backgroundstatus;
  std::int16_t backgroundpoly;
  bool backgroundtestresult;
  auto backgroundtest(const PolyMapSector::Poly &poly) -> bool;
  void backgroundtestbigpolycenter(const tvector2 &pos);
  auto backgroundfindcurrentpoly(const tvector2 &pos) -> std::int16_t;
  void backgroundtestprepare();
  void backgroundtestreset();
};

using tbackgroundstate = BackgroundState<Config::GetModule()>;
