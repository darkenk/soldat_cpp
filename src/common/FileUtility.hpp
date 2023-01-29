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

  void Mount(const std::string_view item, const std::string_view mount_point);
  File *Open(const std::string_view path, FileMode fm);
  std::size_t Read(File *file, std::byte *data, const std::size_t size);
  bool Write(File *file, const std::byte *data, const std::size_t size);
  
  void Close(File *file);

private:
};
