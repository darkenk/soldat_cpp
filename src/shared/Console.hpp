#pragma once

#include "common/misc/PortUtilsSoldat.hpp"
#include "common/Console.hpp"
#include "shared/misc/SoldatConfig.hpp"
#include <cstdint>
#include <string>

class FileUtility;

template <Config::Module M>
class ConsoleServer : public ConsoleMain
{
public:
  explicit ConsoleServer(FileUtility* filesystem = nullptr, const std::int32_t newMessageWait = 0, const std::int32_t countMax = 254,
                       const std::int32_t scrollTickMax = 150, bool writeToFile = true)
    : ConsoleMain(filesystem, newMessageWait, countMax, scrollTickMax, writeToFile)
  {
  }
  void console(const std::string_view what, std::int32_t col, std::uint8_t sender = 255)
    requires(Config::IsServer(M));
};

Console &InitBigConsole(FileUtility* filesystem, const std::int32_t newMessageWait, const std::int32_t countMax,
                           const std::int32_t scrollTickMax);
Console &GetBigConsole();

ConsoleMain &InitKillConsole(FileUtility* filesystem, const std::int32_t newMessageWait, const std::int32_t countMax,
                                const std::int32_t scrollTickMax);
ConsoleMain &GetKillConsole();
