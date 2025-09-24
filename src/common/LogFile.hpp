#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <string_view>

class FFileUtility;

class FLogFile
{
public:
	explicit FLogFile(FFileUtility& fu) : mFileUtility(fu) { }
	void Init(std::string_view filenamePrefix);
	void WriteToFile();
	void Log(std::string_view s, bool withdate = true);
	void CreateNewLogIfCurrentLogIsTooBig();
	[[nodiscard]] std::string_view GetLogName() const { return mLogName; }
	void Enable(bool enable) { mEnabled = enable; }
	void SetLogLevel(int level) { mLogLevel = level; }

private:
	FFileUtility& mFileUtility;
	std::string mLogName;
	std::string mLogNamePrefix;
	std::vector<std::string> mLogList;
	std::mutex mLogLock;
	bool mEnabled = true;
	int mLogLevel = 0;

	void CreateNewLogFile(const std::string& prefix);
};
