#pragma once
#include <ostream>
#include <spdlog/fmt/bundled/format.h>

typedef struct tvector2 *pvector2;
struct tvector2
{
    tvector2(float _x = 0.0f, float _y = 0.0f) : x(_x), y(_y)
    {
    }
    float x, y;
    tvector2 operator+(const tvector2 &v) const;
    tvector2 operator-(const tvector2 &v) const;
};

typedef struct tvector3 *pvector3;
struct tvector3
{
    tvector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : x(_x), y(_y), z(_z)
    {
    }
    float x, y, z;
};

tvector2 vector2(float x, float y);

float vec2length(tvector2 v);
float vec2length2(tvector2 v);
float vec2dot(tvector2 v1, tvector2 v2);
tvector2 vec2add(tvector2 v1, tvector2 v2);
tvector2 vec2subtract(tvector2 v1, tvector2 v2);
pvector2 vec2scale(tvector2 &vout, tvector2 v, float s);
pvector2 vec2normalize(tvector2 &vout, tvector2 v);

float vec3length(tvector3 v);
