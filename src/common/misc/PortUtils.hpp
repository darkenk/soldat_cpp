#pragma once

#if __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_lib_source_location)
#include <source_location>
using source_location = std::source_location;
#else
#include <experimental/source_location>
using source_location = std::experimental::source_location;
#endif
#include "Config.hpp"
#include <algorithm>
#include <array>
#include <csignal>

namespace PortUtils
{

class MsgLineWrapper
{
  public:
    constexpr MsgLineWrapper(const char *_msg,
                             const source_location &_location = source_location::current())
        : msg(_msg), location(_location)
    {
    }
    const char *msg;
    const source_location &location;
};

class NotImplementedArea
{
  public:
    NotImplementedArea() = delete;
    ~NotImplementedArea() = delete;
    NotImplementedArea &operator=(const NotImplementedArea &) = delete;

    enum Tag
    {
        AUDIO,
        GFX,
        SDL2,
        FILESYSTEM,
        STEAM,
        GAME,
        INPUT,
        LOCALIZATION,
        MAP,
        PARTS,
        CHECKSUM,
        NETWORK,
        OTHER
    };

    static constexpr bool IsEnabled(NotImplementedArea::Tag area)
    {
        return std::find_if(std::begin(AllowedTags), std::end(AllowedTags), [=](auto &v) {
                   return v.first == area && !v.second;
               }) == std::end(AllowedTags);
    }

  private:
    static constexpr auto AllowedTags =
        std::to_array<std::pair<Tag, bool>>({{NotImplementedArea::AUDIO, true},
                                             {NotImplementedArea::GFX, true},
                                             {NotImplementedArea::SDL2, true},
                                             {NotImplementedArea::FILESYSTEM, true},
                                             {NotImplementedArea::GAME, true},
                                             {NotImplementedArea::LOCALIZATION, true},
                                             {NotImplementedArea::MAP, true},
                                             {NotImplementedArea::PARTS, true},
                                             {NotImplementedArea::OTHER, true}});
};

template <size_t N>
struct StringLiteral
{
    constexpr StringLiteral(const char (&str)[N])
    {
        std::copy_n(str, N, value);
    }

    char value[N];
};

consteval const char *GetRelativePath(const std::string_view v)
{
    return v.substr(v.find("soldat_cpp") + sizeof("soldat_cpp")).data();
}

void NotImplementedImpl(const source_location &location,
                        std::string_view msg) requires(Config::IsDebug());

template <int Line, StringLiteral File, StringLiteral Function>
static inline void NotImplementedGeneric(NotImplementedArea::Tag area,
                                         std::string_view msg = "") requires(Config::IsDebug())
{
    if (!NotImplementedArea::IsEnabled(area))
    {
        return;
    }
    static auto fired = false;
    if (fired)
    {
        return;
    }
    fired = true;
    NotImplementedImpl(source_location::current(GetRelativePath(File.value), Function.value, Line), msg);
};

static inline void NotImplementedGeneric() requires(Config::IsRelease())
{
}

void AssertImpl(const bool condition, const MsgLineWrapper &msg = "") requires(Config::IsDebug());

inline void AssertImpl(const bool /*condition*/,
                       const char * /*msg*/ = "") requires(Config::IsRelease())
{
}

} // namespace PortUtils

using NITag = PortUtils::NotImplementedArea::Tag;

#define NotImplemented PortUtils::NotImplementedGeneric<__LINE__, __FILE__, __FUNCTION__>
#define SoldatAssert(condition) PortUtils::AssertImpl(condition, #condition)
#define AssertL(condition, location) PortUtils::AssertImpl(condition, {#condition, location})
