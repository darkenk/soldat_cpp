#pragma once

#include "mechanics/Sprites.hpp"
#include <cstdint>

constexpr std::int32_t dist_away = 731;
constexpr std::int32_t dist_too_far = 730;
constexpr std::int32_t dist_very_far = 500;
constexpr std::int32_t dist_far = 350;
constexpr std::int32_t dist_rock_throw = 180;
constexpr std::int32_t dist_close = 95;
constexpr std::int32_t dist_very_close = 55;
constexpr std::int32_t dist_too_close = 35;
constexpr std::int32_t dist_collide = 20;
constexpr std::int32_t dist_stop_prone = 25;

std::int32_t checkdistance(float posa, float posb);
void simpledecision(std::uint8_t snum, const twaypoints &botpath);
void gotothing(std::uint8_t snum, std::uint8_t tnum);
void controlbot(tsprite &spritec, const twaypoints &botpath);
