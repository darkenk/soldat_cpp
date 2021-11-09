#pragma once

#include <csignal>
#include <cstdio>

inline void Abort()
{
    std::fflush(stdout);
    std::abort();
}
