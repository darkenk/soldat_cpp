#include "FileUtility.hpp"
#include "PhysFSExt.hpp"

class FileUtility::FileUtilityPIMPL
{
public:
  FileUtilityPIMPL() {}
  ~FileUtilityPIMPL() {}
};

struct Memory
{

  static void *OpenArchive(PHYSFS_Io *io, const char *name, int forWrite, int *claimed)
  {
    return nullptr;
  }

  static PHYSFS_EnumerateCallbackResult Enumerate(void *opaque, const char *dirname,
                                                  PHYSFS_EnumerateCallback cb, const char *origdir,
                                                  void *callbackdata)
  {
  }
  static PHYSFS_Io *OpenRead(void *opaque, const char *fnm) {}

  static PHYSFS_Io *OpenWrite(void *opaque, const char *filename) {}

  static PHYSFS_Io *OpenAppend(void *opaque, const char *filename) {}
  static int Remove(void *opaque, const char *filename) {}
  static int Mkdir(void *opaque, const char *filename) {}
  static int Stat(void *opaque, const char *fn, PHYSFS_Stat *stat) {}
  static void CloseArchive(void *opaque) {}
};
static PHYSFS_Archiver s_memoryArchiver = {.version = 0,
                                           .info = {.extension = ".memory",
                                                    .description = "Memory filesystem",
                                                    .author = "DK",
                                                    .url = "mem://",
                                                    .supportsSymlinks = false},
                                           .openArchive = Memory::OpenArchive,
                                           .enumerate = Memory::Enumerate,
                                           .openRead = Memory::OpenRead,
                                           .openWrite = Memory::OpenWrite,
                                           .openAppend = Memory::OpenAppend,
                                           .remove = Memory::Remove,
                                           .mkdir = Memory::Mkdir,
                                           .stat = Memory::Stat,
                                           .closeArchive = Memory::CloseArchive};

struct FileUtility::File
{
};

#include <iostream>

FileUtility::FileUtility() : PIMPL{std::make_unique<FileUtilityPIMPL>()}
{
  SoldatAssert(PhysFS_InitThreadSafe());
  SoldatAssert(PHYSFS_registerArchiver(&s_memoryArchiver));
}

FileUtility::~FileUtility() { SoldatAssert(PHYSFS_deinit()); }

void FileUtility::Mount(const std::string_view item, const std::string_view mount_point)
{
  auto e = PHYSFS_mount(item.data(), mount_point.data(), 1);
  std::cout << PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()) << std::endl;
}

FileUtility::File &FileUtility::Open(const std::string_view path) {}

std::size_t FileUtility::Read(File &file) {}

bool FileUtility::Write(File &file, const std::byte *data, const std::size_t size) {}

void FileUtility::Close(File &file) {}

// tests
#include <doctest/doctest.h>

namespace
{

class FileUtilityFixture
{
public:
  FileUtilityFixture() {}
  ~FileUtilityFixture() {}

protected:
  FileUtilityFixture(const FileUtilityFixture &) = delete;
};

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount memory and read file") { CHECK(false); }

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount memory and write file")
{
  FileUtility fu;
  fu.Mount("tmpfs.memory", "/fs_mem/");
  auto f = fu.Open("/fs_mem/valid");
  std::array<std::byte, 4> d = {std::byte(42), std::byte(42), std::byte(42), std::byte(40)};
  fu.Write(f, d.data(), 4);
  fu.Close(f);
  CHECK(false);
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount file system and read file") { CHECK(false); }

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount file system and write file") { CHECK(false); }

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount zip and read file") {}

TEST_CASE_FIXTURE(FileUtilityFixture, "Dump memory file system to actual filesystem") {}

} // namespace
