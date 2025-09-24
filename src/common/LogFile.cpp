// automatically converted
#include "LogFile.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <format>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

#include "FileUtility.hpp"
#include "Logging.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include "port_utils/NotImplemented.hpp"

static constexpr std::int32_t kMaxLogfilesize = 512000;

static auto SGetCurrentDate(const std::string_view InFormat) -> std::string
{
	auto Now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::stringstream ss;
#if __EMSCRIPTEN__
	NotImplemented("wasm");
#else
	ss << std::put_time(std::localtime(&Now), InFormat.data());
#endif
	return ss.str();
}

void FLogFile::Init(const std::string_view InFilenamePrefix)
{
	mLogNamePrefix = InFilenamePrefix;
	CreateNewLogFile(mLogNamePrefix);
}

void FLogFile::CreateNewLogFile(const std::string& prefix)
{
	if (not mEnabled)
	{
		return;
	}

	std::string s2{ SGetCurrentDate("%y-%m-%d") };

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
		std::scoped_lock const Lock(mLogLock);
		mLogList.clear();
	}

	auto* Logfile = mFileUtility.Open(mLogName, FFileUtility::EFileMode::Write);
	if (Logfile == nullptr)
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
	FFileUtility::Close(Logfile);

	Log("   Console Log Started");
}

void FLogFile::Log(const std::string_view InS, bool withdate)
{
	LogTraceG("{}", InS);

	if (not mEnabled)
	{
		return;
	}

	if (InS.empty())
	{
		return;
	}

	if (mLogLevel == 0)
	{
		return;
	}

	{
		std::scoped_lock const Lock(mLogLock);
		if (withdate)
		{
			mLogList.emplace_back(std::format("{} {}", SGetCurrentDate("%y/%m/%d %H:%M:%S"), InS));
		}
		else
		{
			mLogList.emplace_back(InS);
		}
	}

	if (mLogLevel > 1)
	{
		WriteToFile();
	}
}

void FLogFile::WriteToFile()
{
	if (not mEnabled)
	{
		return;
	}

	auto* Logfile = mFileUtility.Open(mLogName, FFileUtility::EFileMode::Write);
	{
		std::scoped_lock const Lock(mLogLock);
		for (auto& Line : mLogList)
		{
			FFileUtility::Write(Logfile, reinterpret_cast<const std::byte*>(Line.c_str()), Line.size());
			FFileUtility::Write(Logfile, reinterpret_cast<const std::byte*>("\n"), 1);
		}
		mLogList.clear();
	}
	FFileUtility::Close(Logfile);
}

void FLogFile::CreateNewLogIfCurrentLogIsTooBig()
{
	if (mFileUtility.Size(mLogName) <= kMaxLogfilesize)
	{
		return;
	}
	CreateNewLogFile(mLogNamePrefix);
}

// TEST
#include <doctest/doctest.h>
#include <spdlog/fmt/bundled/core.h>

class FLogFileFixture
{
public:
	FFileUtility MockFileUtility;
	tstringlist LogList;
	std::string LogName = "/user/testlog";
	FLogFile LogFile{ MockFileUtility };
	FLogFileFixture()
	{
		MockFileUtility.Mount("tmpfs.memory", "/user");
		MockFileUtility.MkDir("/user/logs");
		MockFileUtility.MkDir("/user/logs/kills");
		LogFile.SetLogLevel(1);
		LogFile.Enable(true);
		LogFile.Init(LogName);
	}
	~FLogFileFixture() { MockFileUtility.Unmount("tmpfs.memory"); }

	auto ReadFile(const std::string_view InFilename) -> std::string
	{
		auto* File = MockFileUtility.Open(InFilename, FFileUtility::EFileMode::Read);
		std::string Content;
		auto Size = MockFileUtility.Size(InFilename);
		Content.resize(Size);
		FFileUtility::Read(File, reinterpret_cast<std::byte*>(Content.data()), Size);
		FFileUtility::Close(File);
		return Content;
	}
};

