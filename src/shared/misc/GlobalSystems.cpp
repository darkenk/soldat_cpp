#include "GlobalSystems.hpp"
#include "shared/AnimationSystem.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include <memory>

template <Config::Module M>
GlobalSystems<M>::GlobalSystems()
{
    SpriteSystem::Init();
    AnimationSystem::Init();
    WeaponSystemObject = std::make_unique<WeaponSystem>();
}

template <Config::Module M>
GlobalSystems<M>::~GlobalSystems()
{
    WeaponSystemObject.release();
    AnimationSystem::Deinit();
    SpriteSystem::Deinit();
}

template class GlobalSystems<>;
