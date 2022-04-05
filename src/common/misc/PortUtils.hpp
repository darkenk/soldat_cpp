#pragma once
#include "../port_utils/NotImplemented.hpp"
#include "../port_utils/SourceLocation.hpp"
#include "../port_utils/Utilities.hpp"
#include "Config.hpp"

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

void AssertImpl(const bool condition, const MsgLineWrapper &msg = "") requires(Config::IsDebug());

inline void AssertImpl(const bool /*condition*/,
                       const char * /*msg*/ = "") requires(Config::IsRelease())
{
}

} // namespace PortUtils

#define SoldatAssert(condition) PortUtils::AssertImpl(condition, #condition)
#define AssertL(condition, location) PortUtils::AssertImpl(condition, {#condition, location})
