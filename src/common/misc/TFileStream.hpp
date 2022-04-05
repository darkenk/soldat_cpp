#pragma once

#include "TStream.hpp"
#include <memory>

std::unique_ptr<TStream> ReadAsFileStream(const std::string_view &file);
