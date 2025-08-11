#pragma once
#include "Things.hpp"
#include "common/Constants.hpp"
#include "common/misc/PortUtilsSoldat.hpp"

template <class thing = tthing>
class TThingSystem
{
public:
  TThingSystem();

  auto &GetThings()
  {
    return Things;
  }

  void KillAll()
  {
    for (auto i = 1; i <= Constants::MAX_THINGS; i++)
    {
      Things[i].kill();
    }
  }

private:
  TThingSystem(const TThingSystem &) = delete;
  PascalArray<tthing, 1, Constants::MAX_THINGS> Things{};
};

using ThingSystem = TThingSystem<>;
