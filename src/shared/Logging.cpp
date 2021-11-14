#include "Logging.hpp"

// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>
// clang-format on

void InitLogging()
{
    {
        auto log = spdlog::default_logger();
        log->set_level(spdlog::level::debug);
    }

    {
        auto log = spdlog::stdout_color_mt("audio");
        log->set_level(spdlog::level::info);
    }

    {
        auto log = spdlog::stdout_color_mt("net_msg");
        log->set_level(spdlog::level::info);
    }

    {
        auto log = spdlog::stdout_color_mt("physfs");
        log->set_level(spdlog::level::info);
    }

    {
        auto log = spdlog::stdout_color_mt("input");
        log->set_level(spdlog::level::debug);
    }

    {
        auto log = spdlog::stdout_color_mt("network");
        log->set_level(spdlog::level::debug);
    }

    {
        auto log = spdlog::stdout_color_mt("sprites");
        log->set_level(spdlog::level::trace);
    }
}
