#pragma once
#include "Config.hpp"

namespace Config
{

enum Module
{
    CLIENT_MODULE,
    SERVER_MODULE,
    TEST_MODULE
};

constexpr bool IsServer() noexcept
{
    return true;
}

constexpr bool IsClient() noexcept
{
    return false;
}

template <Config::Module T>
using EnableInServer = Enable<T, Config::SERVER_MODULE>;

template <Config::Module T>
using EnableInClient = Enable<T, Config::CLIENT_MODULE>;

} // namespace Config

#if __has_include(<ConfigModule.tweaks.hpp>)
#include <ConfigModule.tweaks.hpp>
#endif
