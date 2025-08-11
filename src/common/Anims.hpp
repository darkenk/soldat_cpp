#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "Vector.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include "misc/TStream.hpp"

class TStream;

constexpr std::int32_t max_pos_index = 20;
constexpr std::int32_t max_frames_index = 40;

enum AnimationType : std::int32_t
{
  Stand = 0,
  Run = 1,
  RunBack = 2,
  Jump = 3,
  JumpSide = 4,
  Fall = 5,
  Crouch = 6,
  CrouchRun = 7,
  Reload = 8,
  Throw = 9,
  Recoil = 10,
  SmallRecoil = 11,
  Shotgun = 12,
  ClipOut = 13,
  ClipIn = 14,
  SlideBack = 15,
  Change = 16,
  ThrowWeapon = 17,
  WeaponNone = 18,
  Punch = 19,
  ReloadBow = 20,
  Barret = 21,
  Roll = 22,
  RollBack = 23,
  CrouchRunBack = 24,
  Cigar = 25,
  Match = 26,
  Smoke = 27,
  Wipe = 28,
  Groin = 29,
  Piss = 30,
  Mercy = 31,
  Mercy2 = 32,
  TakeOff = 33,
  Prone = 34,
  Victory = 35,
  Aim = 36,
  HandSupAim = 37,
  ProneMove = 38,
  GetUp = 39,
  AimRecoil = 40,
  HandSupRecoil = 41,
  Melee = 42,
  Own = 43,

  Last
};

struct tframe
{
  PascalArray<tvector3, 1, max_pos_index> pos;
};

class tanimation
{
public:
  //    tanimation(AnimationType animId = AnimationType::Aim): id(animId){}
  //    tanimation& operator=(const tanimation& other){ return *this;}

  std::int32_t id;
  PascalArray<tframe, 1, max_frames_index> frames;
  std::int32_t numframes;
  std::int32_t speed, count;
  std::int32_t currframe;
  bool loop;
  void DoAnimation();
  void loadfromfile(TStream &stream);
  std::int32_t checksum();
};