TEST_SUITE("LogFile")
{
	TEST_CASE_FIXTURE(FLogFileFixture, "AddLineToLogFile_LogEnableFalse_DoesNothing")
	{
		FLogFile LogFile(MockFileUtility);
		LogFile.Enable(false);
		LogFile.Init("/user/logfile");
		LogFile.Log("Test log entry", false);
		CHECK_EQ("", LogFile.GetLogName());
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "AddLineToLogFile_EmptyLogEntry_DoesNothing")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(2);
		LogFile.Log("", false);
		CHECK_EQ(0, MockFileUtility.Size(LogFile.GetLogName()));
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "AddLineToLogFile_LogLevelZero_DoesNothing")
	{
		FLogFile LogFile(MockFileUtility);
		LogFile.Enable(true);
		LogFile.SetLogLevel(0);
		LogFile.Init(LogName);
		LogFile.Log("Test log entry", false);
		LogFile.WriteToFile();
		CHECK_EQ(0, MockFileUtility.Size(LogFile.GetLogName()));
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "AddLineToLogFile_ValidLogEntry_AddsToLogFile")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(2);
		LogFile.Log("Test log entry", false);
		std::string const Content = ReadFile(LogFile.GetLogName());
		CHECK(Content.find("Test log entry") != std::string::npos);
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "AddLineToLogFile_WithDate_AddsTimestampToLogFile")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(2);
		LogFile.Log("Test log entry", true);
		std::string const Content = ReadFile(LogFile.GetLogName());
		CHECK(Content.find("Test log entry") != std::string::npos);
		CHECK(Content.find('/') != std::string::npos); // Check for date format
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "AddLineToLogFile_LogLevelGreaterThanOne_WritesLogFile")
	{
		LogFile.Init(LogName);
		LogFile.Enable(true);
		LogFile.SetLogLevel(2);
		LogFile.Log("Test log entry", false);
		CHECK(MockFileUtility.Exists(LogFile.GetLogName()));
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "CreateNewLogIfCurrentLogIsTooBig_FileSizeLessThanMax_DoesNothing")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(0);
		LogFile.Init(LogName);
		LogFile.SetLogLevel(1);
		std::string const InitialLogName{ LogFile.GetLogName() };
		LogFile.Log(std::string(kMaxLogfilesize - 1, 'a'), false);
		LogFile.WriteToFile();
		LogFile.CreateNewLogIfCurrentLogIsTooBig();
		CHECK_EQ(InitialLogName, LogFile.GetLogName());
		// max_logfilesize because of the newline character
		CHECK_EQ(MockFileUtility.Size(LogFile.GetLogName()), kMaxLogfilesize);
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "CreateNewLogIfCurrentLogIsTooBig_FileSizeEqualToMax_CreatesNewLogFile")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(2);
		LogFile.Init(LogName);
		std::string const PrevLogName{ LogFile.GetLogName() };
		LogFile.Log(std::string(kMaxLogfilesize, 'a'), false);
		LogFile.CreateNewLogIfCurrentLogIsTooBig();
		CHECK_GT(MockFileUtility.Size(PrevLogName), 0);
		// there is welcome message in the log file
		CHECK_LT(MockFileUtility.Size(LogFile.GetLogName()), 100);
		CHECK_NE(LogFile.GetLogName(), PrevLogName);
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "NewLogFileName_UniqueNameGeneratedWhenFileExists")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(1);
		std::string Date{ SGetCurrentDate("%y-%m-%d") };
		auto ExistingLog = std::format("/user/logfile-{}.txt", Date);
		auto* h = MockFileUtility.Open(ExistingLog, FFileUtility::EFileMode::Write);
		FFileUtility::Close(h);

		std::string const ExpectedName = std::format("/user/logfile-{}-01.txt", Date);

		LogFile.Init("/user/logfile");
		CHECK_EQ(LogFile.GetLogName(), ExpectedName);
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "NewLogFileName_BaseNameUsedWhenLogLevelIsZero")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(0);
		LogFile.Init(LogName);
		std::string const BaseName = "/user/logfile";
		std::string const ExpectedName = "/user/logfile.txt";
		LogFile.Init(BaseName);
		CHECK_EQ(LogFile.GetLogName(), ExpectedName);
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "NewLogFileName_DateAppendedWhenLogLevelIsNonZero")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(1);
		std::string Date{ SGetCurrentDate("%y-%m-%d") };
		auto ExpectedName = std::format("/user/logfile-{}.txt", Date);
		LogFile.Init("/user/logfile");
		CHECK_EQ(LogFile.GetLogName(), ExpectedName);
	}

	TEST_CASE_FIXTURE(FLogFileFixture, "CreateNewLogIfCurrentLogIsTooBig_UniqueNameGeneratedForNewLog")
	{
		LogFile.Enable(true);
		LogFile.SetLogLevel(1);
		LogFile.Init(LogName);

		std::string const InitialLogName{ LogFile.GetLogName() };

		LogFile.Log(std::string(kMaxLogfilesize, 'a'), false);
		LogFile.WriteToFile();
		LogFile.CreateNewLogIfCurrentLogIsTooBig();

		std::string Date{ SGetCurrentDate("%y-%m-%d") };
		auto ExpectedName = std::format("/user/testlog-{}-01.txt", Date);
		auto ExpectedName02 = std::format("/user/testlog-{}-02.txt", Date);
		CHECK_EQ(InitialLogName, ExpectedName);
		CHECK_EQ(LogFile.GetLogName(), ExpectedName02);
	}
}