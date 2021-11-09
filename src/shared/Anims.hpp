#pragma once

#include "Vector.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include <array>
#include <cstdint>
#include <string>

constexpr std::int32_t max_pos_index = 20;
constexpr std::int32_t max_frames_index = 40;

struct tframe
{
    PascalArray<tvector3, 1, max_pos_index> pos;
};

class tanimation
{
  public:
    std::int32_t id;
    PascalArray<tframe, 1, max_frames_index> frames;
    std::int32_t numframes;
    std::int32_t speed, count;
    std::int32_t currframe;
    bool loop;
    void doanimation();
    void loadfromfile(const std::string &filename);
    std::int32_t checksum();
};

void loadanimobjects(const std::string &moddir);

extern tanimation run, stand, runback;
extern tanimation jump, jumpside, roll, rollback;
extern tanimation fall;
extern tanimation crouch, crouchrun, crouchrunback;
extern tanimation reload, throw_, recoil, shotgun, barret, smallrecoil, aimrecoil, handsuprecoil;
extern tanimation clipin, clipout, slideback;
extern tanimation change;
extern tanimation throwweapon, weaponnone;
extern tanimation punch, reloadbow, melee;
extern tanimation cigar, match, smoke, wipe, groin, takeoff, victory, piss, mercy, mercy2, own;
extern tanimation prone, getup, pronemove;
extern tanimation aim, handsupaim;
