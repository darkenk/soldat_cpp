// automatically converted
#include "Console.hpp"
#include "LogFile.hpp"
#include "common/Logging.hpp"
#include "network/Net.hpp"
#include "network/NetworkServerMessages.hpp"
#include "shared/misc/GlobalSystems.hpp"

#include <locale>

template <Config::Module M>
void Console<M>::scrollconsole()
{
  std::int32_t x;

  if (count > 0)
  {
    for (x = 1; x <= count - 1; x++)
    {
      textmessagecolor[x] = textmessagecolor[x + 1];
      textmessage[x] = textmessage[x + 1];
      nummessage[x] = nummessage[x + 1]; // scroll the messages up 1
      alphacount = 255;
    }
    textmessage[count] = ""; // blank the last message
    nummessage[count] = 0;
    count -= 1;
  }
  scrolltick = 0;
}

template <Config::Module M>
void Console<M>::consoleadd(const std::string &what, std::int32_t col)
{
  // adds a new message
  count += 1;
  scrolltick = -newmessagewait;
  textmessage[count] = what;
  textmessagecolor[count] = col;
  nummessage[count] = -255;
  if (count == 1)
  {
    alphacount = 255;
  }
  if (count == countmax)
  {
    scrollconsole();
  }
}

template <Config::Module M>
void Console<M>::consolenum(const std::string &what, std::int32_t col, std::int32_t num)
{
  // adds a new message
  count += 1;
  scrolltick = -newmessagewait;
  textmessage[count] = what;
  textmessagecolor[count] = col;
  nummessage[count] = num;
  if (count == countmax)
  {
    scrollconsole();
  }
}

template <Config::Module M>
void Console<M>::console(const std::string &what, std::int32_t col) // overload;
{
  if (what.empty())
  {
    return;
  }

  addlinetologfile(GetGameLog(), what, GetGameLogFilename());

  if constexpr (Config::IsServer(M))
  {
    LogDebugG("{}", what);
#ifdef RCON
    broadcastmsg(std::string(what));
#endif

    // adds a new message
    // NOTE: not thread save!
    // added mod to prevent AVs
    count += 1;
    if (count >= countmax)
      count = 1;

    scrolltick = -newmessagewait;
    textmessage[count] = what;
    textmessagecolor[count] = col;
    nummessage[count] = -255;
    if (count == 1)
      alphacount = 255;
    if (count == countmax)
      scrollconsole();
  }
  if constexpr (Config::IsClient(M))
  {
    GS::GetMainConsole().consoleadd(what, col);
    GetBigConsole().consoleadd(what, col);
  }
}

template <Config::Module M>
void Console<M>::console(const std::string &what, std::int32_t col,
                         std::uint8_t sender) requires(Config::IsServer(M))
{
  this->console(what, col);
  if ((sender > 0) && (sender < max_players + 1))
  {
    if constexpr (Config::IsServer(M))
    {
      serversendstringmessage(what, sender, 255, msgtype_pub);
    }
  }
}

template class Console<Config::GetModule()>;

template <Config::Module M>
Console<M> &GetBigConsole()
{
  static Console<M> bigconsole;
  return bigconsole;
}

template <Config::Module M>
Console<M> &GetKillConsole()
{
  static Console<M> killconsole;
  return killconsole;
}

template tconsole &GetKillConsole<Config::GetModule()>();
template tconsole &GetBigConsole<Config::GetModule()>();
