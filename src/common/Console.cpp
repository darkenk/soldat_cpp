#include "Console.hpp"
#include "Logging.hpp"
#include "network/Net.hpp"

void Console::ScrollConsole()
{
  mScrollTick = 0;
  if (mCount < 0)
  {
    return;
  }
  for (std::int32_t x = 0; x < mCount; x++)
  {
    mTextMessageColor[x] = mTextMessageColor[x + 1];
    mTextMessage[x] = mTextMessage[x + 1];
    mNumMessage[x] = mNumMessage[x + 1]; // scroll the messages up 1
  }
  mTextMessage[mCount] = ""; // blank the last message
  mNumMessage[mCount] = 0;
  mCount -= 1;

}

void Console::ConsoleAdd(const std::string_view what, std::int32_t col, std::int32_t num)
{
  mCount += 1;
  mScrollTick = -mNewMessageWait;
  mTextMessage[mCount] = what;
  mTextMessageColor[mCount] = col;
  mNumMessage[mCount] = num;
  if (mCount == mCountMax - 1)
  {
    ScrollConsole();
  }
}

void ConsoleMain::Update(const bool killConsole)
{
  ++this->mScrollTick;
  if (this->mScrollTick != this->mScrollTickMax)
  {
    return;
  }
  this->ScrollConsole();
  if (!killConsole)
  {
    return;
  }
  if ((this->mCount > -1) && (this->mNumMessage[this->mCount] == -255))
    this->ScrollConsole();
}

void ConsoleMain::console(const std::string_view what, std::int32_t col) // overload;
{
  if (what.empty())
  {
    return;
  }
  if (this->mWriteToFile)
  {
    NotImplemented("logging", "GetGameLog is implemented in shared directory");
#if 0
    auto &fs = *::Console::mFileSystem;
    addlinetologfile(fs, GetGameLog(), std::string(what), GetGameLogFilename());
#endif
  }
  LogDebugG("{}", what);

  this->ConsoleAdd(what, col);
  if (mBigConsole != nullptr)
  {
    mBigConsole->ConsoleAdd(what, col);
  }
}