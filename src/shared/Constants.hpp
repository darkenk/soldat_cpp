#pragma once

#include "common/Constants.hpp"
#include <string_view>

namespace Constants
{
#if SERVER
constexpr std::int32_t MAX_PUSHTICK = 0;
#else
constexpr std::int32_t MAX_PUSHTICK = 125;
#endif

#if !SERVER
constexpr std::int32_t WEP_RESTRICT_WIDTH = 64;
constexpr std::int32_t WEP_RESTRICT_HEIGHT = 64;
constexpr std::int32_t GOS_RESTRICT_WIDTH = 16;
constexpr std::int32_t GOS_RESTRICT_HEIGHT = 16;
#endif

}; // namespace Constants

#include "Constants.cpp.h"
