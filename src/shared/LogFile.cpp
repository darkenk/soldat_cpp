// automatically converted
#include "LogFile.hpp"
#include "Constants.cpp.h"
#include "Cvar.hpp"
#include "common/Console.hpp"
#include "common/FileUtility.hpp"
#include "common/Logging.hpp"
#include "misc/GlobalSystems.hpp"

#include <sstream>

namespace
{
std::mutex loglock;
}

template <Config::Module M>
void newlogfile(FileUtility &fu, tstringlist *f, const std::string &name)
{
  if (not CVar::log_enable)
  {
    return;
  }
  SoldatAssert(f != nullptr);
  {
    std::lock_guard<std::mutex> lock(loglock);
    f->clear();
  }

  auto logfile = fu.Open(name, FileUtility::FileMode::Write);
  if (logfile == nullptr)
  {
    LogErrorG("File logging error {}", name);
    if constexpr (Config::IsServer((M)))
    {
      const std::string_view s = "File logging error (N): ";
      fu.Write(logfile, reinterpret_cast<const std::byte *>(s.data()), s.size());
    }
    else
    {
      GS::GetMainConsole().console("File logging error (N): ", debug_message_color);
    }
  }
  fu.Close(logfile);
}

template <Config::Module M>
void addlinetologfile(FileUtility &fu, tstringlist *f, const std::string &s,
                      const std::string &name, bool withdate)
{
  LogTraceG("{}", s);

  if (not f)
  {
    return;
  }

  if (not CVar::log_enable)
  {
    return;
  }

  if (s.empty())
  {
    return;
  }

  if (CVar::log_level == 0)
  {
    return;
  }

  {
    std::lock_guard lock(loglock);
    if (withdate)
    {
      std::string s2;

      auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      std::stringstream ss;
#if __EMSCRIPTEN__
      NotImplemented("wasm");
#else
      ss << std::put_time(std::localtime(&now), "%y/%m/%d %H:%M:%S");
#endif
      s2 = s2 + ' ' + ss.str();
      f->push_back(s2 + ' ' + s);
    }
    else
    {
      f->push_back(s);
    }
  }

  if (CVar::log_level > 1)
  {
    writelogfile(fu, f, name);
  }
}

template <Config::Module M>
void writelogfile(FileUtility &fu, tstringlist *f, const std::string &name)
{
  if (not CVar::log_enable)
  {
    return;
  }

  if (not f)
  {
    return;
  }

  if (f->size() > 1000000)
  {
    return;
  }

  auto logfile = fu.Open(name, FileUtility::FileMode::Write);
  {
    std::lock_guard lock(loglock);
    for (auto &line : *f)
    {
      fu.Write(logfile, reinterpret_cast<const std::byte *>(line.c_str()), line.size());
      fu.Write(logfile, reinterpret_cast<const std::byte *>("\n"), 1);
    }
    f->clear();
  }
  fu.Close(logfile);
}

template <Config::Module M>
void newlogfiles(FileUtility &fu)
{
  std::int32_t j;
  std::string s2;

  {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
#if __EMSCRIPTEN__
    NotImplemented("wasm");
#else
    ss << std::put_time(std::localtime(&now), "%y-%m-%d");
#endif
    s2 = ss.str();
  }

  std::string consolelogfilename = "/user/logs/consolelog-" + s2 + ".txt";
  for (auto i = 1; fu.Exists(consolelogfilename); i++)
  {
    consolelogfilename = "/user/logs/consolelog-" + s2 + "-" + std::to_string(i) + ".txt";
  }
  if (CVar::log_level == 0)
  {
    consolelogfilename = "/user/logs/consolelog.txt";
  }

  GetGameLogFilename() = consolelogfilename;

  newlogfile(fu, GetGameLog(), consolelogfilename);
  addlinetologfile(fu, GetGameLog(), "   Console Log Started", consolelogfilename);

  // TODO error logging once mainconsole is ready
  if constexpr (Config::IsServer((M)))
  {
    GetKillLogFilename<M>() = std::format("/user/logs/kills/killlog-{}.txt", s2.c_str());
    j = 0;
    while (fu.Exists(GetKillLogFilename<M>()))
    {
      j += 1;
      GetKillLogFilename<M>() = std::format("/user/logs/kills/killlog-{}-{:02d}.txt", s2, j);
    }
    newlogfile(fu, &GetKillLog<M>(), GetKillLogFilename<M>());
    addlinetologfile(fu, &GetKillLog<M>(), "   Kill Log Started", GetKillLogFilename<M>());
  }
}

