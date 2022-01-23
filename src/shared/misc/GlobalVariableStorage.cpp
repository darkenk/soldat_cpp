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

auto &teamscore = InitGlobalVariable<std::array<std::int32_t, 5>, "teamscore">();
auto &teamflag = InitGlobalVariable<std::array<std::int32_t, 4>, "teamflag">();
auto &sinuscounter = InitGlobalVariable<float, "sinuscounter">();
auto &custommodchecksum = InitGlobalVariable<tsha1digest, "custommodchecksum">();
auto &gamemodchecksum = InitGlobalVariable<tsha1digest, "gamemodchecksum">();
auto &mapchecksum = InitGlobalVariable<tsha1digest, "mapchecksum">();
auto &mapindex = InitGlobalVariable<std::int32_t, "mapindex">();
auto &sortedplayers = InitGlobalVariable<PascalArray<tkillsort, 1, max_sprites>, "sortedplayers">();

auto &bullet = InitGlobalVariable<PascalArray<tbullet, 1, max_bullets>, "bullet">();
auto &things = InitGlobalVariable<PascalArray<tthing, 1, max_things>, "things">();

auto &guns = InitGlobalVariable<GunsDescription, "guns">();
auto &defaultguns = InitGlobalVariable<GunsDescription, "defaultguns">();
auto &defaultwmchecksum = InitGlobalVariable<std::uint64_t, "defaultwmchecksum">();
auto &loadedwmchecksum = InitGlobalVariable<std::uint64_t, "loadedwmchecksum">();

auto &demorecorder = InitGlobalVariable<tdemorecorder<>, "demorecorder">();
} // namespace
