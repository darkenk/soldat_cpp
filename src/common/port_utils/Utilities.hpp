#pragma once

#include <algorithm>

#ifndef __has_attribute
#define __has_attribute(x) 0 // Compatibility with non-clang compilers.
#endif

#if __has_attribute(always_inline)
#define PU_ALWAYS_INLINE __attribute__((always_inline))
#else
#define PU_ALWAYS_INLINE inline
#endif

namespace PortUtilities
{

template <std::size_t N>
struct StringLiteral
{
  constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

  char value[N];
};

consteval const char *GetRelativePath(const std::string_view v)
{
  return v.substr(v.find("soldat_cpp") + sizeof("soldat_cpp")).data();
}

} // namespace PortUtilities
