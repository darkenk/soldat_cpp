#include "Calc.hpp"
#include <cmath>
#include <numbers>

static float sqr(float v)
{
    return v * v;
}

template <typename T>
static bool inrange(const T v, const T &min, const T &max)
{
    return v >= min && v <= max;
}

tintersectionresult islineintersectingcircle(tvector2 line1, tvector2 line2, tvector2 circlecenter,
                                             float radius)
{
    MyFloat a = 0.f, a1 = 0.f, b = 0.f, b1 = 0.f, c1 = 0.f, delta = 0.f, diffx = 0.f, diffy = 0.f,
            sqrtdelta = 0.f, a2 = 0.f, minx = 0.f, miny = 0.f, maxx = 0.f, maxy = 0.f, temp = 0.f;
    bool flipped;
    tvector2 intersect;

    tintersectionresult result;

    diffx = line2.x - line1.x;
    diffy = line2.y - line1.y;

    if ((std::abs(diffx) < 0.00001) && (std::abs(diffy) < 0.00001))
        // The line is a lie!
        // On a more serious note, let's not test the limit of floating point
        return result;

    // if the angle of the bullet is bigger than 45 degrees,
    // flip the coordinate system.
    // This algorithm deals with lines being nearly horizontal just fine,
    // but nearly vertical would cause a havoc, as vertical line is not a function.
    if (std::abs(diffy) > std::abs(diffx))
    {
        flipped = true;
        temp = line1.x;
        line1.x = line1.y;
        line1.y = temp;

        temp = line2.x;
        line2.x = line2.y;
        line2.y = temp;

        temp = circlecenter.x;
        circlecenter.x = circlecenter.y;
        circlecenter.y = temp;

        temp = diffx;
        diffx = diffy;
        diffy = temp;
    }
    else
        flipped = false;

    // Line equation: ax + b - y = 0. given x1, y1, x2, y2, let's calculate a and b
    // a = (y1 - y2)/(x1 - x2)
    a = diffy / diffx;
    // b := y - ax
    b = line1.y - a * line1.x;
    // Circle equation: (x - x1)^2 + (y - y1)^2 - r^2 = 0
    // Now we need to solve: (x - x1)^2 + (y - y1)^2 - r^2 = ax + b - y
    // Simplyfing above: (a^2 + 1)x^2 + 2(ab − ay1 − x1)x + (y1^2 − r^2 + x1^2 − 2by1b^2)=0
    // now, since this is a standard Ax^2 + Bx + C equation, we find x and y using
    // x = (-B +/- sqrt(B^2 - 4ac))/(2A)
    // A = (a^2 + 1)
    a1 = sqr(a) + 1;
    // B = 2(ab - ay1 - x1)
    b1 = 2 * (a * b - a * circlecenter.y - circlecenter.x);
    // C = y1^2 − r^2 + x1^2 − 2by1 + b^2
    c1 = sqr(circlecenter.y) - sqr(radius) + sqr(circlecenter.x) - 2 * b * circlecenter.y + sqr(b);
    // delta = B^2 - 4AC;
    delta = sqr(b1) - 4 * a1 * c1;
    // having x1 and x2 result, we can calculate y1 and y2 from y = a * x + b

    // if delta < 0, no intersection
    if (delta < 0)
        return result;

    if (line1.x < line2.x)
    {
        minx = line1.x;
        maxx = line2.x;
    }
    else
    {
        minx = line2.x;
        maxx = line1.x;
    }

    if (line1.y < line2.y)
    {
        miny = line1.y;
        maxy = line2.y;
    }
    else
    {
        miny = line2.y;
        maxy = line1.y;
    }

    // we don't care about a case of delta = 0 as it's extremaly rare,
    // also this will handle it fine, just less effecient
    sqrtdelta = sqrt(delta);
    a2 = 2 * a1;
    intersect.x = (-b1 - sqrtdelta) / a2;
    intersect.y = a * intersect.x + b;
    // we know that infinite line does intersect the circle, now let's see if our part does
    if (inrange(intersect.x, minx, maxx) && inrange(intersect.y, miny, maxy))
    {
        if (flipped)
        {
            temp = intersect.x;
            intersect.x = intersect.y;
            intersect.y = temp;
        }
        result.points[result.numintersections] = intersect;
        result.numintersections = result.numintersections + 1;
    }

    intersect.x = (-b1 + sqrtdelta) / a2;
    intersect.y = a * intersect.x + b;
    if (inrange(intersect.x, minx, maxx) && inrange(intersect.y, miny, maxy))
    {
        if (flipped)
        {
            temp = intersect.x;
            intersect.x = intersect.y;
            intersect.y = temp;
        }
        result.points[result.numintersections] = intersect;
        result.numintersections = result.numintersections + 1;
    }
    return result;
}

