#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "common/Anims.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include "common/misc/SoldatConfig.hpp" // IWYU pragma: keep
#include "common/Parts.hpp"

class particlesystem;
class Skeleton;

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

  auto GetAnimation(AnimationType type) -> tanimation &;

  [[nodiscard]] auto GetSkeleton(const SkeletonType type) const -> const particlesystem &;

private:
  std::vector<tanimation> Animations;
  std::vector<particlesystem> Skeletons;
};

using AnimationSystem = TAnimationSystem<>;
