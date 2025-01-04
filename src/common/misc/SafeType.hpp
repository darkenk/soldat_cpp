#pragma once

#include "PortUtils.hpp"
#include <atomic>
#include <cmath>
#include <cstdint>

template <typename T, void (*FASSERT)(const T &)>
struct SafeType
{
  constexpr SafeType()
  {
    FASSERT(Value);
  }

  constexpr SafeType(const T value) : Value{value}
  {
    FASSERT(Value);
  }

  constexpr operator T() const
  {
    return Value;
  }

  SafeType operator++()
  {
    return ++Value;
  }

  SafeType operator++(int)
  {
    return Value++;
  }

  SafeType operator--()
  {
    return --Value;
  }

  SafeType operator--(int)
  {
    return Value--;
  }

  SafeType &operator+=(const SafeType &b)
  {
    Value += b.Value;
    FASSERT(Value);
    return *this;
  }

  SafeType &operator-=(const SafeType &b)
  {
    Value -= b.Value;
    FASSERT(Value);
    return *this;
  }

  SafeType &operator*=(const SafeType &b)
  {
    Value *= b.Value;
    FASSERT(Value);
    return *this;
  }

  SafeType &operator/=(const SafeType &b)
  {
    Value /= b.Value;
    FASSERT(Value);
    return *this;
  }

private:
  T Value;
};

template <typename T>
void EnsureNotNan(const T &v)
{
  SoldatAssert(not std::isnan(v));
  SoldatAssert(not std::isinf(v));
}

template <typename T, T Start, T End>
void EnsureInt(const T &v)
{
  SoldatAssert(v >= Start);
  SoldatAssert(v <= End);
}

using MyFloat = SafeType<float, EnsureNotNan<float>>;
using MyDouble = SafeType<double, EnsureNotNan<double>>;
using MyLongDouble = SafeType<long double, EnsureNotNan<long double>>;

template <std::uint64_t Start, std::uint64_t End>
using MyUInt64 = SafeType<std::uint64_t, EnsureInt<std::uint64_t, Start, End>>;
template <std::uint32_t Start, std::uint32_t End>
using MyUInt32 = SafeType<std::uint32_t, EnsureInt<std::uint32_t, Start, End>>;
template <std::uint16_t Start, std::uint16_t End>
using MyUInt16 = SafeType<std::uint16_t, EnsureInt<std::uint16_t, Start, End>>;
template <std::uint8_t Start, std::uint8_t End>
using MyUInt8 = SafeType<std::uint8_t, EnsureInt<std::uint8_t, Start, End>>;

template <std::int64_t Start, std::int64_t End>
using MyInt64 = SafeType<std::int64_t, EnsureInt<std::int64_t, Start, End>>;
template <std::int32_t Start, std::int32_t End>
using MyInt32 = SafeType<std::int32_t, EnsureInt<std::int32_t, Start, End>>;
template <std::int16_t Start, std::int16_t End>
using MyInt16 = SafeType<std::int16_t, EnsureInt<std::int16_t, Start, End>>;
template <std::int8_t Start, std::int8_t End>
using MyInt8 = SafeType<std::int8_t, EnsureInt<std::int8_t, Start, End>>;
