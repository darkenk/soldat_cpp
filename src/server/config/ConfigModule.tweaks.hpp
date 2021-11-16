#pragma once
#include <type_traits>

namespace Config
{

constexpr Module GetModule() noexcept
{
    return SERVER_MODULE;
}

constexpr bool IsServer() noexcept
{
    return true;
}

} // namespace Config
