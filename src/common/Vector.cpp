// automatically converted
#include "Vector.hpp"
#include <cmath>

auto vector2(MyFloat x, MyFloat y) -> tvector2 { return tvector2(x, y); }

auto vec2length2(const tvector2 &v) -> MyFloat { return v.x * v.x + v.y * v.y; }

auto vec2dot(const tvector2 &v1, const tvector2 &v2) -> MyFloat
{
  return v1.x * v2.x + v1.y * v2.y;
}

auto vec2scale(tvector2 &vout, const tvector2 &v, MyFloat s) -> pvector2
{
  vout.x = v.x * s;
  vout.y = v.y * s;
  return &vout;
}

auto vec2normalize(tvector2 &vout, const tvector2 &v) -> pvector2
{
  MyFloat len = vec2length(v);
  if ((len < 0.001) && (len > -0.001))
  {
    vout.x = 0;
    vout.y = 0;
  }
  else
  {
    vout.x = v.x / len;
    vout.y = v.y / len;
  }
  return &vout;
}

auto vec3length(const tvector3 &v) -> MyFloat
{
  return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
