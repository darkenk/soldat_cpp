#include "NotImplemented.hpp"
// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>
// clang-format on

namespace PortUtilities::NotImplemented
{

void DefaultShowNotImplementedMessage(const std::string_view area, const std::string_view msg,
                                      const source_location &location)
{
    auto constexpr LOG_NI = "NotImplemented";
    auto logger = spdlog::get(LOG_NI);
    if (logger == nullptr)
    {
        logger = spdlog::stdout_color_mt(LOG_NI, spdlog::color_mode::always);
        auto sink = static_cast<spdlog::sinks::stdout_color_sink_mt *>(logger->sinks()[0].get());
        sink->set_color(spdlog::level::warn, sink->yellow);
    }
    if (msg.empty())
    {
        logger->warn("{}:{}: {}", location.file_name(), location.line(), location.function_name());
    }
    else
    {
        logger->warn("{}:{}: {}: {}", location.file_name(), location.line(),
                     location.function_name(), msg);
    }
}

} // namespace PortUtilities::NotImplemented
