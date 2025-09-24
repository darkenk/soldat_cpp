#pragma once

#include <memory>
#include <string_view>

#include "TStream.hpp"
#include "../FileUtility.hpp"

class FFileUtility;

std::unique_ptr<TStream> ReadAsFileStream(FFileUtility& fs, const std::string_view& file);
