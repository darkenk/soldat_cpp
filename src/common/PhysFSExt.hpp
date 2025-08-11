#pragma once

#include <physfs.h>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <string_view>

#include "misc/TStream.hpp"

using PhysFS_Buffer = std::vector<std::uint8_t>;

PhysFS_Buffer PhysFS_readBuffer(const std::string_view &Name);
void PhysFS_ReadLn(PHYSFS_File *fileHandle, std::string &line);

bool PhysFS_CopyFileFromArchive(const std::string_view &sourceFile,
                                const std::string_view &destination);
std::unique_ptr<TStream> PhysFS_ReadAsStream(const std::string_view &file);

std::uint32_t PhysFS_InitThreadSafe();
bool PhysFS_DeinitThreadSafe();