#pragma once

#include <memory>
#include <string_view>

#include "TStream.hpp"

std::unique_ptr<TStream> ReadAsMemoryStream(const std::string_view &content);
