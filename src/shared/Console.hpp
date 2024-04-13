#pragma once

#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/misc/SoldatConfig.hpp"
#include <array>
#include <cstdint>
#include <string>

#ifdef SERVER
constexpr std::int32_t console_max_messages = 20;
#else
constexpr std::int32_t console_max_messages = 255;
#endif

template <Config::Module M>
class Console
{
public:
  explicit Console(const std::int32_t newMessageWait = 0, const std::int32_t countMax = 254, const std::int32_t scrollTickMax = 150)
    : mNewMessageWait(newMessageWait), mCountMax(countMax), mScrollTickMax(scrollTickMax)
  {
    if (mCountMax > 254) { mCountMax = 254; }
  }
  [[nodiscard]] const std::string& GetTextMessage(const std::int32_t i) const
  {
    return mTextMessage[i];
  }
  [[nodiscard]] std::uint32_t GetTextMessageColor(const std::int32_t i) const
  {
    return mTextMessageColor[i];
  }
  [[nodiscard]] std::int32_t GetNumMessage(const std::int32_t i) const
  {
    return mNumMessage[i];
  }
  // scrolls - in ticks 60=1 sec}
  [[nodiscard]] std::int32_t GetNewMessageWait() const { return mNewMessageWait; }
  // resuming the scroll count down
  [[nodiscard]] std::uint8_t GetAlphaCount() const { return mAlphaCount; }
  [[nodiscard]] std::int32_t GetCount() const { return mCount; }
  void ResetCount() { mCount = 0; }

  void UpdateMainConsole();
  void UpdateKillConsole();

  //  public
  void ScrollConsole();
  void console(const std::string_view what, std::int32_t col, std::uint8_t sender) requires(Config::IsServer(M));
  void console(const std::string_view what, std::int32_t col);

  void ConsoleAdd(const std::string_view what, std::int32_t col);
  void ConsoleNum(const std::string_view what, std::int32_t col, std::int32_t num);
private:
  std::uint8_t mAlphaCount = 255;
  std::int32_t mNewMessageWait = 0; // how long it waits after a new message before
  std::int32_t mCount = 0;
  std::int32_t mCountMax = 1;
  std::int32_t mScrollTickMax = 1;  // how long the scroll count down is before it
  std::int32_t mScrollTick = 0;
  PascalArray<std::string, 1, console_max_messages> mTextMessage;
  PascalArray<std::uint32_t, 1, console_max_messages> mTextMessageColor;
  PascalArray<std::int32_t, 1, console_max_messages> mNumMessage;
};

using tconsole = Console<Config::GetModule()>;

template <Config::Module M = Config::GetModule()>
Console<M> &InitBigConsole(const std::int32_t newMessageWait, const std::int32_t countMax,
                           const std::int32_t scrollTickMax);
template <Config::Module M = Config::GetModule()>
Console<M> &GetBigConsole();

template <Config::Module M = Config::GetModule()>
Console<M> &InitKillConsole(const std::int32_t newMessageWait, const std::int32_t countMax,
                            const std::int32_t scrollTickMax);
template <Config::Module M = Config::GetModule()>
Console<M> &GetKillConsole();
