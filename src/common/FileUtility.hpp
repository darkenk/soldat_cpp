#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

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
  File *Open(const std::string_view path, FileMode fm);
  void Close(File *file);
  std::size_t Read(File *file, std::byte *data, const std::size_t size);
  bool Write(File *file, const std::byte *data, const std::size_t size);
  bool Exists(const std::string_view path);
  std::size_t Size(File *file);
  std::size_t Size(const std::string_view path);
  bool MkDir(const std::string_view dirPath);
  bool Copy(const std::string_view src, const std::string_view dst);

  std::vector<std::uint8_t> ReadFile(const std::string_view path);

  std::string GetBasePath();
  std::string GetPrefPath(const std::string_view postfix, const bool debugBuild = Config::IsDebug());

private:
  std::string RootPrefix;

  auto ApplyRootPrefix(const std::string_view path) const {
    return RootPrefix + (path.size() == 0 || path[0] != '/' ? "/" : "") + std::string(path);
  }
};
