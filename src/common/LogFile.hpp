#pragma once

#include <mutex>
#include <string>
#include <vector>

class FileUtility;

class LogFile
{
public:
  explicit LogFile(FileUtility &fu):
    mFileUtility(fu){}
  void Init(const std::string_view filenamePrefix);
  void WriteToFile();
  void Log(const std::string_view s, bool withdate = true);
  void CreateNewLogIfCurrentLogIsTooBig();
  [[nodiscard]] std::string_view GetLogName() const { return mLogName; }
  void Enable(bool enable) { mEnabled = enable; }
  void SetLogLevel(int level) { mLogLevel = level; }

private:
  FileUtility &mFileUtility;
  std::string mLogName;
  std::string mLogNamePrefix;
  std::vector<std::string> mLogList;
  std::mutex mLogLock;
  bool mEnabled = true;
  int mLogLevel = 0;

  void CreateNewLogFile(const std::string &prefix);
};
