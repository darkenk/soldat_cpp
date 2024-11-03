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

template <Config::Module M>
void Console<M>::ConsoleAdd(const std::string_view what, std::int32_t col, std::int32_t num)
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

template <Config::Module M>
void ConsoleMain<M>::Update(const bool killConsole)
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

template <Config::Module M>
void ConsoleMain<M>::console(const std::string_view what, std::int32_t col) // overload;
{
  if (what.empty())
  {
    return;
  }
  if (this->mWriteToFile)
  {
    auto &fs = *::Console<M>::mFileSystem;
    addlinetologfile(fs, GetGameLog(), std::string(what), GetGameLogFilename());
  }
  LogDebugG("{}", what);

  this->ConsoleAdd(what, col);
  if (mBigConsole)
  {
    mBigConsole->ConsoleAdd(what, col);
  }
}

template <Config::Module M>
void ConsoleMain<M>::console(const std::string_view what, std::int32_t col, std::uint8_t sender)
  requires(Config::IsServer(M))
{
  this->console(what, col);
  if ((sender > 0) && (sender < max_players + 1))
  {
    serversendstringmessage(std::string(what), sender, 255, msgtype_pub);
  }
}

template class Console<Config::GetModule()>;
template class ConsoleMain<Config::GetModule()>;

static Console<Config::GetModule()> sBigConsole;
static ConsoleMain<Config::GetModule()> sKillConsole;

template <Config::Module M>
Console<M> &InitBigConsole(FileUtility* filesystem, const std::int32_t newMessageWait, const std::int32_t countMax,
                           const std::int32_t scrollTickMax)
{
  return *new (&sBigConsole) Console<M>(filesystem, newMessageWait, countMax, scrollTickMax);
}

template <Config::Module M>
Console<M> &GetBigConsole()
{
  return sBigConsole;
}

template <Config::Module M>
ConsoleMain<M> &InitKillConsole(FileUtility* filesystem, const std::int32_t newMessageWait, const std::int32_t countMax,
                                const std::int32_t scrollTickMax) requires(Config::IsClient(M))
{
  return *new (&sKillConsole) ConsoleMain<M>(filesystem, newMessageWait, countMax, scrollTickMax);
}

template <Config::Module M>
ConsoleMain<M> &GetKillConsole() requires(Config::IsClient(M))
{
  return sKillConsole;
}

#ifndef SERVER
template ConsoleMain<Config::GetModule()> &GetKillConsole<Config::GetModule()>();
template Console<Config::GetModule()> &GetBigConsole<Config::GetModule()>();
template ConsoleMain<Config::GetModule()> &InitKillConsole<Config::GetModule()>(FileUtility* filesystem,
  const std::int32_t newMessageWait, const std::int32_t countMax, const std::int32_t scrollTickMax);
template Console<Config::GetModule()> &InitBigConsole<Config::GetModule()>(FileUtility* filesystem,
  const std::int32_t newMessageWaitTime, const std::int32_t countMax,
  const std::int32_t scrollTickMax);
#endif

// tests
#include <doctest/doctest.h>

namespace
{

class ConsoleFixture
{
public:
  ConsoleFixture() = default;
  ~ConsoleFixture() = default;
  ConsoleFixture(const ConsoleFixture &) = delete;

