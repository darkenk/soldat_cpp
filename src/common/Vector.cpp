// automatically converted
#include "Vector.hpp"
#include <cmath>

tvector2 vector2(MyFloat x, MyFloat y)
{
  return tvector2(x, y);
}

MyFloat vec2length2(const tvector2 &v)
{
  return v.x * v.x + v.y * v.y;
}

MyFloat vec2dot(const tvector2 &v1, const tvector2 &v2)
{
  return v1.x * v2.x + v1.y * v2.y;
}

pvector2 vec2scale(tvector2 &vout, const tvector2 &v, MyFloat s)
{
  vout.x = v.x * s;
  vout.y = v.y * s;
  return &vout;
}

pvector2 vec2normalize(tvector2 &vout, const tvector2 &v)
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

MyFloat vec3length(const tvector3 &v)
{
  return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

tvector2 tvector2::operator+(const tvector2 &v) const
{
  return tvector2(x + v.x, y + v.y);
}

tvector2 tvector2::operator-(const tvector2 &v) const
{
  return tvector2(x - v.x, y - v.y);
}
