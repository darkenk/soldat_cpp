// IWYU pragma: private, include "common/misc/SoldatConfig.hpp"
#pragma once
#include <type_traits>

namespace Config
{

constexpr Module GetModule() noexcept
{
  return SERVER_MODULE;
}

} // namespace Config
