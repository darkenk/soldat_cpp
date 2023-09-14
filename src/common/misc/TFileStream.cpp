#include "TFileStream.hpp"
#include "TStream.hpp"
#include "../FileUtility.hpp"
#include <sstream>
#include <memory>

class TFileStream : public TStream
{
public:
  explicit TFileStream(FileUtility &fs, const std::string_view filename) : Stream()
  {
    std::size_t fileSize = 0;
    auto f = fs.Open(filename, FileUtility::FileMode::Read);
    fileSize = fs.Size(f);
    buff = std::make_unique<std::byte[]>(fileSize);
    fs.Read(f, buff.get(), fileSize);
    fs.Close(f);

#if __EMSCRIPTEN__
    Stream.str(std::string(reinterpret_cast<char*>(buff.get()), fileSize));
#else
    Stream.rdbuf()->pubsetbuf(reinterpret_cast<char*>(buff.get()), fileSize);
#endif


  }

  ~TFileStream()
  {
  }

  bool ReadLine(std::string &out) override
  {
    std::getline(Stream, out);
    return Stream.good();
  }

  void Reset() override
  {
    Stream.clear();
    Stream.seekg(0);
  }

private:
  std::istringstream Stream;
  std::unique_ptr<std::byte[]> buff;
};

std::unique_ptr<TStream> ReadAsFileStream(FileUtility &fs, const std::string_view &file)
{
  if (!fs.Exists(file))
  {
    return nullptr;
  }
  return std::make_unique<TFileStream>(fs, file);
}

// tests
#include <doctest/doctest.h>

namespace
{

constexpr std::string_view TestFile = "/fs_mem/test.ini";
constexpr std::string_view NonExistingFile = "/fs_mem/non_existing_file.ini";

class TFileStreamFixture
{
public:
  TFileStreamFixture(): FS("/TFileStream") {
    std::string_view testData= R"(; Some comments
[TestSection1]
TestEntry=1.0
)";
    FS.Mount("tmpfs.memory", "/fs_mem");
    auto f = FS.Open(TestFile, FileUtility::FileMode::Write);
    auto r = FS.Write(f, reinterpret_cast<const std::byte*>(testData.data()), testData.size());
    FS.Close(f);
  }
  ~TFileStreamFixture() {
  }

protected:
  TFileStreamFixture(const TFileStreamFixture &) = delete;
  FileUtility FS;
};

TEST_CASE_FIXTURE(TFileStreamFixture, "Mount memory and write file and later read it")
{
}


TEST_CASE_FIXTURE(TFileStreamFixture, "Return null if there is no file")
{
  auto fileStream = ReadAsFileStream(FS, NonExistingFile.data());
  CHECK_EQ(nullptr, fileStream);
}

TEST_CASE_FIXTURE(TFileStreamFixture, "New stream reads data frm the file beginning")
{
  auto fileStream = ReadAsFileStream(FS, TestFile);
  CHECK_NE(nullptr, fileStream);
  std::string line;
  auto success = fileStream->ReadLine(line);
  CHECK_EQ(true, success);
  CHECK_EQ(0, std::strcmp("; Some comments", line.c_str()));
}

TEST_CASE_FIXTURE(TFileStreamFixture, "ReadLine returns false if reaches eof")
{
  auto fileStream = ReadAsFileStream(FS, TestFile);
  CHECK_NE(nullptr, fileStream);
  std::string line;
  fileStream->ReadLine(line);
  fileStream->ReadLine(line);
  fileStream->ReadLine(line);
  auto success = fileStream->ReadLine(line);
  CHECK_EQ(false, success);
}

TEST_CASE_FIXTURE(TFileStreamFixture, "Reset allows ReadLine to read file from the beginning")
{
  auto fileStream = ReadAsFileStream(FS, TestFile);
  CHECK_NE(nullptr, fileStream);
  std::string line;
  auto success = fileStream->ReadLine(line);
  CHECK_EQ(true, success);
  CHECK_EQ(0, std::strcmp("; Some comments", line.c_str()));
  while(fileStream->ReadLine(line));
  CHECK_NE(0, std::strcmp("; Some comments", line.c_str()));
  fileStream->Reset();
  success = fileStream->ReadLine(line);
  CHECK_EQ(true, success);
  CHECK_EQ(0, std::strcmp("; Some comments", line.c_str()));
}

}
