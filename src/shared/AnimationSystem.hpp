#pragma once

#include "common/Anims.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include "misc/SoldatConfig.hpp"
#include <string>

template <Config::Module M = Config::GetModule()>
class TAnimationSystem : public GlobalSubsystem<TAnimationSystem<M>>
{
  public:
    void LoadAnimObjects(const std::string &moddir);

    tanimation& GetAnimation(AnimationType type);
  private:
    std::vector<tanimation> Animations;
};

using AnimationSystem = TAnimationSystem<>;
