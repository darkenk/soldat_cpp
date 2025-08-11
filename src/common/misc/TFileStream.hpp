#pragma once

#include <memory>
#include <string_view>

#include "TStream.hpp"
#include "../FileUtility.hpp"

class FileUtility;

std::unique_ptr<TStream> ReadAsFileStream(FileUtility &fs, const std::string_view &file);
