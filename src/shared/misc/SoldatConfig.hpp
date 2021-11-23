#pragma once
#include "common/misc/Config.hpp"

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

} // namespace defaults

using namespace defaults;

} // namespace Config

#if __has_include(<ConfigModule.tweaks.hpp>)
#include <ConfigModule.tweaks.hpp>
#endif

namespace Config
{
constexpr bool IsServer(Config::Module module = Config::GetModule()) noexcept
{
    return module == Module::SERVER_MODULE;
}

constexpr bool IsClient(Config::Module module = Config::GetModule()) noexcept
{
    return module == Module::CLIENT_MODULE;
}

constexpr bool IsTest(Config::Module module = Config::GetModule()) noexcept
{
    return module == Config::Module::TEST_MODULE;
}
} // namespace Config
