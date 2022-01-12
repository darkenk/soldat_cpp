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
auto &ticks = InitGlobalVariable<std::int32_t, "ticks">();
auto &tickspersecond = InitGlobalVariable<std::int32_t, "tickspersecond">();
auto &frames = InitGlobalVariable<std::int32_t, "frames">();
auto &framespersecond = InitGlobalVariable<std::int32_t, "framespersecond">();
auto &ticktime = InitGlobalVariable<std::int32_t, "ticktime">();
auto &ticktimelast = InitGlobalVariable<std::int32_t, "ticktimelast">();
auto &goalticks = InitGlobalVariable<std::int32_t, "goalticks">();
auto &bullettimetimer = InitGlobalVariable<std::int32_t, "bullettimetimer">();

auto &alivenum = InitGlobalVariable<std::uint8_t, "alivenum">();
auto &teamalivenum = InitGlobalVariable<std::array<std::int8_t, 6>, "teamalivenum">();
auto &teamplayersnum = InitGlobalVariable<PascalArray<std::int8_t, 0, 4>, "teamplayersnum">();
auto &survivalendround = InitGlobalVariable<bool, "survivalendround">();
auto &weaponscleaned = InitGlobalVariable<bool, "weaponscleaned">();

auto &ceasefiretime = InitGlobalVariable<std::int32_t, "ceasefiretime">();
auto &mapchangetime = InitGlobalVariable<std::int32_t, "mapchangetime">();
auto &mapchangecounter = InitGlobalVariable<std::int32_t, "mapchangecounter">();
auto &mapchangename = InitGlobalVariable<std::string, "mapchangename">();
auto &mapchange = InitGlobalVariable<tmapinfo, "mapchange">();
auto &mapchangeitemid = InitGlobalVariable<std::uint64_t, "mapchangeitemid">();
auto &mapchangechecksum = InitGlobalVariable<tsha1digest, "mapchangechecksum">();
auto &timelimitcounter = InitGlobalVariable<std::int32_t, "timelimitcounter">();
auto &starthealth = InitGlobalVariable<std::int32_t, "starthealth">();
auto &timeleftsec = InitGlobalVariable<std::int32_t, "timeleftsec">();
auto &timeleftmin = InitGlobalVariable<std::int32_t, "timeleftmin">();
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

auto &maintickcounter = InitGlobalVariable<std::int32_t, "maintickcounter">();
auto &playersnum = InitGlobalVariable<std::int32_t, "playersnum">();
auto &botsnum = InitGlobalVariable<std::int32_t, "botsnum">();
auto &spectatorsnum = InitGlobalVariable<std::int32_t, "spectatorsnum">();
auto &playersteamnum = InitGlobalVariable<PascalArray<std::int32_t, 1, 4>, "playersteamnum">();
} // namespace
