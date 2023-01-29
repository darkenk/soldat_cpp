#include "FileUtility.hpp"
#include "PhysFSExt.hpp"
#include "misc/PortUtils.hpp"
#include "port_utils/NotImplemented.hpp"
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace
{

struct PHYSFS_IoMemory
{
  using FileContent = std::vector<std::byte>;

  static PHYSFS_IoMemory *GetThis(PHYSFS_Io *io)
  {
    return reinterpret_cast<PHYSFS_IoMemory *>(io->opaque);
  }

  static PHYSFS_Io *Create(const std::string &nodeName, FileContent *content = nullptr)
  {
    auto *io = new PHYSFS_Io();
    io->version = 0, io->opaque = new PHYSFS_IoMemory(nodeName, content);
    io->read = PHYSFS_IoMemory::Read;
    io->write = PHYSFS_IoMemory::Write;
    io->seek = PHYSFS_IoMemory::Seek;
    io->tell = PHYSFS_IoMemory::Tell;
    io->length = PHYSFS_IoMemory::Length;
    io->duplicate = PHYSFS_IoMemory::Duplicate;
    io->flush = PHYSFS_IoMemory::Flush;
    io->destroy = PHYSFS_IoMemory::Destroy;
    return io;
  }

  static PHYSFS_sint64 Read(struct PHYSFS_Io *io, void *buf, PHYSFS_uint64 len)
  {
    auto *iom = GetThis(io);
    auto &fileContent = *iom->m_content;
    auto &pos = iom->m_position;
    if ((pos + len) > fileContent.size())
    {
      len = fileContent.size() - pos;
    }
    auto readStart = std::begin(fileContent) + pos;
    std::copy(readStart, readStart + len, reinterpret_cast<std::byte *>(buf));
    pos += len;
    return len;
  }

  static PHYSFS_sint64 Write(struct PHYSFS_Io *io, const void *buffer, PHYSFS_uint64 len)
  {
    auto *iom = GetThis(io);
    auto &fileContent = *iom->m_content;
    auto &pos = iom->m_position;
    auto newSize = pos + len;
    if (newSize > fileContent.size())
    {
      fileContent.reserve(newSize);
    }
    std::copy(reinterpret_cast<const std::byte *>(buffer),
              reinterpret_cast<const std::byte *>(buffer) + len,
              std::inserter(fileContent, fileContent.begin() + pos));
    pos += len;
    return len;
  }

  static int Seek(struct PHYSFS_Io *io, PHYSFS_uint64 offset)
  {
    auto t = GetThis(io);
    t->m_position = offset;
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
    // do nothing?
    return 1;
  }
  static void Destroy(struct PHYSFS_Io *io)
  {
    delete GetThis(io);
    io->opaque = nullptr;
    delete io;
  }

private:
  std::size_t m_position = 0;
  std::string m_nodeName = "";
  FileContent *m_content = nullptr;
  bool m_ownsContent = false;

  PHYSFS_IoMemory(const std::string &nodeName, FileContent *content)
    : m_nodeName(nodeName), m_content(content)
  {
    if (m_content == nullptr)
    {
      m_content = new FileContent();
      m_ownsContent = true;
    }
  }

  ~PHYSFS_IoMemory()
  {
    if (m_ownsContent)
    {
      delete m_content;
    }
  }
};

struct Memory
{
  std::string m_name;
  std::string m_mountPoint;
  PHYSFS_Io *m_io{nullptr};
  std::unordered_map<std::string, std::vector<std::byte>> m_files;

  Memory(PHYSFS_Io *io, const char *name) : m_name{name}, m_io{io} {}

  static Memory *GetThis(void *opaque) { return reinterpret_cast<Memory *>(opaque); }

  static void *OpenArchive(PHYSFS_Io *io, const char *name, int forWrite, int *claimed)
  {
    *claimed = 1;
    return new Memory(io, name);
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
    auto m = GetThis(opaque);
    auto &files = m->m_files;
    auto f = files.find(fnm);
    if (f == std::end(files))
    {
      return nullptr;
    }
    return PHYSFS_IoMemory::Create(fnm, &f->second);
  }

  static PHYSFS_Io *OpenWrite(void *opaque, const char *filename)
  {
    auto m = GetThis(opaque);
    return PHYSFS_IoMemory::Create(filename, &m->m_files[filename]);
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
    auto m = GetThis(opaque);
    if (m->m_name == (std::string("/") + fn))
    {
      stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
    }
    return 1;
  }
  static void CloseArchive(void *opaque) { delete GetThis(opaque); }
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
} // namespace

struct FileUtility::File
{
};

FileUtility::FileUtility()
{
  auto r = PhysFS_InitThreadSafe();
  SoldatAssert(r);
  r = PHYSFS_registerArchiver(&s_memoryArchiver);
  SoldatAssert(r);
}

FileUtility::~FileUtility()
{
  auto r = PHYSFS_deinit();
  SoldatAssert(r);
}

void FileUtility::Mount(const std::string_view item, const std::string_view mount_point)
{
  if (item == "tmpfs.memory")
  {
    auto io = PHYSFS_IoMemory::Create(mount_point.data());
    auto e = PHYSFS_mountIo(io, item.data(), mount_point.data(), 0);
    SoldatAssert(e != 0);
    return;
  }
  if (fs::is_directory(item))
  {
    auto e = PHYSFS_mount(item.data(), mount_point.data(), 0);
    SoldatAssert(e != 0);
    return;
  }

  SoldatAssert(false);
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
  SoldatAssert(r != 0);
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

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount memory and write file and later read it")
{
  FileUtility fu;
  fu.Mount("tmpfs.memory", "/fs_mem");
  constexpr auto TEST_DATA_SIZE = 4;
  std::array<std::byte, TEST_DATA_SIZE> testData = {std::byte(42), std::byte(42), std::byte(42),
                                                    std::byte(40)};
  {
    auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
    auto r = fu.Write(f, testData.data(), TEST_DATA_SIZE);
    fu.Close(f);
    CHECK_EQ(true, r);
  }
  {
    auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Read);
    std::array<std::byte, TEST_DATA_SIZE> d = {};
    std::fill(std::begin(d), std::end(d), std::byte(0));
    auto r = fu.Read(f, d.data(), TEST_DATA_SIZE);
    fu.Close(f);
    CHECK_EQ(TEST_DATA_SIZE, r);
    CHECK_EQ(d, testData);
  }
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Mount file system write and read file")
{
  auto testDirectory = fs::temp_directory_path() / "Soldat_test";
  fs::remove_all(testDirectory);
  fs::create_directories(testDirectory);
  FileUtility fu;
  fu.Mount(testDirectory.c_str(), "/fs_mem");
  constexpr auto TEST_DATA_SIZE = 4;
  std::array<std::byte, TEST_DATA_SIZE> testData = {std::byte(42), std::byte(42), std::byte(42),
                                                    std::byte(40)};
  {
    auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
    auto r = fu.Write(f, testData.data(), TEST_DATA_SIZE);
    fu.Close(f);
    CHECK_EQ(true, r);
  }
  {
    auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Read);
    std::array<std::byte, TEST_DATA_SIZE> d = {};
    std::fill(std::begin(d), std::end(d), std::byte(0));
    auto r = fu.Read(f, d.data(), TEST_DATA_SIZE);
    fu.Close(f);
    CHECK_EQ(TEST_DATA_SIZE, r);
    CHECK_EQ(d, testData);
  }
}

} // namespace
