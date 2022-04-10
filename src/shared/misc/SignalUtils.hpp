#pragma once

#include "common/Logging.hpp"
#include "common/port_utils/Utilities.hpp"
#include <csignal>
#include <cstdio>

inline void Abort()
{
  std::fflush(stdout);
  std::abort();
}

template <typename... Args>
PU_ALWAYS_INLINE void AbortIf(bool v, fmt::format_string<Args...> fmt, Args &&...args)
{
  if (!v) [[likely]]
  {
    return;
  }
  LogCriticalG(fmt, std::forward<Args>(args)...);
  Abort();
}
