#pragma once
#include "misc/SafeType.hpp"
#include <ostream>
#include <spdlog/fmt/bundled/format.h>

typedef struct tvector2 *pvector2;
struct tvector2
{
    tvector2(MyFloat _x = 0.0f, MyFloat _y = 0.0f) : x(_x), y(_y)
    {
    }
    MyFloat x;
    MyFloat y;
    tvector2 operator+(const tvector2 &v) const;
    tvector2 operator-(const tvector2 &v) const;
};

typedef struct tvector3 *pvector3;
struct tvector3
{
    tvector3(MyFloat _x = 0.0f, MyFloat _y = 0.0f, MyFloat _z = 0.0f) : x(_x), y(_y), z(_z)
    {
    }
    MyFloat x, y, z;
};

tvector2 vector2(MyFloat x, MyFloat y);

MyFloat vec2length(const tvector2 &v);
MyFloat vec2length2(const tvector2 &v);
MyFloat vec2dot(const tvector2 &v1, const tvector2 &v2);
tvector2 vec2add(const tvector2 &v1, const tvector2 &v2);
tvector2 vec2subtract(const tvector2 &v1, const tvector2 &v2);
pvector2 vec2scale(tvector2 &vout, const tvector2 &v, MyFloat s);
pvector2 vec2normalize(tvector2 &vout, const tvector2 &v);

MyFloat vec3length(const tvector3 &v);
