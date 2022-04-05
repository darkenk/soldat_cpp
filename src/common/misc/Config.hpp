#pragma once
#include <type_traits>

namespace Config
{

enum BuildConfiguration
{
  DEBUG,
  RELEASE,
  MASTER
};

namespace defaults
{

// DK_TODO: write implementation in CMAKE
// CMake should generate Config.tweakme.hpp wit proper configuration
constexpr BuildConfiguration GetConfiguration() noexcept
{
  return DEBUG;
}

} // namespace defaults

using namespace defaults;

constexpr bool IsDebug() noexcept
{
  return GetConfiguration() == DEBUG;
}

constexpr bool IsRelease() noexcept
{
  return GetConfiguration() == RELEASE;
}

constexpr bool IsMaster() noexcept
{
  return GetConfiguration() == MASTER;
}

namespace impl
{
template <auto T, auto U>
struct is_same : public std::false_type
{
};

template <auto T>
struct is_same<T, T> : public std::true_type
{
};

} // namespace impl

template <auto T, auto V>
using Enable = std::enable_if_t<impl::is_same<T, V>::value, bool>;

template <Config::BuildConfiguration T>
using EnableInDebug = Enable<T, Config::DEBUG>;

template <Config::BuildConfiguration T>
using EnableInRelease = Enable<T, Config::RELEASE>;

template <Config::BuildConfiguration T>
using EnableInMaster = Enable<T, Config::MASTER>;

} // namespace Config