  template <Config::Module T>
  static void addMessagesUntilScroll(Console<T> &console, std::int32_t countMax)
  {
    auto noOfMessagesTillScroll = countMax - console.GetCount();
    for (auto i = 0; i < noOfMessagesTillScroll; ++i)
    {
      char buffer[100];
      snprintf(buffer, 100, "Filler Message %d", i + 1);
      console.ConsoleAdd(buffer, i * 10);
    }
  }

protected:
};
TEST_SUITE("Console")
{

  TEST_CASE_FIXTURE(ConsoleFixture, "Write message")
  {
    Console<Config::CLIENT_MODULE> big(nullptr);
    ConsoleMain<Config::CLIENT_MODULE> cl(nullptr, 0, 254, 150, false);
    cl.SetBigConsole(&big);
    cl.console("Test message", game_message_color);
    CHECK_EQ(1, cl.GetCount());
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ScrollConsole - Normal Scrolling")
  {
    constexpr auto newMessageWait = 0;
    constexpr auto countMax = 254;
    constexpr auto scrollTickMax = 150;
    constexpr auto writeToFile = true;
    Console<Config::CLIENT_MODULE> console(nullptr, newMessageWait, countMax, scrollTickMax, writeToFile);
    console.ConsoleAdd("Message 1", 10);
    console.ConsoleAdd("Message 2", 20);
    console.ConsoleAdd("Message 3", 30);
    CHECK(console.GetCount() == 3);

    addMessagesUntilScroll(console, countMax); // Trigger ScrollConsole indirectly

    // After scrolling, the first two messages should be shifted up, and the last slot should be
    // cleared
    CHECK(console.GetTextMessage(1) == "Message 2");
    CHECK(console.GetTextMessage(2) == "Message 3");
    CHECK(console.GetNumMessage(1) == -255);
    CHECK(console.GetNumMessage(2) == -255);
    CHECK(console.GetTextMessageColor(1) == 20);
    CHECK(console.GetTextMessageColor(2) == 30);
    CHECK(console.GetCount() == countMax - 1);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ScrollConsole - Scroll Tick Reset")
  {
    constexpr auto newMessageWait = 0;
    constexpr auto countMax = 254;
    constexpr auto scrollTickMax = 150;
    constexpr auto writeToFile = true;
    Console<Config::CLIENT_MODULE> console(nullptr, newMessageWait, countMax, scrollTickMax, writeToFile);
    console.ConsoleAdd("Message 1", 10);
    addMessagesUntilScroll(console, countMax);

    // CHECK(console.mScrollTick == 0);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ScrollConsole - Single Message")
  {
    constexpr auto newMessageWait = 0;
    constexpr auto countMax = 254;
    constexpr auto scrollTickMax = 150;
    constexpr auto writeToFile = true;
    Console<Config::CLIENT_MODULE> console(nullptr, newMessageWait, countMax, scrollTickMax, writeToFile);
    console.ConsoleAdd("Only Message", 99);
    addMessagesUntilScroll(console, countMax);

    CHECK(console.GetCount() == countMax - 1);
    CHECK(console.GetTextMessage(1) == "Filler Message 1");
    CHECK(console.GetNumMessage(1) == -255);
    CHECK(console.GetTextMessageColor(1) == 0);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "Console - Add Empty Message")
  {
    ConsoleMain<Config::CLIENT_MODULE> cl;
    cl.console("", 10);
    CHECK_EQ(cl.GetCount(), 0);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "Console - Add Message to File" * doctest::skip(true))
  {
    constexpr auto newMessageWait = 0;
    constexpr auto countMax = 20;
    constexpr auto scrollTickMax = 150;
    constexpr auto writeToFile = true;
    ConsoleMain<Config::SERVER_MODULE> cl(nullptr, newMessageWait, countMax, scrollTickMax,
                                               writeToFile);
    cl.console("Test message", 10);
    // Assuming GetGameLog() and GetGameLogFilename() are accessible and return expected values
    auto &fs = GS::GetFileSystem();
    // CHECK(fs.FileExists(GetGameLogFilename()));
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "Console - Add Message Server")
  {
    constexpr auto newMessageWait = 0;
    constexpr auto countMax = 20;
    constexpr auto scrollTickMax = 150;
    constexpr auto writeToFile = false;
    ConsoleMain<Config::SERVER_MODULE> cl(nullptr, newMessageWait, countMax, scrollTickMax,
                                               writeToFile);
    cl.console("Server message", 20);
    CHECK_EQ(cl.GetCount(), 1);
    CHECK_EQ(cl.GetTextMessage(1), "Server message");
    CHECK_EQ(cl.GetTextMessageColor(1), 20);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "Console - Add Message Client")
  {
    constexpr auto newMessageWait = 0;
    constexpr auto countMax = 254;
    constexpr auto scrollTickMax = 150;
    constexpr auto writeToFile = false;
    ConsoleMain<Config::CLIENT_MODULE> cl(nullptr, newMessageWait, countMax, scrollTickMax,
                                               writeToFile);
    Console<Config::CLIENT_MODULE> big;
    cl.SetBigConsole(&big);
    cl.console("Client message", 30);
    CHECK_EQ(cl.GetCount(), 1);
    CHECK_EQ(cl.GetTextMessage(1), "Client message");
    CHECK_EQ(cl.GetTextMessageColor(1), 30);
    CHECK_EQ(big.GetCount(), 1);
    CHECK_EQ(big.GetTextMessage(1), "Client message");
    CHECK_EQ(big.GetTextMessageColor(1), 30);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "Console - Add Message and Scroll")
  {
    constexpr auto newMessageWait = 0;
    constexpr auto countMax = 20;
    constexpr auto scrollTickMax = 150;
    constexpr auto writeToFile = false;
    ConsoleMain<Config::SERVER_MODULE> cl(nullptr, newMessageWait, countMax, scrollTickMax, writeToFile);
    cl.console("Message 1", 10);
    cl.console("Message 2", 20);
    cl.console("Message 3", 30);
    CHECK_EQ(cl.GetCount(), 3);
    addMessagesUntilScroll(cl, countMax);
    CHECK_EQ(cl.GetCount(), countMax - 1);
    CHECK_EQ(cl.GetTextMessage(1), "Message 2");
    CHECK_EQ(cl.GetTextMessage(2), "Message 3");
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ConsoleNum - Add New Message")
  {
    ConsoleMain<Config::CLIENT_MODULE> console;
    console.ConsoleAdd("Test message", 10, 5);
    CHECK_EQ(console.GetCount(), 1);
    CHECK_EQ(console.GetTextMessage(1), "Test message");
    CHECK_EQ(console.GetTextMessageColor(1), 10);
    CHECK_EQ(console.GetNumMessage(1), 5);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ConsoleNum - Scroll When Max Count Reached")
  {
    constexpr auto countMax = 3;
    ConsoleMain<Config::CLIENT_MODULE> console(nullptr, 0, countMax, 150);
    console.ConsoleAdd("Message 1", 10, 1);
    console.ConsoleAdd("Message 2", 20, 2);
    console.ConsoleAdd("Message 3", 30, 3);

    CHECK_EQ(console.GetCount(), countMax - 1);
    CHECK_EQ(console.GetTextMessage(1), "Message 2");
    CHECK_EQ(console.GetTextMessage(2), "Message 3");
    CHECK_EQ(console.GetNumMessage(1), 2);
    CHECK_EQ(console.GetNumMessage(2), 3);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ConsoleNum - Empty Message")
  {
    ConsoleMain<Config::CLIENT_MODULE> console;
    console.ConsoleAdd("", 10, 5);
    CHECK_EQ(console.GetCount(), 1);
    CHECK_EQ(console.GetTextMessage(1), "");
    CHECK_EQ(console.GetTextMessageColor(1), 10);
    CHECK_EQ(console.GetNumMessage(1), 5);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ConsoleNum - Negative Color Value")
  {
    ConsoleMain<Config::CLIENT_MODULE> console;
    console.ConsoleAdd("Test message", -10, 5);
    CHECK_EQ(console.GetCount(), 1);
    CHECK_EQ(console.GetTextMessage(1), "Test message");
    CHECK_EQ(console.GetTextMessageColor(1), -10);
    CHECK_EQ(console.GetNumMessage(1), 5);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "ConsoleNum - Negative Num Value")
  {
    ConsoleMain<Config::CLIENT_MODULE> console;
    console.ConsoleAdd("Test message", 10, -5);
    CHECK_EQ(console.GetCount(), 1);
    CHECK_EQ(console.GetTextMessage(1), "Test message");
    CHECK_EQ(console.GetTextMessageColor(1), 10);
    CHECK_EQ(console.GetNumMessage(1), -5);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "UpdateKillConsole - Scrolls When ScrollTickMax Reached")
  {
    ConsoleMain<Config::CLIENT_MODULE> console(0, 0, 2, 1);
    console.ConsoleAdd("Message 1", 10, 1);
    console.Update(true);
    CHECK_EQ(console.GetCount(), 0);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "UpdateKillConsole - Scrolls Twice When Last Message Num is -255")
  {
    ConsoleMain<Config::CLIENT_MODULE> console(0, 0, 3, 1);
    console.ConsoleAdd("Message 1", 10, -255);
    console.ConsoleAdd("Message 2", 10, -255);
    console.Update(true);
    CHECK_EQ(console.GetCount(), 0);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "UpdateKillConsole - Does Not Scroll When ScrollTickMax Not Reached")
  {
    ConsoleMain<Config::CLIENT_MODULE> console(0, 3, 2, 4);
    console.ConsoleAdd("Message 1", 10, 1);
    console.Update(true);
    CHECK_EQ(console.GetCount(), 1);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "UpdateKillConsole - Does Not Scroll When No Messages")
  {
    ConsoleMain<Config::CLIENT_MODULE> console(0, 3, 1);
    console.Update(true);
    CHECK_EQ(console.GetCount(), 0);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "UpdateMainConsole - Scrolls When ScrollTickMax Reached")
  {
    ConsoleMain<Config::CLIENT_MODULE> console(0, 0, 2, 1);
    console.ConsoleAdd("Message 1", 10);
    console.Update();
    CHECK_EQ(console.GetCount(), 0);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "UpdateMainConsole - Does Not Scroll When ScrollTickMax Not Reached")
  {
    ConsoleMain<Config::CLIENT_MODULE> console(0, 3, 2, 4);
    console.ConsoleAdd("Message 1", 10);
    console.Update();
    CHECK_EQ(console.GetCount(), 1);
  }

  TEST_CASE_FIXTURE(ConsoleFixture, "UpdateMainConsole - Does Not Scroll When No Messages")
  {
    ConsoleMain<Config::CLIENT_MODULE> console(0, 3, 1);
    console.Update();
    CHECK_EQ(console.GetCount(), 0);
  }

} // TEST_SUITE("Console")

} // namespace
