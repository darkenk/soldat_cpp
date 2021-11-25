#pragma once

#include <string_view>
#include <thread>

bool SetCurrentThreadName(const std::string_view name);
bool SetThreadName(std::thread& thread, const std::string_view name);
