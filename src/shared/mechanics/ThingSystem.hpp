#pragma once
#include "Things.hpp"

template<class thing = tthing>
class TThingSystem
{
  public:
    TThingSystem();

    auto& GetThings()
    {
        return Things;
    }

    void KillAll()
    {
        for (auto i = 1; i <= max_things; i++)
        {
            Things[i].kill();
        }
    }

  private:
    TThingSystem(const TThingSystem&) = delete;
    PascalArray<tthing, 1, max_things> Things;
};

using ThingSystem = TThingSystem<>;
