// automatically converted
#include "Console.hpp"
#include "LogFile.hpp"
#include "common/Logging.hpp"
#include "network/Net.hpp"
#include "network/NetworkServerMessages.hpp"
#include "shared/misc/GlobalSystems.hpp"

#include <locale>

template <Config::Module M>
void Console<M>::ScrollConsole()
{
  if (mCount > 0)
  {
    for (std::int32_t x = 1; x <= mCount - 1; x++)
    {
      mTextMessageColor[x] = mTextMessageColor[x + 1];
      mTextMessage[x] = mTextMessage[x + 1];
      mNumMessage[x] = mNumMessage[x + 1]; // scroll the messages up 1
      mAlphaCount = 255;
    }
    mTextMessage[mCount] = ""; // blank the last message
    mNumMessage[mCount] = 0;
    mCount -= 1;
  }
  mScrollTick = 0;
}

template <Config::Module M>
void Console<M>::ConsoleAdd(const std::string_view what, std::int32_t col)
{
  // adds a new message
  mCount += 1;
  mScrollTick = -mNewMessageWait;
  mTextMessage[mCount] = what;
  mTextMessageColor[mCount] = col;
  mNumMessage[mCount] = -255;
  if (mCount == 1)
  {
    mAlphaCount = 255;
  }
  if (mCount == mCountMax)
  {
    ScrollConsole();
  }
}

template <Config::Module M>
void Console<M>::ConsoleNum(const std::string_view what, std::int32_t col, std::int32_t num)
{
  // adds a new message
  mCount += 1;
  mScrollTick = -mNewMessageWait;
  mTextMessage[mCount] = what;
  mTextMessageColor[mCount] = col;
  mNumMessage[mCount] = num;
  if (mCount == mCountMax)
  {
    ScrollConsole();
  }
}
template <Config::Module M>
void Console<M>::UpdateMainConsole()
{
  mScrollTick = mScrollTick + 1;
  if (mScrollTick == mScrollTickMax)
    ScrollConsole();

  if (mAlphaCount > 0)
  {
    mAlphaCount--;
  }
}

template <Config::Module M>
void Console<M>::UpdateKillConsole()
{
  mScrollTick = mScrollTick + 1;
  if (mScrollTick == mScrollTickMax)
  {
    ScrollConsole();
    if ((mCount > 0) && (mNumMessage[mCount] == -255))
      ScrollConsole();
  }
}

template <Config::Module M>
void Console<M>::console(const std::string_view what, std::int32_t col) // overload;
{
  if (what.empty())
  {
    return;
  }
  auto& fs = GS::GetFileSystem();
  addlinetologfile(fs, GetGameLog(), std::string(what), GetGameLogFilename());

  if constexpr (Config::IsServer(M))
  {
    LogDebugG("{}", what);
#ifdef RCON
    broadcastmsg(std::string(what));
#endif

    // adds a new message
    // NOTE: not thread save!
    // added mod to prevent AVs
    mCount += 1;
    if (mCount >= mCountMax)
      mCount = 1;

    mScrollTick = -mNewMessageWait;
    mTextMessage[mCount] = what;
    mTextMessageColor[mCount] = col;
    mNumMessage[mCount] = -255;
    if (mCount == 1)
      mAlphaCount = 255;
    if (mCount == mCountMax)
      ScrollConsole();
  }
  if constexpr (Config::IsClient(M))
  {
    GS::GetMainConsole().ConsoleAdd(what, col);
    GetBigConsole().ConsoleAdd(what, col);
  }
}

template <Config::Module M>
void Console<M>::console(const std::string_view what, std::int32_t col, std::uint8_t sender) requires(Config::IsServer(M))
{
  this->console(what, col);
  if ((sender > 0) && (sender < max_players + 1))
  {
    if constexpr (Config::IsServer(M))
    {
      serversendstringmessage(std::string(what), sender, 255, msgtype_pub);
    }
  }
}

template class Console<Config::GetModule()>;

static Console<Config::GetModule()> sBigConsole;
static Console<Config::GetModule()> sKillConsole;

template <Config::Module M>
Console<M> &InitBigConsole(const std::int32_t newMessageWait, const std::int32_t countMax,
                           const std::int32_t scrollTickMax)
{
  return *new (&sBigConsole)Console<M>(newMessageWait, countMax, scrollTickMax);
}

template <Config::Module M>
Console<M> &GetBigConsole()
{
  return sBigConsole;
}
template <Config::Module M>
Console<M> &InitKillConsole(const std::int32_t newMessageWait, const std::int32_t countMax,
                            const std::int32_t scrollTickMax)
{
  return *new (&sKillConsole)Console<M>(newMessageWait, countMax, scrollTickMax);
}

template <Config::Module M>
Console<M> &GetKillConsole()
{
  return sKillConsole;
}

template tconsole &GetKillConsole<Config::GetModule()>();
template tconsole &GetBigConsole<Config::GetModule()>();
template tconsole &InitKillConsole<Config::GetModule()>(const std::int32_t newMessageWait,
                                                        const std::int32_t countMax,
                                                        const std::int32_t scrollTickMax);
template tconsole &InitBigConsole<Config::GetModule()>(const std::int32_t newMessageWaitTime,
                                                       const std::int32_t countMax,
                                                       const std::int32_t scrollTickMax);

// tests
#include <doctest/doctest.h>

namespace
{

class ConsoleFixture
{
public:
  ConsoleFixture() {}
  ~ConsoleFixture() {}
  ConsoleFixture(const ConsoleFixture &) = delete;

protected:
};
TEST_SUITE("Console")
{

  TEST_CASE_FIXTURE(ConsoleFixture, "Initial test")
  {
    Console<Config::CLIENT_MODULE> cl;
    CHECK_EQ(255, cl.GetAlphaCount());
    cl.UpdateMainConsole();
    CHECK_EQ(254, cl.GetAlphaCount());
  }

} // TEST_SUITE("Console")

} // namespace