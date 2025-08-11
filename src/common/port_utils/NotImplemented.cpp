#include "NotImplemented.hpp"

// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <memory>
#include <string>
#include <vector>
// clang-format on

namespace PortUtilities::NotImplemented
{

void DefaultShowNotImplementedMessage([[maybe_unused]] const std::string_view area, const std::string_view msg,
                                      const std::string_view file, const std::string_view function,
                                      const std::int32_t line)
{
  auto constexpr LOG_NI = "NotImplemented";
  auto logger = spdlog::get(LOG_NI);
  if (logger == nullptr)
  {
    logger = spdlog::stdout_color_mt(LOG_NI, spdlog::color_mode::always);
    auto *sink = static_cast<spdlog::sinks::stdout_color_sink_mt *>(logger->sinks()[0].get());
    sink->set_color(spdlog::level::warn, sink->yellow);
  }
  if (msg.empty())
  {
    logger->warn("{}:{}: {}", file, function, line);
  }
  else
  {
    logger->warn("{}:{}: {}: {}", file, function, line, msg);
  }
}

} // namespace PortUtilities::NotImplemented