bool linecirclecollision(tvector2 startpoint, tvector2 endpoint, tvector2 circlecenter,
                         float radius, tvector2 &collisionpoint)
{
    float r2;
    tintersectionresult intersectionresult;

    bool result = false;
    r2 = sqr(radius);

    if (sqrdist(startpoint, circlecenter) <= r2)
    {
        collisionpoint = startpoint;
        return true;
        ;
    }

    if (sqrdist(endpoint, circlecenter) <= r2)
    {
        collisionpoint = endpoint;
        return true;
    }

    intersectionresult = islineintersectingcircle(startpoint, endpoint, circlecenter, radius);
    if (intersectionresult.numintersections > 0)
    {
        result = true;
        collisionpoint = intersectionresult.points[0];
        if ((intersectionresult.numintersections == 2) &&
            (sqrdist(intersectionresult.points[0], startpoint) >
             sqrdist(intersectionresult.points[1], startpoint)))
            collisionpoint = intersectionresult.points[1];
    }
    return result;
}

float pointlinedistance(tvector2 p1, tvector2 p2, tvector2 p3)
{
    float u, x, y;

    u = ((p3.x - p1.x) * (p2.x - p1.x) + (p3.y - p1.y) * (p2.y - p1.y)) /
        (sqr(p2.x - p1.x) + sqr(p2.y - p1.y));

    x = p1.x + u * (p2.x - p1.x);
    y = p1.y + u * (p2.y - p1.y);

    return std::sqrt(sqr(x - p3.x) + sqr(y - p3.y));
}

float angle2points(const tvector2 p1, tvector2 p2)
{
    float result = 0.0f;
    if ((p2.x - p1.x) != 0.0f)
    {
        if (p1.x > p2.x)
            result = std::atan((p2.y - p1.y) / (p2.x - p1.x)) + std::numbers::pi_v<float>;
        else
            result = std::atan((p2.y - p1.y) / (p2.x - p1.x));
    }
    else
    {
        if (p2.y > p1.y)
            result = std::numbers::pi_v<float> / 2.0f;
        else if (p2.y < p1.y)
            result = -std::numbers::pi_v<float> / 2.0f;
        else
            result = 0;
    }
    return result;
}

float distance(float x1, float y1, float x2, float y2)
{
    return std::sqrt(sqr(x1 - x2) + sqr(y1 - y2));
}

float distance(tvector2 p1, tvector2 p2)
{
    return std::sqrt(sqr(p1.x - p2.x) + sqr(p1.y - p2.y));
}

float sqrdist(float x1, float y1, float x2, float y2)
{
    return sqr(x1 - x2) + sqr(y1 - y2);
}

float sqrdist(tvector2 p1, tvector2 p2)
{
    return sqr(p1.x - p2.x) + sqr(p1.y - p2.y);
}

std::int32_t greaterpowerof2(std::int32_t n)
{
    // 2 ^ roundup(log2(n))
    return std::floor(std::pow(2, std::ceil(std::log2(n))));
}

// Rounds, but witout that "Banker's rule" that prefers even numbers.
std::int32_t roundfair(float value)
{
    return std::floor(value + 0.5f);
}
