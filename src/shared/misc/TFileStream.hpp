#pragma once

#include <memory>
#include "TStream.hpp"

std::unique_ptr<TStream> ReadAsFileStream(const std::string_view &file);
