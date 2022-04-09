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
  bool backgroundtest(const PolyMapSector::Poly &poly);
  void backgroundtestbigpolycenter(const tvector2 &pos);
  std::int16_t backgroundfindcurrentpoly(const tvector2 &pos);
  void backgroundtestprepare();
  void backgroundtestreset();
};

using tbackgroundstate = BackgroundState<Config::GetModule()>;
