#include "FileUtility.hpp"
#include "Logging.hpp"
#include "PhysFSExt.hpp"
#include "misc/PortUtils.hpp"
#include "physfs.h"
#include "port_utils/NotImplemented.hpp"
#include <filesystem>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace
{
auto LOG = "fs";

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
    auto t = GetThis(io);
    return t->m_content->size();
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
  std::vector<std::string> m_directories;

  Memory(PHYSFS_Io *io, const char *name) : m_name{name}, m_io{io} { m_directories.push_back(""); }

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
    auto m = GetThis(opaque);
    auto &directories = m->m_directories;
    auto it = std::find(directories.begin(), directories.end(), filename);
    if (it == std::end(directories))
    {
      directories.push_back(filename);
    }
    return 1;
  }
  static int Stat(void *opaque, const char *fn, PHYSFS_Stat *stat)
  {
    if (strcmp("", fn) == 0)
    {
      stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
      return 1;
    }
    auto m = GetThis(opaque);
    if (std::end(m->m_directories) != std::find(m->m_directories.begin(), m->m_directories.end(), fn))
    {
      stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
      return 1;
    }
    if (std::end(m->m_files) != m->m_files.find(fn))
    {
      stat->filetype = PHYSFS_FILETYPE_REGULAR;
      return 1;
    }

    PHYSFS_setErrorCode(PHYSFS_ERR_NOT_FOUND);
    return 0;
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

FileUtility::FileUtility(const std::string_view rootPrefix): RootPrefix{rootPrefix}
{
  auto r = PhysFS_InitThreadSafe();
  if (!r)
  {
    LogError(LOG, "FS init failed {}", PHYSFS_getLastErrorCode());
  }
  SoldatAssert(r);
  if (r == 1)
  {
    r = PHYSFS_registerArchiver(&s_memoryArchiver);
  }
  SoldatAssert(r);
}

FileUtility::~FileUtility()
{
  auto r = PhysFS_DeinitThreadSafe();
  SoldatAssert(r);
}

bool FileUtility::Mount(const std::string_view item, const std::string_view mount_point)
{
  auto mp = ApplyRootPrefix(mount_point);
  if (item == "tmpfs.memory")
  {
    auto io = PHYSFS_IoMemory::Create(mp.c_str());
    auto e = PHYSFS_mountIo(io, item.data(), mp.c_str(), 0);
    SoldatAssert(e != 0);
    return e;
  }
  auto e = PHYSFS_mount(item.data(), mp.c_str(), 0);
  SoldatAssert(e != 0);

  std::string_view nmp = PHYSFS_getMountPoint(item.data());
  SoldatAssert(nmp.size() >= mount_point.size());
  nmp.remove_suffix(mount_point.size());
  RootPrefix = nmp;
  return e;
}

FileUtility::File *FileUtility::Open(const std::string_view path, FileUtility::FileMode fm)
{
  PHYSFS_File *f = nullptr;
  auto p = ApplyRootPrefix(path);
  switch (fm)
  {
  case FileMode::Read:
    f = PHYSFS_openRead(p.c_str());
    break;
  case FileMode::Write:
    f = PHYSFS_openWrite(p.c_str());
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

bool FileUtility::Exists(const std::string_view path)
{
  return PHYSFS_exists(ApplyRootPrefix(path).c_str()) != 0;
}

std::size_t FileUtility::Size(File *file)
{
  return PHYSFS_fileLength(reinterpret_cast<PHYSFS_File *>(file));
}

std::size_t FileUtility::Size(const std::string_view path)
{
  auto p = ApplyRootPrefix(path);
  SoldatAssert(Exists(p.c_str()));
  auto f = Open(p.c_str(), FileMode::Read);
  auto size = Size(f);
  Close(f);
  return size;
}

void FileUtility::Close(File *file)
{
  auto r = PHYSFS_close(reinterpret_cast<PHYSFS_File *>(file));
  SoldatAssert(r != 0);
}

bool FileUtility::MkDir(const std::string_view dirPath) { return PHYSFS_mkdir(ApplyRootPrefix(dirPath).c_str()); }

bool FileUtility::Copy(const std::string_view src, const std::string_view dst)
{
  if (Exists(dst))
  {
    return false;
  }
  auto input = Open(src, FileMode::Read);
  auto output = Open(dst, FileMode::Write);
  auto inputFileSize = Size(input);
  auto buffer = std::make_unique<std::byte[]>(inputFileSize);
  Read(input, buffer.get(), inputFileSize);
  Write(output, buffer.get(), inputFileSize);
  Close(output);
  Close(input);
  return true;
}

std::vector<std::uint8_t> FileUtility::ReadFile(const std::string_view path)
{
  SoldatAssert(not path.empty());
  LogDebug(LOG, "Loading file {}", path);
  std::vector<std::uint8_t> result;
  if (!Exists(path.data()))
  {
    LogWarn(LOG, "File does not exist {}", path);
    return result;
  }
  auto fh = Open(path, FileMode::Read);
  auto length = Size(fh);
  result.resize(length);
  auto read = Read(fh, reinterpret_cast<std::byte*>(result.data()), length);
  Close(fh);
  if (read == -1)
  {
    LogError(LOG, "Error while reading data");
    result.resize(0);
  }
  return result;
}

#if __EMSCRIPTEN__
std::string FileUtility::GetBasePath() { return "/game"; }
#else
std::string FileUtility::GetBasePath() { return PHYSFS_getBaseDir(); }
#endif

std::string FileUtility::GetPrefPath(const std::string_view postfix, const bool debugBuild)
{
  std::string prefPath{debugBuild ? PHYSFS_getBaseDir() : PHYSFS_getPrefDir("Soldat", "Soldat")};
  prefPath += postfix.data();
  if (!std::filesystem::exists(prefPath))
  {
    SoldatEnsure(std::filesystem::create_directories(prefPath));
  }
  SoldatAssert(std::filesystem::is_directory(prefPath));
  return prefPath;
}

// tests
#include <doctest/doctest.h>
#include <fstream>

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

TEST_CASE_FIXTURE(FileUtilityFixture, "FileUtility initialized twice does not crash")
{
  FileUtility fu1;
  FileUtility fu2;
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Create directory in memory")
{
  FileUtility fu;
  fu.Mount("tmpfs.memory", "/fs_mem");
  auto created = fu.MkDir("/fs_mem/test_directory");
  CHECK_EQ(true, created);
  created = fu.MkDir("/fs_mem/test_directory");
  CHECK_EQ(true, created);
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Create two directories in memory")
{
  FileUtility fu;
  fu.Mount("tmpfs.memory", "/fs_mem");
  auto created = fu.MkDir("/fs_mem/test_directory");
  CHECK_EQ(true, created);
  created = fu.MkDir("/fs_mem/test_directory/test_directory2");
  CHECK_EQ(true, created);
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Create directory in filesystem")
{
  auto testDirectory = fs::temp_directory_path() / "Soldat_test";
  fs::remove_all(testDirectory);
  fs::create_directories(testDirectory);
  FileUtility fu;
  fu.Mount(testDirectory.c_str(), "/fs_mem");
  auto created = fu.MkDir("/fs_mem/test_directory");
  CHECK_EQ(true, created);
  created = fu.MkDir("/fs_mem/test_directory");
  CHECK_EQ(true, created);
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Get base path returns path to directory with with exe")
{
  FileUtility fu;
  auto s = fu.GetBasePath();
  CHECK_NE("", s);
  CHECK_EQ(true, std::filesystem::is_directory(s));
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Get pref data returns path to directory with user settings")
{
  FileUtility fu;
  auto s = fu.GetPrefPath("test_pref");
  CHECK_EQ(s.substr(s.rfind('/') + 1), "test_pref");
  CHECK_EQ(true, std::filesystem::is_directory(s));
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Exists return false if file does not exist")
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
  CHECK_EQ(true, fu.Exists("/fs_mem/valid"));
  CHECK_EQ(false, fu.Exists("/fs_mem/invalid"));
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Return size of file")
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
    CHECK_EQ(4, fu.Size(f));
    fu.Close(f);
  }
  {
    CHECK_EQ(4, fu.Size("/fs_mem/valid"));
  }
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Copy file")
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
    auto copied = fu.Copy("/fs_mem/valid", "/fs_mem/copy");
    CHECK_EQ(true, copied);
    auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Read);
    CHECK_EQ(4, fu.Size(f));
    fu.Close(f);
  }
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Filesystem does not leak between two different FileUtility objects")
{
  FileUtility fu("/test1");
  fu.Mount("tmpfs.memory", "/fs_mem");
  constexpr auto TEST_DATA_SIZE = 4;
  std::array<std::byte, TEST_DATA_SIZE> testData = {std::byte(42), std::byte(42), std::byte(42),
                                                    std::byte(40)};
  auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
  auto r = fu.Write(f, testData.data(), TEST_DATA_SIZE);
  fu.Close(f);
  FileUtility fu2("/test2");
  fu2.Mount("tmpfs.memory", "/fs_mem");

  CHECK_EQ(true, fu.Exists("/fs_mem/valid"));
  CHECK_EQ(false, fu2.Exists("/fs_mem/valid"));
}

TEST_CASE_FIXTURE(FileUtilityFixture, "The same file can be mounted twice in different objects")
{
  // test soldat.smod, generated with xxd --include soldat.smod
  // contains:
  // client_test.txt
  // server_test.txt
  // shared_test.txt
  unsigned char soldat_smod[] = {
                                 0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0x24, 0x33,
                                 0x50, 0xf5, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x63, 0x6c,
                                 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d,
                                 0x2e, 0x89, 0x4f, 0xce, 0xc9, 0x4c, 0xcd, 0x2b, 0xe1, 0x02, 0x00, 0x50, 0x4b, 0x03, 0x04, 0x14,
                                 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xa7, 0xe8, 0x12, 0xba, 0x0e, 0x00, 0x00,
                                 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x5f,
                                 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d, 0x2e, 0x89, 0x2f, 0x4e, 0x2d,
                                 0x2a, 0x4b, 0x2d, 0xe2, 0x02, 0x00, 0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00,
                                 0x00, 0x00, 0x21, 0x00, 0xab, 0x34, 0x36, 0xb2, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
                                 0x0f, 0x00, 0x00, 0x00, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e,
                                 0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d, 0x2e, 0x89, 0x2f, 0xce, 0x48, 0x2c, 0x4a, 0x4d, 0xe1, 0x02,
                                 0x00, 0x50, 0x4b, 0x01, 0x02, 0x14, 0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21,
                                 0x00, 0x24, 0x33, 0x50, 0xf5, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63,
                                 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b,
                                 0x01, 0x02, 0x14, 0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xa7, 0xe8,
                                 0x12, 0xba, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76,
                                 0x65, 0x72, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b, 0x01, 0x02, 0x14,
                                 0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xab, 0x34, 0x36, 0xb2, 0x0e,
                                 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x5f,
                                 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00,
                                 0x03, 0x00, 0x03, 0x00, 0xb7, 0x00, 0x00, 0x00, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00};
  unsigned int soldat_smod_len = 382;

  FileUtility fu("/t1");
  auto testDir = fu.GetPrefPath("mount_test", true);
  std::filesystem::remove_all(testDir);
  // recreate directory
  testDir = fu.GetPrefPath("mount_test", true);
  {
    std::ofstream s(testDir + "/soldat.smod", std::ios_base::binary | std::ios_base::trunc);
    s.write((char*)soldat_smod, soldat_smod_len);
  }

  CHECK_EQ(true, fu.Mount(testDir + "/soldat.smod", "/"));
  CHECK_EQ(true, fu.Exists("/client_test.txt"));
  FileUtility fu2("/t2");
  CHECK_EQ(true, fu2.Mount(testDir + "/soldat.smod", "/"));
  CHECK_EQ(true, fu2.Exists("/client_test.txt"));
}

TEST_CASE_FIXTURE(FileUtilityFixture, "Filesystem does not leak between two different FileUtility objects")
{
  FileUtility fu("/test1");
  fu.Mount("tmpfs.memory", "/fs_mem");
  constexpr auto TEST_DATA_SIZE = 4;
  std::array<std::uint8_t, TEST_DATA_SIZE> testData = {42, 42, 42, 40};
  auto f = fu.Open("/fs_mem/valid", FileUtility::FileMode::Write);
  auto r = fu.Write(f, reinterpret_cast<std::byte*>(testData.data()), TEST_DATA_SIZE);
  fu.Close(f);

  auto data = fu.ReadFile("/fs_mem/valid");
  CHECK_EQ(testData.size(), data.size());
  CHECK_EQ(testData[0], data[0]);
  CHECK_EQ(testData[1], data[1]);
  CHECK_EQ(testData[2], data[2]);
  CHECK_EQ(testData[3], data[3]);
}

} // namespace
