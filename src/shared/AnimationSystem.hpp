#pragma once

#include "common/Anims.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include "misc/SoldatConfig.hpp"
#include <string>

class particlesystem;

enum SkeletonType : std::int32_t
{
  Gostek,
  Box,
  Flag,
  Para,
  Stat,
  Rifle10,
  Rifle11,
  Rifle18,
  Rifle22,
  Rifle28,
  Rifle36,
  Rifle37,
  Rifle39,
  Rifle43,
  Rifle50,
  Rifle55
};

template <Config::Module M = Config::GetModule()>
class TAnimationSystem : public GlobalSubsystem<TAnimationSystem<M>>
{
public:
  void LoadAnimObjects(const std::string &moddir);

  tanimation &GetAnimation(AnimationType type);

  [[nodiscard]] const particlesystem &GetSkeleton(const SkeletonType type) const;

private:
  std::vector<tanimation> Animations;
  std::vector<particlesystem> Skeletons;
};

using AnimationSystem = TAnimationSystem<>;
