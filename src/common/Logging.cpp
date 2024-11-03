#include "Logging.hpp"

// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
// clang-format on

void InitLogging()
{
  spdlog::set_formatter(std::make_unique<spdlog::pattern_formatter>("[%H:%M:%S.%e][%n][%L] %v"));
  {
    auto log = spdlog::stdout_color_mt("global");
    spdlog::set_default_logger(log);
    log->set_level(spdlog::level::debug);
  }

  {
    auto log = spdlog::stdout_color_mt("audio");
    log->set_level(spdlog::level::info);
  }

  {
    auto log = spdlog::stdout_color_mt("net_msg");
    log->set_level(spdlog::level::debug);
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

  {
    auto log = spdlog::stdout_color_mt("gfx");
    log->set_level(spdlog::level::info);
  }

  {
    auto log = spdlog::stdout_color_mt("fs");
    log->set_level(spdlog::level::info);
  }
}
