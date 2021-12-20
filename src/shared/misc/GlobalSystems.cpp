#include "GlobalSystems.hpp"
#include "shared/AnimationSystem.hpp"
#include "shared/mechanics/SpriteSystem.hpp"

template <Config::Module M>
void GlobalSystems<M>::Init()
{
    SpriteSystem::Init();
    AnimationSystem::Init();
}

template <Config::Module M>
void GlobalSystems<M>::Deinit()
{
    SpriteSystem::Deinit();
    AnimationSystem::Deinit();
}

template struct GlobalSystems<>;
