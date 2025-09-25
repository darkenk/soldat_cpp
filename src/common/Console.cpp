#include "Console.hpp"

#include "Constants.hpp"
#include "Logging.hpp"
#include "port_utils/NotImplemented.hpp"
#include <cstdint>
#include <string_view>

void FConsole::ScrollConsole()
{
	ScrollTick = 0;
	if (Count < 0)
	{
		return;
	}
	for (std::int32_t x = 0; x < Count; x++)
	{
		mTextMessageColor[x] = mTextMessageColor[x + 1];
		mTextMessage[x] = mTextMessage[x + 1];
		mNumMessage[x] = mNumMessage[x + 1]; // scroll the messages up 1
	}
	mTextMessage[Count] = ""; // blank the last message
	mNumMessage[Count] = 0;
	Count -= 1;
}

void FConsole::ConsoleAdd(const std::string_view InWhat, std::int32_t col, std::int32_t num)
{
	Count += 1;
	ScrollTick = -NewMessageWait;
	mTextMessage[Count] = InWhat;
	mTextMessageColor[Count] = col;
	mNumMessage[Count] = num;
	if (Count == CountMax - 1)
	{
		ScrollConsole();
	}
}

void FConsoleMain::Update(const bool InKillConsole)
{
	++this->ScrollTick;
	if (this->ScrollTick != this->ScrollTickMax)
	{
		return;
	}
	this->ScrollConsole();
	if (!InKillConsole)
	{
		return;
	}
	if ((this->Count > -1) && (this->mNumMessage[this->Count] == -255))
	{
		this->ScrollConsole();
	}
}

void FConsoleMain::Console(const std::string_view InWhat, std::int32_t col) // overload;
{
	if (InWhat.empty())
	{
		return;
	}
	if (this->WriteToFile)
	{
		NotImplemented("logging", "GetGameLog is implemented in shared directory");
#if 0
    GS::GetConsleLogFile().addlinetologfile(std::string(what));
#endif
	}
	LogDebugG("{}", InWhat);

	this->ConsoleAdd(InWhat, col);
	if (BigConsole != nullptr)
	{
		BigConsole->ConsoleAdd(InWhat, col);
	}
}

// tests
#include <cstdio>
#include <doctest/doctest.h>
#include <spdlog/fmt/bundled/core.h>

namespace
{

	class FConsoleFixture
	{
	public:
		FConsoleFixture() = default;
		FConsoleFixture(FConsoleFixture&&) = delete;
		FConsoleFixture& operator=(const FConsoleFixture&) = delete;
		FConsoleFixture& operator=(FConsoleFixture&&) = delete;
		~FConsoleFixture() = default;
		FConsoleFixture(const FConsoleFixture&) = delete;

		static void AddMessagesUntilScroll(FConsole& console, std::int32_t countMax)
		{
			auto NoOfMessagesTillScroll = countMax - console.GetCount();
			for (auto i = 0; i < NoOfMessagesTillScroll; ++i)
			{
				console.ConsoleAdd(std::format("Filler Message {:d}", i + 1), i * 10);
			}
		}