template <Config::Module M>
tstringlist *GetGameLog()
{
  static tstringlist gamelog;
  return &gamelog;
}

template <Config::Module M>
std::string &GetGameLogFilename()
{
  static std::string filename;
  return filename;
}

template <Config::Module M>
tstringlist &GetKillLog()
  requires(Config::IsServer(M))
{
  static tstringlist killlog;
  return killlog;
}

template <Config::Module M>
std::string &GetKillLogFilename()
  requires(Config::IsServer(M))
{
  static std::string killlogfilename;
  return killlogfilename;
}

template void newlogfile(FileUtility &fu, tstringlist *f, const std::string &name);
template void writelogfile(FileUtility &fu, tstringlist *f, const std::string &name);
template void addlinetologfile(FileUtility &fu, tstringlist *f, const std::string &s,
                               const std::string &name, bool withdate);
template void newlogfiles(FileUtility &fu);

template tstringlist *GetGameLog();
template std::string &GetGameLogFilename();

// TEST
#include "Cvar.hpp"
#include "LogFile.hpp"
#include <doctest/doctest.h>

class LogFileFixture
{
public:
  FileUtility mockFileUtility;
  tstringlist logList;
  std::string logName = "/user/testlog.txt";
  LogFileFixture()
  {
    mockFileUtility.Mount("tmpfs.memory", "/user");
    mockFileUtility.MkDir("/user/logs");
    mockFileUtility.MkDir("/user/logs/kills");
  }
  ~LogFileFixture() { mockFileUtility.Unmount("tmpfs.memory"); }
};

