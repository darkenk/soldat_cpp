// automatically converted
#include "LogFile.hpp"
#include "Console.hpp"
#include "FileUtility.hpp"
#include "Logging.hpp"

#include <sstream>

static constexpr std::int32_t max_logfilesize = 512000;

static std::string sGetCurrentDate(const std::string_view format)
{
  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream ss;
#if __EMSCRIPTEN__
  NotImplemented("wasm");
#else
  ss << std::put_time(std::localtime(&now), format.data());
#endif
  return ss.str();
}

void LogFile::Init(const std::string_view filenamePrefix)
{
  mLogNamePrefix = filenamePrefix;
  CreateNewLogFile(mLogNamePrefix);
}


void LogFile::CreateNewLogFile(const std::string &prefix)
{
  if (not mEnabled)
  {
    return;
  }

  std::string s2 {sGetCurrentDate("%y-%m-%d")};

  if (mLogLevel == 0)
  {
    mLogName = std::format("{}.txt", prefix);
  }
  else
  {
    mLogName = std::format("{}-{}.txt", prefix, s2);
    for (auto i = 1; mFileUtility.Exists(mLogName); i++)
    {
      mLogName = std::format("{}-{}-{:02d}.txt", prefix, s2, i);
    }
  }

  {
    std::lock_guard<std::mutex> lock(mLogLock);
    mLogList.clear();
  }

  auto logfile = mFileUtility.Open(mLogName, FileUtility::FileMode::Write);
  if (logfile == nullptr)
  {
    LogErrorG("File logging error {}", mLogName);
    NotImplemented("logging", "How to pass main console to LogFile?");
#if 0
    if constexpr (Config::IsServer((M)))
    {
      const std::string_view s = "File logging error (N): ";
      fu.Write(logfile, reinterpret_cast<const std::byte *>(s.data()), s.size());
    }
    else
    {
      GS::GetMainConsole().console("File logging error (N): ", debug_message_color);
    }
#endif
  }
  mFileUtility.Close(logfile);

  Log("   Console Log Started");
}

void LogFile::Log(const std::string_view s, bool withdate)
{
  LogTraceG("{}", s);

  if (not mEnabled)
  {
    return;
  }

  if (s.empty())
  {
    return;
  }

  if (mLogLevel == 0)
  {
    return;
  }

  {
    std::lock_guard lock(mLogLock);
    if (withdate)
    {
      mLogList.emplace_back(std::format("{} {}", sGetCurrentDate("%y/%m/%d %H:%M:%S"), s));
    }
    else
    {
      mLogList.emplace_back(s);
    }
  }

  if (mLogLevel > 1)
  {
    WriteToFile();
  }
}

void LogFile::WriteToFile()
{
  if (not mEnabled)
  {
    return;
  }

  auto logfile = mFileUtility.Open(mLogName, FileUtility::FileMode::Write);
  {
    std::lock_guard lock(mLogLock);
    for (auto &line : mLogList)
    {
      mFileUtility.Write(logfile, reinterpret_cast<const std::byte *>(line.c_str()), line.size());
      mFileUtility.Write(logfile, reinterpret_cast<const std::byte *>("\n"), 1);
    }
    mLogList.clear();
  }
  mFileUtility.Close(logfile);
}

void LogFile::CreateNewLogIfCurrentLogIsTooBig()
{
  if (mFileUtility.Size(mLogName) <= max_logfilesize)
  {
    return;
  }
  CreateNewLogFile(mLogNamePrefix);
}

// TEST
#include <doctest.h>

class LogFileFixture
{
public:
  FileUtility mockFileUtility;
  tstringlist logList;
  std::string logName = "/user/testlog";
  LogFile logFile {mockFileUtility};
  LogFileFixture()
  {
    mockFileUtility.Mount("tmpfs.memory", "/user");
    mockFileUtility.MkDir("/user/logs");
    mockFileUtility.MkDir("/user/logs/kills");
    logFile.SetLogLevel(1);
    logFile.Enable(true);
    logFile.Init(logName);
  }
  ~LogFileFixture() { mockFileUtility.Unmount("tmpfs.memory"); }

  std::string ReadFile(const std::string_view filename)
  {
    auto file = mockFileUtility.Open(filename, FileUtility::FileMode::Read);
    std::string content;
    auto size = mockFileUtility.Size(filename);
    content.resize(size);
    mockFileUtility.Read(file, reinterpret_cast<std::byte *>(content.data()), size);
    mockFileUtility.Close(file);
    return content;
  }
};

