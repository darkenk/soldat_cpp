#include "../AnimationSystem.hpp"
#include "../Constants.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../mechanics/Sprites.hpp"
#include "GlobalVariable.hpp"
#include "common/Parts.hpp"
#include "common/PolyMap.hpp"
#include "common/Waypoints.hpp"
#include "common/Weapons.hpp"

namespace
{
auto &things = InitGlobalVariable<PascalArray<tthing, 1, max_things>, "things">();
} // namespace