TEST_SUITE("LogFile")
{
  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFile_LogEnableFalse_DoesNothing")
  {
    CVar::log_enable = false;
    newlogfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, logName);
    CHECK(logList.empty());
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFile_ValidLogList_ClearsLogList")
  {
    CVar::log_enable = true;
    logList.push_back("Existing log entry");
    newlogfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, logName);
    CHECK(logList.empty());
    CHECK(mockFileUtility.Exists(logName));
  }

  //  TODO "Need to extend FileUtility stub with option to throw errors on file open"
  TEST_CASE_FIXTURE(LogFileFixture,
                    "NewLogFile_FileOpenSuccess_WritesErrorToFile" * doctest::skip(true))
  {
    CVar::log_enable = true;
    newlogfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, logName);
    CHECK(mockFileUtility.Exists(logName));
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_LogEnableFalse_DoesNothing")
  {
    CVar::log_enable = false;
    addlinetologfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, "Test log entry", logName,
                                            false);
    CHECK(logList.empty());
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_NullLogList_DoesNothing")
  {
    CVar::log_enable = true;
    addlinetologfile<Config::CLIENT_MODULE>(mockFileUtility, nullptr, "Test log entry", logName,
                                            false);
    CHECK(logList.empty());
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_EmptyLogEntry_DoesNothing")
  {
    CVar::log_enable = true;
    addlinetologfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, "", logName, false);
    CHECK(logList.empty());
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_LogLevelZero_DoesNothing")
  {
    CVar::log_enable = true;
    CVar::log_level = 0;
    addlinetologfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, "Test log entry", logName,
                                            false);
    CHECK(logList.empty());
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_ValidLogEntry_AddsToLogList")
  {
    CVar::log_enable = true;
    CVar::log_level = 1;
    addlinetologfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, "Test log entry", logName,
                                            false);
    CHECK_EQ(logList.size(), 1);
    CHECK_EQ(logList.front(), "Test log entry");
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_WithDate_AddsTimestampToLogEntry")
  {
    CVar::log_enable = true;
    CVar::log_level = 1;
    addlinetologfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, "Test log entry", logName,
                                            true);
    CHECK_EQ(logList.size(), 1);
    CHECK(logList.front().find("Test log entry") != std::string::npos);
    CHECK(logList.front().find("/") != std::string::npos); // Check for date format
  }

  TEST_CASE_FIXTURE(LogFileFixture, "AddLineToLogFile_LogLevelGreaterThanOne_WritesLogFile")
  {
    CVar::log_enable = true;
    CVar::log_level = 2;
    addlinetologfile<Config::CLIENT_MODULE>(mockFileUtility, &logList, "Test log entry", logName,
                                            false);
    CHECK(logList.empty());
    CHECK(mockFileUtility.Exists(logName));
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_LogLevelZero_UsesDefaultLogFilename")
  {
    CVar::log_level = 0;
    newlogfiles<Config::CLIENT_MODULE>(mockFileUtility);
    CHECK_EQ(GetGameLogFilename<Config::CLIENT_MODULE>(), "/user/logs/consolelog.txt");
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_LogLevelNonZero_UsesTimestampedLogFilename")
  {
    CVar::log_level = 1;
    newlogfiles<Config::CLIENT_MODULE>(mockFileUtility);
    CHECK(GetGameLogFilename<Config::CLIENT_MODULE>().find("/user/logs/consolelog-") !=
          std::string::npos);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_ExistingLogFile_AppendsIndexToFilename")
  {
    CVar::log_level = 1;
    newlogfiles<Config::CLIENT_MODULE>(mockFileUtility);
    newlogfiles<Config::CLIENT_MODULE>(mockFileUtility);
    CHECK(GetGameLogFilename<Config::CLIENT_MODULE>().find("/user/logs/consolelog-") !=
          std::string::npos);
    CHECK(GetGameLogFilename<Config::CLIENT_MODULE>().find("-1.txt") != std::string::npos);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_CreatesNewLogFile")
  {
    CVar::log_enable = true;
    newlogfiles<Config::CLIENT_MODULE>(mockFileUtility);
    CHECK(mockFileUtility.Exists(GetGameLogFilename<Config::CLIENT_MODULE>()));
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_AddsConsoleLogStartedEntry")
  {
    CVar::log_enable = true;
    newlogfiles<Config::CLIENT_MODULE>(mockFileUtility);
    CHECK_EQ(GetGameLog<Config::CLIENT_MODULE>()->size(), 1);
    CHECK(GetGameLog<Config::CLIENT_MODULE>()->front().find("   Console Log Started") != std::string::npos);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_ServerModule_CreatesKillLogFile")
  {
    CVar::log_enable = true;
    newlogfiles<Config::SERVER_MODULE>(mockFileUtility);
    CHECK(mockFileUtility.Exists(GetKillLogFilename<Config::SERVER_MODULE>()));
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_ServerModule_AddsKillLogStartedEntry")
  {
    CVar::log_enable = true;
    newlogfiles<Config::SERVER_MODULE>(mockFileUtility);
    CHECK_EQ(GetKillLog<Config::SERVER_MODULE>().size(), 1);
    CHECK(GetKillLog<Config::SERVER_MODULE>().front().find("   Kill Log Started") != std::string::npos);
  }

  TEST_CASE_FIXTURE(LogFileFixture, "NewLogFiles_ServerModule_Adds1_IfExistingKillLogFile")
  {
    CVar::log_enable = true;
    newlogfiles<Config::SERVER_MODULE>(mockFileUtility);
    newlogfiles<Config::SERVER_MODULE>(mockFileUtility);
    CHECK(GetKillLogFilename<Config::SERVER_MODULE>().find("-01.txt") != std::string::npos);
    CHECK_EQ(GetKillLog<Config::SERVER_MODULE>().size(), 1);
    CHECK(GetKillLog<Config::SERVER_MODULE>().front().find("   Kill Log Started") != std::string::npos);
  }

}