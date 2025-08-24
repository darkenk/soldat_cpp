#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "misc/Config.hpp"

class FileUtility
{
public:
  enum class FileMode
  {
    Read,
    Write
  };

  struct File;

  FileUtility(const std::string_view rootPrefix = "");
  ~FileUtility();

  bool Mount(const std::string_view item, const std::string_view mount_point);
  void Unmount(const std::string_view item);
  File *Open(const std::string_view path, FileMode fm);
  static void Close(File *file);
  static std::size_t Read(File *file, std::byte *data, const std::size_t size);
  static bool Write(File *file, const std::byte *data, const std::size_t size);
  bool Exists(const std::string_view path);
  static std::size_t Size(File *file);
  std::size_t Size(const std::string_view path);
  bool MkDir(const std::string_view dirPath);
  bool Copy(const std::string_view src, const std::string_view dst);

  std::vector<std::uint8_t> ReadFile(const std::string_view path);

  static std::string GetBasePath();
  static std::string GetPrefPath(const std::string_view postfix,
                                 const bool debugBuild = Config::IsDebug());

private:
  std::string RootPrefix;

  auto ApplyRootPrefix(const std::string_view path) const {
    return RootPrefix + (path.size() == 0 || path[0] != '/' ? "/" : "") + std::string(path);
  }
};
