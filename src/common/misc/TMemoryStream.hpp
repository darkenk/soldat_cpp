#pragma once

#include "TStream.hpp"
#include <memory>

std::unique_ptr<TStream> ReadAsMemoryStream(const std::string_view &content);
