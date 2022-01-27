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
auto &weaponsel =
    InitGlobalVariable<PascalArray<PascalArray<std::uint8_t, 1, main_weapons>, 1, max_sprites>,
                       "weaponsel">();

auto &bullet = InitGlobalVariable<PascalArray<tbullet, 1, max_bullets>, "bullet">();
auto &things = InitGlobalVariable<PascalArray<tthing, 1, max_things>, "things">();
} // namespace