TEST_SUITE("LogFile")
{
  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_LogEnableFalse_DoesNothing")
  {
    LogFile logFile(mockFileUtility);
    logFile.Enable(false);
    logFile.Init("/user/logfile");
    logFile.Log("Test log entry", false);
    CHECK_EQ("", logFile.GetLogName());
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_EmptyLogEntry_DoesNothing")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(2);
    logFile.Log("", false);
    CHECK_EQ(0, mockFileUtility.Size(logFile.GetLogName()));
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_LogLevelZero_DoesNothing")
  {
    LogFile logFile(mockFileUtility);
    logFile.Enable(true);
    logFile.SetLogLevel(0);
    logFile.Init(logName);
    logFile.Log("Test log entry", false);
    logFile.WriteToFile();
    CHECK_EQ(0, mockFileUtility.Size(logFile.GetLogName()));
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_ValidLogEntry_AddsToLogFile")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(2);
    logFile.Log("Test log entry", false);
    std::string content = ReadFile(logFile.GetLogName());
    CHECK(content.find("Test log entry") != std::string::npos);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_WithDate_AddsTimestampToLogFile")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(2);
    logFile.Log("Test log entry", true);
    std::string content = ReadFile(logFile.GetLogName());
    CHECK(content.find("Test log entry") != std::string::npos);
    CHECK(content.find("/") != std::string::npos); // Check for date format
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_LogLevelGreaterThanOne_WritesLogFile")
  {
    logFile.Init(logName);
    logFile.Enable(true);
    logFile.SetLogLevel(2);
    logFile.Log("Test log entry", false);
    CHECK(mockFileUtility.Exists(logFile.GetLogName()));
  }

  TEST_CASE_FIXTURE(LogFileFixture, "CreateNewLogIfCurrentLogIsTooBig_FileSizeLessThanMax_DoesNothing")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(0);
    logFile.Init(logName);
    logFile.SetLogLevel(1);
    std::string initialLogName{logFile.GetLogName()};
    logFile.Log(std::string(max_logfilesize - 1, 'a'), false);
    logFile.WriteToFile();
    logFile.CreateNewLogIfCurrentLogIsTooBig();
    CHECK_EQ(initialLogName, logFile.GetLogName());
    // max_logfilesize because of the newline character
    CHECK_EQ(mockFileUtility.Size(logFile.GetLogName()), max_logfilesize);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "CreateNewLogIfCurrentLogIsTooBig_FileSizeEqualToMax_CreatesNewLogFile")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(2);
    logFile.Init(logName);
    std::string prevLogName{logFile.GetLogName()};
    logFile.Log(std::string(max_logfilesize, 'a'), false);
    logFile.CreateNewLogIfCurrentLogIsTooBig();
    CHECK_GT(mockFileUtility.Size(prevLogName), 0);
    // there is welcome message in the log file
    CHECK_LT(mockFileUtility.Size(logFile.GetLogName()), 100);
    CHECK_NE(logFile.GetLogName(), prevLogName);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFileName_UniqueNameGeneratedWhenFileExists")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(1);
    std::string date {sGetCurrentDate("%y-%m-%d")};
    auto existing_log = std::format("/user/logfile-{}.txt", date);
    auto h = mockFileUtility.Open(existing_log, FileUtility::FileMode::Write);
    mockFileUtility.Close(h);

    std::string expectedName = std::format("/user/logfile-{}-01.txt", date);

    logFile.Init("/user/logfile");
    CHECK_EQ(logFile.GetLogName(), expectedName);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFileName_BaseNameUsedWhenLogLevelIsZero")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(0);
    logFile.Init(logName);
    std::string baseName = "/user/logfile";
    std::string expectedName = "/user/logfile.txt";
    logFile.Init(baseName);
    CHECK_EQ(logFile.GetLogName(), expectedName);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFileName_DateAppendedWhenLogLevelIsNonZero")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(1);
    std::string date {sGetCurrentDate("%y-%m-%d")};
    auto expectedName = std::format("/user/logfile-{}.txt", date);
    logFile.Init("/user/logfile");
    CHECK_EQ(logFile.GetLogName(), expectedName);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "CreateNewLogIfCurrentLogIsTooBig_UniqueNameGeneratedForNewLog")
  {
    logFile.Enable(true);
    logFile.SetLogLevel(1);
    logFile.Init(logName);

    std::string initialLogName{logFile.GetLogName()};

    logFile.Log(std::string(max_logfilesize, 'a'), false);
    logFile.WriteToFile();
    logFile.CreateNewLogIfCurrentLogIsTooBig();

    std::string date {sGetCurrentDate("%y-%m-%d")};
    auto expectedName = std::format("/user/testlog-{}-01.txt", date);
    auto expectedName_02 = std::format("/user/testlog-{}-02.txt", date);
    CHECK_EQ(initialLogName, expectedName);
    CHECK_EQ(logFile.GetLogName(), expectedName_02);
  }
}