// automatically converted
#pragma once

#include "Vector.hpp"
#include <array>
#include <cstdint>

struct tintersectionresult
{
    std::array<tvector2, 2> points;
    std::int8_t numintersections = 0;
};

tintersectionresult islineintersectingcircle(tvector2 line1, tvector2 line2, tvector2 circlecenter,
                                             float radius);
bool linecirclecollision(const tvector2 &startpoint, const tvector2 &endpoint,
                         const tvector2 &circlecenter, float radius, tvector2 &collisionpoint);
float pointlinedistance(const tvector2 &p1, const tvector2 &p2, const tvector2 &p3);
float angle2points(const tvector2 &p1, const tvector2 &p2);
float distance(float x1, float y1, float x2, float y2);
float sqrdist(float x1, float y1, float x2, float y2);
float sqrdist(const tvector2 &p1, const tvector2 &p2);
float distance(const tvector2 &p1, const tvector2 &p2);
std::int32_t greaterpowerof2(std::int32_t n);
std::int32_t roundfair(float value);
