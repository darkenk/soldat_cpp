#pragma once

#include "misc/TStream.hpp"
#include <memory>
#include <physfs.h>
#include <string>
#include <vector>

using PhysFS_Buffer = std::vector<std::uint8_t>;

PhysFS_Buffer PhysFS_readBuffer(const std::string_view &Name);
void PhysFS_ReadLn(PHYSFS_File *fileHandle, std::string &line);

bool PhysFS_CopyFileFromArchive(const std::string_view &sourceFile,
                                const std::string_view &destination);
std::unique_ptr<TStream> PhysFS_ReadAsStream(const std::string_view &file);
