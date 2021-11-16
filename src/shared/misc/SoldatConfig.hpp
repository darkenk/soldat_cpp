#pragma once
#include "Config.hpp"

namespace Config
{

enum Module
{
    CLIENT_MODULE,
    SERVER_MODULE,
    TEST_MODULE,
    INVALID_MODULE
};

namespace defaults
{

constexpr Module GetModule() noexcept
{
    return INVALID_MODULE;
}

constexpr bool IsServer() noexcept
{
    return false;
}

constexpr bool IsClient() noexcept
{
    return false;
}

constexpr bool IsTest() noexcept
{
    return false;
}

} // namespace defaults

using namespace defaults;

} // namespace Config

#if __has_include(<ConfigModule.tweaks.hpp>)
#include <ConfigModule.tweaks.hpp>
#endif
