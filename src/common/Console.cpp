#include "Console.hpp"

#include <boost/di/extension/scopes/shared.hpp>
#include <cstdint>
#include <entt/signal/dispatcher.hpp>
#include <entt/signal/fwd.hpp>
#include <memory>
#include <string_view>

#include "Constants.hpp"
#include "Logging.hpp"
#include "ConsoleMessage.hpp"
#include "port_utils/NotImplemented.hpp"

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

void FConsoleMain::Console(const std::string_view InWhat, std::int32_t InColor) // overload;
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

	this->ConsoleAdd(InWhat, InColor);
	Dispatcher->enqueue<FBigConsoleMessage>(std::string(InWhat), InColor);
}

// tests
#include <boost/di.hpp>
#include <boost/di/extension/injector.hpp>
#include <cstdio>
#include <doctest/doctest.h>
#include <spdlog/fmt/bundled/core.h>
#include "ConsoleListener.hpp"

namespace
{
	namespace di = boost::di;

	class FConsoleFixture
	{
	public:
		FConsoleFixture() = default;
		FConsoleFixture(FConsoleFixture&&) = delete;
		FConsoleFixture& operator=(const FConsoleFixture&) = delete;
		FConsoleFixture& operator=(FConsoleFixture&&) = delete;
		~FConsoleFixture() = default;
		FConsoleFixture(const FConsoleFixture&) = delete;

		static void AddMessagesUntilScroll(FConsole& InConsole, std::int32_t InCountMax)
		{
			auto NoOfMessagesTillScroll = InCountMax - InConsole.GetCount();
			for (auto i = 0; i < NoOfMessagesTillScroll; ++i)
			{
				InConsole.ConsoleAdd(std::format("Filler Message {:d}", i + 1), i * 10);
			}
		}

	protected:
		// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
		// NOLINTEND(misc-non-private-member-variables-in-classes)
		auto GetInjector()
		{
			return di::make_injector<di::extension::shared_config>(di::bind<entt::dispatcher>.to(Dispatcher),
				di::bind<FConsoleMain>().in(di::extension::shared),
				di::bind<FConsoleBig>().in(di::extension::shared),
				di::bind<FBigConsoleListener>().in(di::extension::shared),
				di::bind<std::int32_t>().named("NewMessageWait"_s).to(0),
				di::bind<std::int32_t>().named("CountMax"_s).to(254),
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(150),
				di::bind<bool>().named("WriteToFile"_s).to(false));
		}

	private:
		std::shared_ptr<entt::dispatcher> Dispatcher = std::make_shared<entt::dispatcher>();
	};

