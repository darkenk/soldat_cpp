#include "PortUtils.hpp"
// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>
// clang-format on
#include <csignal>

namespace PortUtils
{

void AssertImpl(const bool condition, const MsgLineWrapper &msg)
{
  if (condition)
  {
    return;
  }
  auto constexpr loggerName = "Assert";
  auto logger = spdlog::get(loggerName);
  if (logger == nullptr)
  {
    logger = spdlog::stdout_color_mt(loggerName, spdlog::color_mode::always);
    auto *sink = static_cast<spdlog::sinks::stdout_color_sink_mt *>(logger->sinks()[0].get());
    sink->set_color(spdlog::level::err, sink->red);
  }
  logger->error("{}:{}: {}: {}", msg.location.file_name(), msg.location.line(),
                msg.location.function_name(), msg.msg);
  __asm__("int $3\n" : :);
}

} // namespace PortUtils
