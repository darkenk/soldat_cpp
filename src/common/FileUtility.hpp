#pragma once

#include <memory>
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
  FileUtility();
  ~FileUtility();

  bool Mount(const std::string_view item, const std::string_view mount_point);
  File *Open(const std::string_view path, FileMode fm);
  void Close(File *file);
  std::size_t Read(File *file, std::byte *data, const std::size_t size);
  bool Write(File *file, const std::byte *data, const std::size_t size);
  bool MkDir(const std::string_view dirPath);

  std::string GetBasePath();
  std::string GetPrefPath(const std::string_view postfix, const bool debugBuild = Config::IsDebug());

private:
};
