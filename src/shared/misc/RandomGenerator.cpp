#include "RandomGenerator.hpp"
#include "PortUtils.hpp"
#include <random>

std::default_random_engine generator;

void Randomize()
{
    NotImplemented(NITag::OTHER);
}

float Random()
{
    std::uniform_real_distribution<> dis(0.0f, 1.0);
    return dis(generator);
}

std::int32_t Random(std::uint32_t range)
{
    std::uniform_int_distribution<> distrib(0, range - 1);
    return distrib(generator);
}

int32_t randomrange(uint32_t lowerBound, uint32_t upperBound)
{
    std::uniform_int_distribution<> distrib(lowerBound, upperBound);
    return distrib(generator);
}
