// automatically converted
#include "Vector.hpp"
#include <cmath>

tvector2 vector2(MyFloat x, MyFloat y)
{
    return tvector2(x, y);
}

template <typename T>
static T sqr(T v)
{
    return v * v;
}

MyFloat vec2length(const tvector2 &v)
{
    return std::sqrt(sqr(v.x) + sqr(v.y));
}

MyFloat vec2length2(const tvector2 &v)
{
    return sqr(v.x) + sqr(v.y);
}

MyFloat vec2dot(const tvector2 &v1, const tvector2 &v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

tvector2 vec2add(const tvector2 &v1, const tvector2 &v2)
{
    return v1 + v2;
}

tvector2 vec2subtract(const tvector2 &v1, const tvector2 &v2)
{
    return v1 - v2;
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
    return std::sqrt(sqr(v.x) + sqr(v.y) + sqr(v.z));
}

tvector2 tvector2::operator+(const tvector2 &v) const
{
    return tvector2(x + v.x, y + v.y);
}

tvector2 tvector2::operator-(const tvector2 &v) const
{
    return tvector2(x - v.x, y - v.y);
}