	TEST_SUITE("Console")
	{
		TEST_CASE_FIXTURE(FConsoleFixture, "Write message")
		{
			auto Injector = GetInjector();
			auto Main = Injector.create<std::shared_ptr<FConsoleMain>>();
			Main->Console("Test message", Constants::GAME_MESSAGE_COLOR);

			CHECK_EQ(1, Main->GetCount());
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
			auto Injector = GetInjector();
			auto Main = Injector.create<std::shared_ptr<FConsoleMain>>();
			Main->Console("", 10);
			CHECK_EQ(Main->GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "Console - Add Message Client")
		{
			auto Injector = GetInjector();
			auto Dispatcher = Injector.create<std::shared_ptr<entt::dispatcher>>();
			auto Main = Injector.create<std::shared_ptr<FConsoleMain>>();
			auto Big = Injector.create<std::shared_ptr<FConsoleBig>>();
			auto Listener = Injector.create<std::shared_ptr<FBigConsoleListener>>();
			Main->Console("Client message", 30);
			Dispatcher->update();
			CHECK_EQ(Main->GetCount(), 1);
			CHECK_EQ(Main->GetTextMessage(1), "Client message");
			CHECK_EQ(Main->GetTextMessageColor(1), 30);
			CHECK_EQ(Big->GetCount(), 1);
			CHECK_EQ(Big->GetTextMessage(1), "Client message");
			CHECK_EQ(Big->GetTextMessageColor(1), 30);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Add New Message")
		{
			auto Injector = GetInjector();
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Test message", 10, 5);
			CHECK_EQ(Console->GetCount(), 1);
			CHECK_EQ(Console->GetTextMessage(1), "Test message");
			CHECK_EQ(Console->GetTextMessageColor(1), 10);
			CHECK_EQ(Console->GetNumMessage(1), 5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Scroll When Max Count Reached")
		{
			auto constexpr kCountMax = 3;
			auto Injector = di::make_injector<di::extension::shared_config>(
				GetInjector(), di::bind<std::int32_t>().named("CountMax"_s).to(kCountMax)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Message 1", 10, 1);
			Console->ConsoleAdd("Message 2", 20, 2);
			Console->ConsoleAdd("Message 3", 30, 3);

			CHECK_EQ(Console->GetCount(), kCountMax - 1);
			CHECK_EQ(Console->GetTextMessage(1), "Message 2");
			CHECK_EQ(Console->GetTextMessage(2), "Message 3");
			CHECK_EQ(Console->GetNumMessage(1), 2);
			CHECK_EQ(Console->GetNumMessage(2), 3);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Empty Message")
		{
			auto Injector = GetInjector();
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("", 10, 5);
			CHECK_EQ(Console->GetCount(), 1);
			CHECK_EQ(Console->GetTextMessage(1), "");
			CHECK_EQ(Console->GetTextMessageColor(1), 10);
			CHECK_EQ(Console->GetNumMessage(1), 5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Negative Color Value")
		{
			auto Injector = GetInjector();
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Test message", -10, 5);
			CHECK_EQ(Console->GetCount(), 1);
			CHECK_EQ(Console->GetTextMessage(1), "Test message");
			CHECK_EQ(Console->GetTextMessageColor(1), -10);
			CHECK_EQ(Console->GetNumMessage(1), 5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "ConsoleNum - Negative Num Value")
		{
			auto Injector = GetInjector();
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Test message", 10, -5);
			CHECK_EQ(Console->GetCount(), 1);
			CHECK_EQ(Console->GetTextMessage(1), "Test message");
			CHECK_EQ(Console->GetTextMessageColor(1), 10);
			CHECK_EQ(Console->GetNumMessage(1), -5);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Scrolls When ScrollTickMax Reached")
		{
			auto Injector = di::make_injector<di::extension::shared_config>(GetInjector(),
				di::bind<std::int32_t>().named("CountMax"_s).to(2)[di::override],
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(1)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Message 1", 10, 1);
			Console->Update(true);
			CHECK_EQ(Console->GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Scrolls Twice When Last Message Num is -255")
		{
			auto Injector = di::make_injector<di::extension::shared_config>(GetInjector(),
				di::bind<std::int32_t>().named("CountMax"_s).to(3)[di::override],
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(1)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Message 1", 10, -255);
			Console->ConsoleAdd("Message 2", 10, -255);
			Console->Update(true);
			CHECK_EQ(Console->GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Does Not Scroll When ScrollTickMax Not Reached")
		{
			auto Injector = di::make_injector<di::extension::shared_config>(GetInjector(),
				di::bind<std::int32_t>().named("NewMessageWait"_s).to(3)[di::override],
				di::bind<std::int32_t>().named("CountMax"_s).to(2)[di::override],
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(4)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Message 1", 10, 1);
			Console->Update(true);
			CHECK_EQ(Console->GetCount(), 1);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateKillConsole - Does Not Scroll When No Messages")
		{
			auto Injector = di::make_injector<di::extension::shared_config>(GetInjector(),
				di::bind<std::int32_t>().named("CountMax"_s).to(3)[di::override],
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(1)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->Update(true);
			CHECK_EQ(Console->GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateMainConsole - Scrolls When ScrollTickMax Reached")
		{
			auto Injector = di::make_injector<di::extension::shared_config>(GetInjector(),
				di::bind<std::int32_t>().named("CountMax"_s).to(2)[di::override],
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(1)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Message 1", 10);
			Console->Update();
			CHECK_EQ(Console->GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateMainConsole - Does Not Scroll When ScrollTickMax Not Reached")
		{
			auto Injector = di::make_injector<di::extension::shared_config>(GetInjector(),
				di::bind<std::int32_t>().named("NewMessageWait"_s).to(3)[di::override],
				di::bind<std::int32_t>().named("CountMax"_s).to(2)[di::override],
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(4)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->ConsoleAdd("Message 1", 10);
			Console->Update();
			CHECK_EQ(Console->GetCount(), 1);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "UpdateMainConsole - Does Not Scroll When No Messages")
		{
			auto Injector = di::make_injector<di::extension::shared_config>(GetInjector(),
				di::bind<std::int32_t>().named("CountMax"_s).to(2)[di::override],
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(1)[di::override]);
			auto Console = Injector.create<std::shared_ptr<FConsoleMain>>();
			Console->Update();
			CHECK_EQ(Console->GetCount(), 0);
		}

		TEST_CASE_FIXTURE(FConsoleFixture, "Write message using dispatcher")
		{
			entt::dispatcher Dispatcher;
			FConsole Big;
			auto Injector = GetInjector();
			auto Main = Injector.create<std::shared_ptr<FConsoleMain>>();
			struct FListener
			{
				FListener(FConsoleMain& InConsole) : Console(InConsole) { };
				void OnMessage(FMainConsoleMessage& InMessage) { Console.Console(InMessage.Message, InMessage.Color); }

			private:
				FConsoleMain& Console;
			};
			FListener Listener(*Main);
			Dispatcher.sink<FMainConsoleMessage>().connect<&FListener::OnMessage>(Listener);
			Dispatcher.enqueue<FMainConsoleMessage>("Test message", Constants::GAME_MESSAGE_COLOR);
			Dispatcher.update();
			CHECK_EQ(1, Main->GetCount());
		}

	} // TEST_SUITE("Console")

} // namespace