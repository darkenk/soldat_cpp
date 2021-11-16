#pragma once

namespace Config
{

constexpr Module GetModule() noexcept
{
    return TEST_MODULE;
}

constexpr bool IsTest() noexcept
{
    return true;
}

} // namespace Config
