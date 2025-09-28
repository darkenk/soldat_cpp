#pragma once
#include <boost/di/extension/injections/named_parameters.hpp>
#include <cstdint>
#include <entt/signal/fwd.hpp>
#include <libassert/assert.hpp>
#include <string>
#include <algorithm>
#include <string_view>
#include <vector>

#include "LogFile.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include "misc/PortUtils.hpp"

using boost::di::extension::operator""_s;

class FFileUtility;

class FConsole
{
public:
	BOOST_DI_INJECT(explicit FConsole,
		(named = "NewMessageWait"_s) const std::int32_t InNewMessageWait = 0,
		(named = "CountMax"_s) const std::int32_t InCountMax = 254,
		(named = "ScrollTickMax"_s) const std::int32_t InScrollTickMax = 150,
		(named = "WriteToFile"_s) bool InWriteToFile = true)
		: NewMessageWait(InNewMessageWait)
		, CountMax(std::min(InCountMax, 254))
		, ScrollTickMax(InScrollTickMax)
		, WriteToFile(InWriteToFile)
	{
		DEBUG_ASSERT(CountMax > 0);
		mTextMessage.resize(CountMax);
		mTextMessageColor.resize(CountMax);
		mNumMessage.resize(CountMax);
	}
	[[nodiscard]] const std::string& GetTextMessage(const std::int32_t i) const { return mTextMessage[i - 1]; }
	[[nodiscard]] std::uint32_t GetTextMessageColor(const std::int32_t i) const { return mTextMessageColor[i - 1]; }
	[[nodiscard]] std::int32_t GetNumMessage(const std::int32_t i) const { return mNumMessage[i - 1]; }
	// scrolls - in ticks 60=1 sec}
	[[nodiscard]] std::int32_t GetNewMessageWait() const { return NewMessageWait; }
	// resuming the scroll count down
	[[nodiscard]] std::int32_t GetCount() const { return Count + 1; }
	void ResetCount() { Count = -1; }
	void ConsoleAdd(std::string_view InWhat, std::int32_t InColor, std::int32_t InNum = -255);

protected:
	void ScrollConsole();

	std::int32_t NewMessageWait = 0; // how long it waits after a new message before
	std::int32_t Count = -1;
	std::int32_t CountMax = 1;
	std::int32_t ScrollTickMax = 1; // how long the scroll count down is before it
	std::int32_t ScrollTick = 0;
	bool WriteToFile = false;

	std::vector<std::string> mTextMessage;
	std::vector<std::uint32_t> mTextMessageColor;
	std::vector<std::int32_t> mNumMessage;
};

class FConsoleMain : public FConsole
{
public:
	BOOST_DI_INJECT(explicit FConsoleMain,
		std::shared_ptr<entt::dispatcher> InDispatcher,
		std::shared_ptr<FLogFile> InFileLog,
		(named = "NewMessageWait"_s) const std::int32_t InNewMessageWait = 1,
		(named = "CountMax"_s) const std::int32_t InCountMax = 254,
		(named = "ScrollTickMax"_s) const std::int32_t InScrollTickMax = 1,
		(named = "WriteToFile"_s) bool InWriteToFile = true)
		: FConsole(InNewMessageWait, InCountMax, InScrollTickMax, InWriteToFile)
		, Dispatcher(std::move(InDispatcher))
		, FileLog(std::move(InFileLog))
	{
	}
	void Update(bool InKillConsole = false);
	void Console(std::string_view InWhat, std::int32_t InColor);

private:
	FConsole* BigConsole = nullptr;
	std::shared_ptr<entt::dispatcher> Dispatcher;
	std::shared_ptr<FLogFile> FileLog;
};

class FConsoleBig : public FConsole
{
public:
	using FConsole::FConsole;
};

class FKillConsole : public FConsoleMain
{
public:
	using FConsoleMain::FConsoleMain;
};