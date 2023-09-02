#pragma once

#include "TStream.hpp"
#include "../FileUtility.hpp"
#include <memory>

std::unique_ptr<TStream> ReadAsFileStream(FileUtility &fs, const std::string_view &file);
