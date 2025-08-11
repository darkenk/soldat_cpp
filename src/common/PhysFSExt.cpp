#include "PhysFSExt.hpp"

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <filesystem>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <mutex>
#include <utility>

#include "Logging.hpp"
#include "misc/TStream.hpp"
#include "misc/PortUtils.hpp"

auto LOG = "physfs";

class PhysFSStream : public TStream
{
public:
  explicit PhysFSStream(const std::string_view &filename)
  {
    Handle = PHYSFS_openRead(filename.data());
    SoldatAssert(Handle != nullptr);
  }

  ~PhysFSStream() override
  {
    auto errorCode = PHYSFS_close(Handle);
    SoldatAssert(errorCode != 0);
  }

  auto ReadLine(std::string &out) -> bool override
  {
    if (PHYSFS_eof(Handle) != 0)
    {
      return false;
    }
    PhysFS_ReadLn(Handle, out);
    return true;
  }

  void Reset() override
  {
    PHYSFS_seek(Handle, 0);
  }

private:
  PHYSFS_File *Handle;
};

void PhysFS_ReadLn(PHYSFS_File *fileHandle, std::string &line)
{
  char c;
  line = "";
  while ((PHYSFS_readBytes(fileHandle, &c, 1) != 0) && c != '\n')
  {
    if (c != '\r')
    {
      line += c;
    }
  }
}

auto PhysFS_readBuffer(const std::string_view &name) -> PhysFS_Buffer
{
  SoldatAssert(not name.empty());
  LogDebug(LOG, "Loading file {}", name);
  PhysFS_Buffer result;
  if (PHYSFS_exists(name.data()) == 0)
  {
    LogWarn(LOG, "File does not exist {}", name);
    return result;
  }
  auto *FileHandle = PHYSFS_openRead(name.data());
  if (FileHandle == nullptr)
  {
    LogError(LOG, "Cannot open file %s", name);
    return result;
  }
  auto length = PHYSFS_fileLength(FileHandle);
  result.resize(length);
  auto read = PHYSFS_readBytes(FileHandle, result.data(), length);
  PHYSFS_close(FileHandle);
  if (read == -1)
  {
    LogError(LOG, "Error while reading data");
    result.resize(0);
  }
  return result;
}

auto PhysFS_CopyFileFromArchive(const std::string_view &sourceFile,
                                const std::string_view &destination) -> bool
{
  if (std::filesystem::exists(destination.data()))
  {
    return false;
  }
  auto *inputFile = PHYSFS_openRead(sourceFile.data());
  auto *outputFile = std::fopen(destination.data(), "wb");
  if ((inputFile != nullptr) && (outputFile != nullptr))
  {
    std::array<std::byte, 1024> data;
    while (PHYSFS_eof(inputFile) == 0)
    {
      auto dataRead = PHYSFS_readBytes(inputFile, data.data(), data.size());
      std::fwrite(data.data(), dataRead, 1, outputFile);
    }
  }
  std::fclose(outputFile);
  PHYSFS_close(inputFile);
  return true;
}

auto PhysFS_ReadAsStream(const std::string_view &file) -> std::unique_ptr<TStream>
{
  if (PHYSFS_exists(file.data()) == 0)
  {
    return nullptr;
  }
  auto ret = std::make_unique<PhysFSStream>(file);
  return ret;
}

namespace
{
  std::mutex sInitMutex;
  std::atomic<std::uint32_t> sNoOfInstances;
}

auto PhysFS_InitThreadSafe() -> std::uint32_t
{
  std::lock_guard m(sInitMutex);
  if ((PHYSFS_isInit() != 0) || (PHYSFS_init(nullptr) != 0))
  {
    sNoOfInstances++;
  }
  return sNoOfInstances;
}

auto PhysFS_DeinitThreadSafe() -> bool
{
  std::lock_guard m(sInitMutex);
  sNoOfInstances--;
  if (sNoOfInstances > 0)
  {
    return true;
  }
  return PHYSFS_deinit() != 0;
}
