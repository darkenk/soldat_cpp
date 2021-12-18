#include "GlobalSystems.hpp"
#include "shared/mechanics/SpriteSystem.hpp"

template <Config::Module M>
void GlobalSystems<M>::Init()
{
    SpriteSystem::Init();
}

template <Config::Module M>
void GlobalSystems<M>::Deinit()
{
    SpriteSystem::Deinit();
}

template struct GlobalSystems<>;
