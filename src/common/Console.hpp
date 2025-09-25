#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <string_view>
#include <vector>

#include "misc/PortUtilsSoldat.hpp"
#include "misc/PortUtils.hpp"

class FFileUtility;

class FConsole
{
public:
	explicit FConsole(FFileUtility* filesystem = nullptr,
		const std::int32_t InNewMessageWait = 0,
		const std::int32_t InCountMax = 254,
		const std::int32_t InScrollTickMax = 150,
		bool writeToFile = true)
		: NewMessageWait(InNewMessageWait)
		, CountMax(std::min(InCountMax, 254))
		, ScrollTickMax(InScrollTickMax)
		, WriteToFile(writeToFile)
		, FileSystem(filesystem)
	{
		SoldatAssert(CountMax > 0);
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
	void ConsoleAdd(std::string_view what, std::int32_t col, std::int32_t num = -255);

protected:
	void ScrollConsole();

	std::int32_t NewMessageWait = 0; // how long it waits after a new message before
	std::int32_t Count = -1;
	std::int32_t CountMax = 1;
	std::int32_t ScrollTickMax = 1; // how long the scroll count down is before it
	std::int32_t ScrollTick = 0;
	bool WriteToFile = false;
	FFileUtility* FileSystem = nullptr;

	std::vector<std::string> mTextMessage;
	std::vector<std::uint32_t> mTextMessageColor;
	std::vector<std::int32_t> mNumMessage;
};

class FConsoleMain : public FConsole
{
public:
	explicit FConsoleMain(FFileUtility* filesystem = nullptr,
		const std::int32_t InNewMessageWait = 0,
		const std::int32_t InCountMax = 254,
		const std::int32_t InScrollTickMax = 150,
		bool writeToFile = true)
		: FConsole(filesystem, InNewMessageWait, InCountMax, InScrollTickMax, writeToFile)
	{
	}
	void Update(bool InKillConsole = false);
	void SetBigConsole(FConsole* bigConsole) { BigConsole = bigConsole; }
	void Console(std::string_view what, std::int32_t col);

private:
	FConsole* BigConsole = nullptr;
};
