#include "FileUtility.hpp"
#include "PhysFSExt.hpp"
#include "misc/PortUtils.hpp"
#include "port_utils/NotImplemented.hpp"

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
    *claimed = 1;
    return new Memory();
  }

  static PHYSFS_EnumerateCallbackResult Enumerate(void *opaque, const char *dirname,
                                                  PHYSFS_EnumerateCallback cb, const char *origdir,
                                                  void *callbackdata)
  {
    NotImplemented("fsMem");
    return {};
  }
  static PHYSFS_Io *OpenRead(void *opaque, const char *fnm)
  {
    NotImplemented("fsMem");
    return {};
  }

  static PHYSFS_Io *OpenWrite(void *opaque, const char *filename)
  {
    NotImplemented("fsMem");
    return {};
  }

  static PHYSFS_Io *OpenAppend(void *opaque, const char *filename)
  {
    NotImplemented("fsMem");
    return {};
  }
  static int Remove(void *opaque, const char *filename)
  {
    NotImplemented("fsMem");
    return {};
  }
  static int Mkdir(void *opaque, const char *filename)
  {
    NotImplemented("fsMem");
    return {};
  }
  static int Stat(void *opaque, const char *fn, PHYSFS_Stat *stat)
  {
    NotImplemented("fsMem");
    return {};
  }
  static void CloseArchive(void *opaque) { NotImplemented("fsMem"); }
};
static PHYSFS_Archiver s_memoryArchiver = {.version = 0,
                                           .info = {.extension = "memory",
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
                                           

struct PHYSFS_IoMemory
{
  std::size_t position = 0;

  static PHYSFS_IoMemory* GetThis(PHYSFS_Io* io)
  {
    return reinterpret_cast<PHYSFS_IoMemory*>(io->opaque);
  }

  static PHYSFS_sint64 Read(struct PHYSFS_Io *io, void *buf, PHYSFS_uint64 len)
  {
    NotImplemented("fsMem");
    return {};
  }
  static PHYSFS_sint64 Write(struct PHYSFS_Io *io, const void *buffer, PHYSFS_uint64 len)
  {
    NotImplemented("fsMem");
    return {};
  }

  static int Seek(struct PHYSFS_Io *io, PHYSFS_uint64 offset)
  {
    auto t = GetThis(io);
    t->position = offset;
    return 1;
  }
  static PHYSFS_sint64 Tell(struct PHYSFS_Io *io)
  {
    NotImplemented("fsMem");
    return {};
  }
  static PHYSFS_sint64 Length(struct PHYSFS_Io *io)
  {
    NotImplemented("fsMem");
    return {};
  }

  static struct PHYSFS_Io *Duplicate(struct PHYSFS_Io *io)
  {
    NotImplemented("fsMem");
    return {};
  }
  static int Flush(struct PHYSFS_Io *io)
  {
    NotImplemented("fsMem");
    return {};
  }
  static void Destroy(struct PHYSFS_Io *io) { NotImplemented("fsMem"); }
};

static PHYSFS_Io s_PhysFSIOMemory = {.version = 0,
                                     .opaque = &s_PhysFSIOMemory,
                                     .read = PHYSFS_IoMemory::Read,
                                     .write = PHYSFS_IoMemory::Write,
                                     .seek = PHYSFS_IoMemory::Seek,
                                     .tell = PHYSFS_IoMemory::Tell,
                                     .length = PHYSFS_IoMemory::Length,
                                     .duplicate = PHYSFS_IoMemory::Duplicate,
                                     .flush = PHYSFS_IoMemory::Flush,
                                     .destroy = PHYSFS_IoMemory::Destroy};

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
  void *mem = new char[1024];
  if (item == "tmpfs.memory")
  {
    auto e = PHYSFS_setWriteDirIo(&s_PhysFSIOMemory, "/");
    SoldatAssert(e != 0);
  }
  auto e = PHYSFS_mountIo(&s_PhysFSIOMemory, item.data(), mount_point.data(), 0);
  SoldatAssert(e != 0);
}

FileUtility::File *FileUtility::Open(const std::string_view path, FileUtility::FileMode fm)
{
  PHYSFS_File *f = nullptr;
  switch (fm)
  {
  case FileMode::Read:
    f = PHYSFS_openRead(path.data());
    break;
  case FileMode::Write:
    f = PHYSFS_openWrite(path.data());
    break;
  }
  SoldatAssert(f != nullptr);
  static_assert(sizeof(f) == sizeof(File *));
  return reinterpret_cast<File *>(f);
}

std::size_t FileUtility::Read(File *file, std::byte *data, const std::size_t size)
{
  SoldatAssert(file);
  auto bytesRead = PHYSFS_readBytes(reinterpret_cast<PHYSFS_File *>(file), data, size);
  SoldatAssert(bytesRead > 0);
  SoldatAssert(bytesRead == size);
  return bytesRead;
}

bool FileUtility::Write(File *file, const std::byte *data, const std::size_t size)
{
  SoldatAssert(file);
  auto bytesWritten = PHYSFS_writeBytes(reinterpret_cast<PHYSFS_File *>(file), data, size);
  SoldatAssert(bytesWritten > 0);
  SoldatAssert(bytesWritten == size);
  return bytesWritten == size;
}

void FileUtility::Close(File *file)
{
  auto r = PHYSFS_close(reinterpret_cast<PHYSFS_File *>(file));
  SoldatAssert(r == 0);
}

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

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount memory and write file")
{
  FileUtility fu;
  fu.Mount("tmpfs.memory", "/fs_mem/");
  auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
  std::array<std::byte, 4> d = {std::byte(42), std::byte(42), std::byte(42), std::byte(40)};
  fu.Write(f, d.data(), 4);
  fu.Close(f);
  CHECK(false);
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount memory and read file") { CHECK(false); }

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount file system and read file") { CHECK(false); }

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount file system and write file") { CHECK(false); }

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount zip and read file") {}

TEST_CASE_FIXTURE(FileUtilityFixture, "Dump memory file system to actual filesystem") {}

} // namespace
