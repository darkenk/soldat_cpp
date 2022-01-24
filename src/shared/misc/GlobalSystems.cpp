#include "GlobalSystems.hpp"
#include "shared/AnimationSystem.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include <memory>

template <Config::Module M>
GlobalSystems<M>::GlobalSystems()
{
    SpriteSystem::Init();
    AnimationSystem::Init();
    WeaponSystemObject = std::make_unique<WeaponSystem>();
    DemoRecorder = std::make_unique<tdemorecorder<M>>();
    Game<M>::Init();
}

template <Config::Module M>
GlobalSystems<M>::~GlobalSystems()
{
    Game<M>::Deinit();
    DemoRecorder.release();
    WeaponSystemObject.release();
    AnimationSystem::Deinit();
    SpriteSystem::Deinit();
}

template class GlobalSystems<>;