	protected:
	};
	TEST_SUITE("Console")
	{

		TEST_CASE_FIXTURE(FConsoleFixture, "Write message")
		{
			FConsole Big;
			FConsoleMain cl(0, 254, 150, false);
			cl.SetBigConsole(&Big);
			cl.Console("Test message", Constants::GAME_MESSAGE_COLOR);
			CHECK_EQ(1, cl.GetCount());
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ScrollConsole - Normal Scrolling")
		{
			constexpr auto kNewMessageWait = 0;
			constexpr auto kCountMax = 254;
			constexpr auto kScrollTickMax = 150;
			constexpr auto kWriteToFile = true;
			FConsole Console(kNewMessageWait, kCountMax, kScrollTickMax, kWriteToFile);
			Console.ConsoleAdd("Message 1", 10);
			Console.ConsoleAdd("Message 2", 20);
			Console.ConsoleAdd("Message 3", 30);
			CHECK(Console.GetCount() == 3);

			AddMessagesUntilScroll(Console, kCountMax); // Trigger ScrollConsole indirectly

			// After scrolling, the first two messages should be shifted up, and the last slot should be
			// cleared
			CHECK(Console.GetTextMessage(1) == "Message 2");
			CHECK(Console.GetTextMessage(2) == "Message 3");
			CHECK(Console.GetNumMessage(1) == -255);
			CHECK(Console.GetNumMessage(2) == -255);
			CHECK(Console.GetTextMessageColor(1) == 20);
			CHECK(Console.GetTextMessageColor(2) == 30);
			CHECK(Console.GetCount() == kCountMax - 1);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ScrollConsole - Scroll Tick Reset")
		{
			constexpr auto kNewMessageWait = 0;
			constexpr auto kCountMax = 254;
			constexpr auto kScrollTickMax = 150;
			constexpr auto kWriteToFile = true;
			FConsole Console(kNewMessageWait, kCountMax, kScrollTickMax, kWriteToFile);
			Console.ConsoleAdd("Message 1", 10);
			AddMessagesUntilScroll(Console, kCountMax);

			// CHECK(console.mScrollTick == 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ScrollConsole - Single Message")
		{
			constexpr auto kNewMessageWait = 0;
			constexpr auto kCountMax = 254;
			constexpr auto kScrollTickMax = 150;
			constexpr auto kWriteToFile = true;
			FConsole Console(kNewMessageWait, kCountMax, kScrollTickMax, kWriteToFile);
			Console.ConsoleAdd("Only Message", 99);
			AddMessagesUntilScroll(Console, kCountMax);

			CHECK(Console.GetCount() == kCountMax - 1);
			CHECK(Console.GetTextMessage(1) == "Filler Message 1");
			CHECK(Console.GetNumMessage(1) == -255);
			CHECK(Console.GetTextMessageColor(1) == 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "Console - Add Empty Message")
		{
			FConsoleMain cl;
			cl.Console("", 10);
			CHECK_EQ(cl.GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "Console - Add Message Client")
		{
			constexpr auto kNewMessageWait = 0;
			constexpr auto kCountMax = 254;
			constexpr auto kScrollTickMax = 150;
			constexpr auto kWriteToFile = false;
			FConsoleMain cl(kNewMessageWait, kCountMax, kScrollTickMax, kWriteToFile);
			FConsole Big;
			cl.SetBigConsole(&Big);
			cl.Console("Client message", 30);
			CHECK_EQ(cl.GetCount(), 1);
			CHECK_EQ(cl.GetTextMessage(1), "Client message");
			CHECK_EQ(cl.GetTextMessageColor(1), 30);
			CHECK_EQ(Big.GetCount(), 1);
			CHECK_EQ(Big.GetTextMessage(1), "Client message");
			CHECK_EQ(Big.GetTextMessageColor(1), 30);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Add New Message")
		{
			FConsoleMain Console;
			Console.ConsoleAdd("Test message", 10, 5);
			CHECK_EQ(Console.GetCount(), 1);
			CHECK_EQ(Console.GetTextMessage(1), "Test message");
			CHECK_EQ(Console.GetTextMessageColor(1), 10);
			CHECK_EQ(Console.GetNumMessage(1), 5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Scroll When Max Count Reached")
		{
			constexpr auto kCountMax = 3;
			FConsoleMain Console(0, kCountMax, 150);
			Console.ConsoleAdd("Message 1", 10, 1);
			Console.ConsoleAdd("Message 2", 20, 2);
			Console.ConsoleAdd("Message 3", 30, 3);

			CHECK_EQ(Console.GetCount(), kCountMax - 1);
			CHECK_EQ(Console.GetTextMessage(1), "Message 2");
			CHECK_EQ(Console.GetTextMessage(2), "Message 3");
			CHECK_EQ(Console.GetNumMessage(1), 2);
			CHECK_EQ(Console.GetNumMessage(2), 3);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Empty Message")
		{
			FConsoleMain Console;
			Console.ConsoleAdd("", 10, 5);
			CHECK_EQ(Console.GetCount(), 1);
			CHECK_EQ(Console.GetTextMessage(1), "");
			CHECK_EQ(Console.GetTextMessageColor(1), 10);
			CHECK_EQ(Console.GetNumMessage(1), 5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Negative Color Value")
		{
			FConsoleMain Console;
			Console.ConsoleAdd("Test message", -10, 5);
			CHECK_EQ(Console.GetCount(), 1);
			CHECK_EQ(Console.GetTextMessage(1), "Test message");
			CHECK_EQ(Console.GetTextMessageColor(1), -10);
			CHECK_EQ(Console.GetNumMessage(1), 5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Negative Num Value")
		{
			FConsoleMain Console;
			Console.ConsoleAdd("Test message", 10, -5);
			CHECK_EQ(Console.GetCount(), 1);
			CHECK_EQ(Console.GetTextMessage(1), "Test message");
			CHECK_EQ(Console.GetTextMessageColor(1), 10);
			CHECK_EQ(Console.GetNumMessage(1), -5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Scrolls When ScrollTickMax Reached")
		{
			FConsoleMain Console(0, 2, 1);
			Console.ConsoleAdd("Message 1", 10, 1);
			Console.Update(true);
			CHECK_EQ(Console.GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Scrolls Twice When Last Message Num is -255")
		{
			FConsoleMain Console(0, 3, 1);
			Console.ConsoleAdd("Message 1", 10, -255);
			Console.ConsoleAdd("Message 2", 10, -255);
			Console.Update(true);
			CHECK_EQ(Console.GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Does Not Scroll When ScrollTickMax Not Reached")
		{
			FConsoleMain Console(3, 2, 4);
			Console.ConsoleAdd("Message 1", 10, 1);
			Console.Update(true);
			CHECK_EQ(Console.GetCount(), 1);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Does Not Scroll When No Messages")
		{
			FConsoleMain Console(0, 3, 1);
			Console.Update(true);
			CHECK_EQ(Console.GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateMainConsole - Scrolls When ScrollTickMax Reached")
		{
			FConsoleMain Console(0, 2, 1);
			Console.ConsoleAdd("Message 1", 10);
			Console.Update();
			CHECK_EQ(Console.GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateMainConsole - Does Not Scroll When ScrollTickMax Not Reached")
		{
			FConsoleMain Console(3, 2, 4);
			Console.ConsoleAdd("Message 1", 10);
			Console.Update();
			CHECK_EQ(Console.GetCount(), 1);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateMainConsole - Does Not Scroll When No Messages")
		{
			FConsoleMain Console(0, 3, 1);
			Console.Update();
			CHECK_EQ(Console.GetCount(), 0);
		}

	} // TEST_SUITE("Console")

} // namespace