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
    PascalArray<std::string, 1, console_max_messages> textmessage;
    PascalArray<std::uint32_t, 1, console_max_messages> textmessagecolor;
    PascalArray<std::int32_t, 1, console_max_messages> nummessage;
    std::int32_t count;
    std::int32_t countmax;
    std::int32_t scrolltick;
    std::int32_t scrolltickmax;  // how long the scroll count down is before it
                                 // scrolls - in ticks 60=1 sec}
    std::int32_t newmessagewait; // how long it waits after a new message before
                                 // resuming the scroll count down
    std::uint8_t alphacount;
#ifdef SERVER
    bool terminalcolors; // use terminal colors
#endif
    //  public
    void scrollconsole();
    void console(const std::string &what, std::int32_t col,
                 std::uint8_t sender) requires(Config::IsServer(M));
    void console(const std::string &what, std::int32_t col);
#if 0
    void console(const std::string &what, std::int32_t col);
    void console(variant what, std::int32_t col);
#endif

    void consoleadd(const std::string &what, std::int32_t col);
    void consolenum(const std::string &what, std::int32_t col, std::int32_t num);
};

using tconsole = Console<Config::GetModule()>;

template <Config::Module M = Config::GetModule()>
Console<M> &GetMainConsole();

constexpr auto GetServerMainConsole = GetMainConsole<Config::SERVER_MODULE>;
constexpr auto GetClientMainConsole = GetMainConsole<Config::CLIENT_MODULE>;

template <Config::Module M = Config::GetModule()>
Console<M> &GetBigConsole();
template <Config::Module M = Config::GetModule()>
Console<M> &GetKillConsole();
