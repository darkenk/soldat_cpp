#pragma once
#include <type_traits>

namespace Config
{

constexpr Module GetModule() noexcept
{
    return CLIENT_MODULE;
}

} // namespace Config
