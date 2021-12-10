#include "../Anims.hpp"
#include "../Constants.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../PolyMap.hpp"
#include "../mechanics/Sprites.hpp"
#include "GlobalVariable.hpp"
#include "common/Parts.hpp"
#include "common/Waypoints.hpp"
#include "common/Weapons.hpp"

namespace
{
auto &weaponactive =
    InitGlobalVariable<PascalArray<std::uint8_t, 1, main_weapons>, "weaponactive">();
auto &userdirectory = InitGlobalVariable<std::string, "userdirectory">();

auto &grav = InitGlobalVariable<float, "grav">();

auto &run = InitGlobalVariable<tanimation, "run">();
auto &stand = InitGlobalVariable<tanimation, "stand">();
auto &runback = InitGlobalVariable<tanimation, "runback">();
auto &jump = InitGlobalVariable<tanimation, "jump">();
auto &jumpside = InitGlobalVariable<tanimation, "jumpside">();
auto &roll = InitGlobalVariable<tanimation, "roll">();
auto &rollback = InitGlobalVariable<tanimation, "rollback">();
auto &fall = InitGlobalVariable<tanimation, "fall">();
auto &crouch = InitGlobalVariable<tanimation, "crouch">();
auto &crouchrun = InitGlobalVariable<tanimation, "crouchrun">();
auto &crouchrunback = InitGlobalVariable<tanimation, "crouchrunback">();
auto &reload = InitGlobalVariable<tanimation, "reload">();
auto &throw_ = InitGlobalVariable<tanimation, "throw_">();
auto &recoil = InitGlobalVariable<tanimation, "recoil">();
auto &shotgun = InitGlobalVariable<tanimation, "shotgun">();
auto &barret = InitGlobalVariable<tanimation, "barret">();
auto &smallrecoil = InitGlobalVariable<tanimation, "smallrecoil">();
auto &aimrecoil = InitGlobalVariable<tanimation, "aimrecoil">();
auto &handsuprecoil = InitGlobalVariable<tanimation, "handsuprecoil">();
auto &clipin = InitGlobalVariable<tanimation, "clipin">();
auto &clipout = InitGlobalVariable<tanimation, "clipout">();
auto &slideback = InitGlobalVariable<tanimation, "slideback">();
auto &change = InitGlobalVariable<tanimation, "change">();
auto &throwweapon = InitGlobalVariable<tanimation, "throwweapon">();
auto &weaponnone = InitGlobalVariable<tanimation, "weaponnone">();
auto &punch = InitGlobalVariable<tanimation, "punch">();
auto &reloadbow = InitGlobalVariable<tanimation, "reloadbow">();
auto &melee = InitGlobalVariable<tanimation, "melee">();
auto &cigar = InitGlobalVariable<tanimation, "cigar">();
auto &match = InitGlobalVariable<tanimation, "match">();
auto &smoke = InitGlobalVariable<tanimation, "smoke">();
auto &wipe = InitGlobalVariable<tanimation, "wipe">();
auto &groin = InitGlobalVariable<tanimation, "groin">();
auto &takeoff = InitGlobalVariable<tanimation, "takeoff">();
auto &victory = InitGlobalVariable<tanimation, "victory">();
auto &piss = InitGlobalVariable<tanimation, "piss">();
auto &mercy = InitGlobalVariable<tanimation, "mercy">();
auto &mercy2 = InitGlobalVariable<tanimation, "mercy2">();
auto &own = InitGlobalVariable<tanimation, "own">();
auto &prone = InitGlobalVariable<tanimation, "prone">();
auto &getup = InitGlobalVariable<tanimation, "getup">();
auto &pronemove = InitGlobalVariable<tanimation, "pronemove">();
auto &aim = InitGlobalVariable<tanimation, "aim">();
auto &handsupaim = InitGlobalVariable<tanimation, "handsupaim">();

auto &spriteparts = InitGlobalVariable<particlesystem, "spriteparts">();
auto &bulletparts = InitGlobalVariable<particlesystem, "bulletparts">();
auto &sparkparts = InitGlobalVariable<particlesystem, "sparkparts">();
auto &gostekskeleton = InitGlobalVariable<particlesystem, "gostekskeleton">();
auto &boxskeleton = InitGlobalVariable<particlesystem, "boxskeleton">();
auto &flagskeleton = InitGlobalVariable<particlesystem, "flagskeleton">();
auto &paraskeleton = InitGlobalVariable<particlesystem, "paraskeleton">();
auto &statskeleton = InitGlobalVariable<particlesystem, "statskeleton">();
auto &rifleskeleton10 = InitGlobalVariable<particlesystem, "rifleskeleton10">();
auto &rifleskeleton11 = InitGlobalVariable<particlesystem, "rifleskeleton11">();
auto &rifleskeleton18 = InitGlobalVariable<particlesystem, "rifleskeleton18">();
auto &rifleskeleton22 = InitGlobalVariable<particlesystem, "rifleskeleton22">();
auto &rifleskeleton28 = InitGlobalVariable<particlesystem, "rifleskeleton28">();
auto &rifleskeleton36 = InitGlobalVariable<particlesystem, "rifleskeleton36">();
auto &rifleskeleton37 = InitGlobalVariable<particlesystem, "rifleskeleton37">();
auto &rifleskeleton39 = InitGlobalVariable<particlesystem, "rifleskeleton39">();
auto &rifleskeleton43 = InitGlobalVariable<particlesystem, "rifleskeleton43">();
auto &rifleskeleton50 = InitGlobalVariable<particlesystem, "rifleskeleton50">();
auto &rifleskeleton55 = InitGlobalVariable<particlesystem, "rifleskeleton55">();

// bullet shot stats
auto &shotdistanceshow = InitGlobalVariable<std::int32_t, "shotdistanceshow">();
auto &shotdistance = InitGlobalVariable<float, "shotdistance">();
auto &shotlife = InitGlobalVariable<float, "shotlife">();
auto &shotricochet = InitGlobalVariable<std::int32_t, "shotricochet">();

auto &weaponsingame = InitGlobalVariable<std::int32_t, "weaponsingame">();

auto &botpath = InitGlobalVariable<twaypoints, "botpath">();

auto &ticks = InitGlobalVariable<std::int32_t, "ticks">();
auto &tickspersecond = InitGlobalVariable<std::int32_t, "tickspersecond">();
auto &frames = InitGlobalVariable<std::int32_t, "frames">();
auto &framespersecond = InitGlobalVariable<std::int32_t, "framespersecond">();
auto &ticktime = InitGlobalVariable<std::int32_t, "ticktime">();
auto &ticktimelast = InitGlobalVariable<std::int32_t, "ticktimelast">();
auto &goalticks = InitGlobalVariable<std::int32_t, "goalticks">();
auto &bullettimetimer = InitGlobalVariable<std::int32_t, "bullettimetimer">();

auto &oldspritepos =
    InitGlobalVariable<PascalArray<PascalArray<tvector2, 0, max_oldpos>, 1, max_sprites>,
                       "oldspritepos">();

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
auto &map = InitGlobalVariable<tpolymap, "map">();
auto &custommodchecksum = InitGlobalVariable<tsha1digest, "custommodchecksum">();
auto &gamemodchecksum = InitGlobalVariable<tsha1digest, "gamemodchecksum">();
auto &mapchecksum = InitGlobalVariable<tsha1digest, "mapchecksum">();
auto &mapindex = InitGlobalVariable<std::int32_t, "mapindex">();
auto &sortedplayers = InitGlobalVariable<PascalArray<tkillsort, 1, max_sprites>, "sortedplayers">();

auto &sprite = InitGlobalVariable<PascalArray<tsprite, 1, max_sprites>, "sprite">();
auto &bullet = InitGlobalVariable<PascalArray<tbullet, 1, max_bullets>, "bullet">();
auto &thing = InitGlobalVariable<PascalArray<tthing, 1, max_things>, "thing">();

auto &voteactive = InitGlobalVariable<bool, "voteactive">();
auto &votetype = InitGlobalVariable<std::uint8_t, "votetype">();
auto &votetarget = InitGlobalVariable<std::string, "votetarget">();
auto &votestarter = InitGlobalVariable<std::string, "votestarter">();
auto &votereason = InitGlobalVariable<std::string, "votereason">();
auto &votetimeremaining = InitGlobalVariable<std::int32_t, "votetimeremaining">();
auto &votenumvotes = InitGlobalVariable<std::uint8_t, "votenumvotes">();
auto &votemaxvotes = InitGlobalVariable<std::uint8_t, "votemaxvotes">();
auto &votehasvoted = InitGlobalVariable<PascalArray<bool, 1, max_sprites>, "votehasvoted">();
auto &votecooldown =
    InitGlobalVariable<PascalArray<std::int32_t, 1, max_sprites>, "votecooldown">();
auto &votekickreasontype = InitGlobalVariable<bool, "votekickreasontype">();

auto &pingticksadd = InitGlobalVariable<std::int32_t, "pingticksadd">();

auto &guns = InitGlobalVariable<GunsDescription, "guns">();
auto &defaultguns = InitGlobalVariable<GunsDescription, "defaultguns">();
auto &defaultwmchecksum = InitGlobalVariable<std::uint64_t, "defaultwmchecksum">();
auto &loadedwmchecksum = InitGlobalVariable<std::uint64_t, "loadedwmchecksum">();

auto &demorecorder = InitGlobalVariable<tdemorecorder<>, "demorecorder">();
auto &rsize = InitGlobalVariable<std::uint64_t, "rsize">();
auto &freecam = InitGlobalVariable<std::uint8_t, "freecam">();
auto &notexts = InitGlobalVariable<std::uint8_t, "notexts">();

auto &gamelog = InitGlobalVariable<tstringlist *, "gamelog">();
auto &consolelogfilename = InitGlobalVariable<std::string, "consolelogfilename">();

auto &maintickcounter = InitGlobalVariable<std::int32_t, "maintickcounter">();
} // namespace
