#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <string_view>
#include <vector>

#include "misc/PortUtilsSoldat.hpp"
#include "misc/PortUtils.hpp"

class FileUtility;

class Console
{
public:
  explicit Console(FileUtility* filesystem = nullptr, const std::int32_t newMessageWait = 0, const std::int32_t countMax = 254,
                   const std::int32_t scrollTickMax = 150, bool writeToFile = true)
    : mNewMessageWait(newMessageWait), mCountMax(std::min(countMax, 254)), mScrollTickMax(scrollTickMax),
      mWriteToFile(writeToFile), mFileSystem(filesystem)
  {
    SoldatAssert(mCountMax > 0);
    mTextMessage.resize(mCountMax);
    mTextMessageColor.resize(mCountMax);
    mNumMessage.resize(mCountMax);
  }
  [[nodiscard]] const std::string &GetTextMessage(const std::int32_t i) const
  {
    return mTextMessage[i - 1];
  }
  [[nodiscard]] std::uint32_t GetTextMessageColor(const std::int32_t i) const
  {
    return mTextMessageColor[i - 1];
  }
  [[nodiscard]] std::int32_t GetNumMessage(const std::int32_t i) const { return mNumMessage[i - 1]; }
  // scrolls - in ticks 60=1 sec}
  [[nodiscard]] std::int32_t GetNewMessageWait() const { return mNewMessageWait; }
  // resuming the scroll count down
  [[nodiscard]] std::int32_t GetCount() const { return mCount + 1; }
  void ResetCount() { mCount = -1; }
  void ConsoleAdd(const std::string_view what, std::int32_t col, std::int32_t num = -255);

protected:
  void ScrollConsole();

  std::int32_t mNewMessageWait = 0; // how long it waits after a new message before
  std::int32_t mCount = -1;
  std::int32_t mCountMax = 1;
  std::int32_t mScrollTickMax = 1; // how long the scroll count down is before it
  std::int32_t mScrollTick = 0;
  bool mWriteToFile = false;
  FileUtility* mFileSystem = nullptr;

  std::vector<std::string> mTextMessage;
  std::vector<std::uint32_t> mTextMessageColor;
  std::vector<std::int32_t> mNumMessage;
};

class ConsoleMain : public Console
{
public:
  explicit ConsoleMain(FileUtility* filesystem = nullptr, const std::int32_t newMessageWait = 0, const std::int32_t countMax = 254,
                       const std::int32_t scrollTickMax = 150, bool writeToFile = true)
    : Console(filesystem, newMessageWait, countMax, scrollTickMax, writeToFile)
  {
  }
  void Update(const bool killConsole = false);
  void SetBigConsole(Console *bigConsole) { mBigConsole = bigConsole; }
  void console(const std::string_view what, std::int32_t col);

private:
  Console *mBigConsole = nullptr;
};
