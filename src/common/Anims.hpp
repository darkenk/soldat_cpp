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
