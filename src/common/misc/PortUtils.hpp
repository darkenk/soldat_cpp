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

void AssertImpl(const bool condition, const MsgLineWrapper &msg);

template<bool enable>
void Assert(const bool condition, const MsgLineWrapper &msg = "")
{
  if constexpr(enable)
  {
    if (condition)
    {
      return;
    }
    AssertImpl(condition, msg);
  }
}

template<bool enable>
void Ensure(const bool condition)
{
  if constexpr(enable)
  {
    if (condition)
    {
      return;
    }
    AssertImpl(condition, "");
  }

}

} // namespace PortUtils

#define SoldatAssert(condition) PortUtils::Assert<Config::IsDebug()>(condition, #condition)
#define AssertL(condition, location) PortUtils::Assert<Config::IsDebug()>(condition, {#condition, location})
#define SoldatEnsure(condition) PortUtils::Ensure<Config::IsDebug()>(condition)
