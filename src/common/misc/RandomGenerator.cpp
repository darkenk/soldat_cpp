#include "RandomGenerator.hpp"

#include <random>

#include "/home/darkenk/workspace/soldat/soldat_cpp/src/common/port_utils/NotImplemented.hpp"
#include "/home/darkenk/workspace/soldat/soldat_cpp/src/common/port_utils/Utilities.hpp"

std::default_random_engine generator;

void Randomize()
{
  NotImplemented();
}

auto Random() -> float
{
  std::uniform_real_distribution<> dis(0.0f, 1.0);
  return dis(generator);
}

auto Random(std::uint32_t range) -> std::int32_t
{
  if (range == 0)
  {
    return 0;
  }
  std::uniform_int_distribution<> distrib(0, range - 1);
  return distrib(generator);
}

auto randomrange(uint32_t lowerBound, uint32_t upperBound) -> int32_t
{
  std::uniform_int_distribution<> distrib(lowerBound, upperBound);
  return distrib(generator);
}
