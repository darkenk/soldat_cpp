#pragma once

// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/fmt/bundled/core.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
// clang-format on

void InitLogging();

template <typename... Args>
inline void LogTraceG(fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::trace(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogDebugG(fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogInfoG(fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogWarnG(fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogErrorG(fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogCriticalG(fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::critical(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogDebug(const std::string_view &logger, fmt::format_string<Args...> fmt,
                     Args &&...args)
{
  spdlog::get(logger.data())->debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogInfo(const std::string_view &logger, fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::get(logger.data())->info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogWarn(const std::string_view &logger, fmt::format_string<Args...> fmt, Args &&...args)
{
  spdlog::get(logger.data())->warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogError(const std::string_view &logger, fmt::format_string<Args...> fmt,
                     Args &&...args)
{
  spdlog::get(logger.data())->error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void LogCritical(const std::string_view &logger, fmt::format_string<Args...> fmt,
                        Args &&...args)
{
  spdlog::get(logger.data())->critical(fmt, std::forward<Args>(args)...);
}
