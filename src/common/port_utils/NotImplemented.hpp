#pragma once

#include "SourceLocation.hpp"
#include "Utilities.hpp"

namespace PortUtilities::NotImplemented
{

void DefaultShowNotImplementedMessage(const std::string_view area, const std::string_view msg,
                                      const source_location &loc);

namespace Config
{
namespace defaults
{
constexpr auto DisabledAreas = std::to_array<std::string_view>({{}});
constexpr auto NotImplementedEnabled = true;
constexpr auto ShowNotImplementedMessage = DefaultShowNotImplementedMessage;
} // namespace defaults

using namespace defaults;
} // namespace Config
} // namespace PortUtilities::NotImplemented

#if __has_include(<NotImplemented.tweaks.hpp>)
#include <NotImplemented.tweaks.hpp>
#endif

namespace PortUtilities::NotImplemented
{

consteval bool IsDisabled(const std::string_view &area)
{
    return !Config::NotImplementedEnabled or
           std::find_if(std::begin(Config::DisabledAreas), std::end(Config::DisabledAreas),
                        [&area](auto &v) { return v == area; }) != std::end(Config::DisabledAreas);
};

template <int line, StringLiteral file, StringLiteral function>
class Wrapper
{
  public:
    static PU_ALWAYS_INLINE void NotImplemented(const char *area = "", const char *msg = "")
    {
        static bool fired = false;
        if (fired)
        {
            return;
        }
        fired = true;
        Config::ShowNotImplementedMessage(
            area, msg, source_location::current(GetRelativePath(file.value), function.value, line));
    }
};

consteval std::string_view GetArea()
{
    return "GENERIC";
}

consteval std::string_view GetArea(const std::string_view area, auto &...args)
{
    return area;
}

} // namespace PortUtilities::NotImplemented

#define NotImplemented(...)                                                                        \
    if constexpr (not PortUtilities::NotImplemented::IsDisabled(                                   \
                      PortUtilities::NotImplemented::GetArea(__VA_ARGS__)))                        \
    {                                                                                              \
        PortUtilities::NotImplemented::Wrapper<__LINE__, __FILE__, __FUNCTION__>::NotImplemented(  \
            __VA_OPT__(__VA_ARGS__));                                                              \
    }
