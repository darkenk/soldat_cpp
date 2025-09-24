#include "Logging.hpp"

// clang-format off
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/common.h>
// clang-format on

void InitLogging()
{
	spdlog::set_formatter(std::make_unique<spdlog::pattern_formatter>("[%H:%M:%S.%e][%n][%L] %v"));
	{
		auto Log = spdlog::stdout_color_mt("global");
		spdlog::set_default_logger(Log);
		Log->set_level(spdlog::level::debug);
	}

	{
		auto Log = spdlog::stdout_color_mt("audio");
		Log->set_level(spdlog::level::info);
	}

	{
		auto Log = spdlog::stdout_color_mt("net_msg");
		Log->set_level(spdlog::level::info);
	}

	{
		auto Log = spdlog::stdout_color_mt("physfs");
		Log->set_level(spdlog::level::info);
	}

	{
		auto Log = spdlog::stdout_color_mt("input");
		Log->set_level(spdlog::level::debug);
	}

	{
		auto Log = spdlog::stdout_color_mt("network");
		Log->set_level(spdlog::level::debug);
	}

	{
		auto Log = spdlog::stdout_color_mt("sprites");
		Log->set_level(spdlog::level::trace);
	}

	{
		auto Log = spdlog::stdout_color_mt("gfx");
		Log->set_level(spdlog::level::info);
	}

	{
		auto Log = spdlog::stdout_color_mt("fs");
		Log->set_level(spdlog::level::info);
	}
}
