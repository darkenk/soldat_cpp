// automatically converted
#include "Console.hpp"
#include "LogFile.hpp"
#include "Logging.hpp"
#ifdef SERVER
#include "network/NetworkServer.hpp"
#include "network/NetworkServerMessages.hpp"
#endif

#include <locale>

tconsole mainconsole;
tconsole bigconsole;
tconsole killconsole;

void tconsole::scrollconsole()
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

void tconsole::consoleadd(const std::string &what, std::int32_t col)
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

void tconsole::consolenum(const std::string &what, std::int32_t col, std::int32_t num)
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

void tconsole::console(const std::string &what, std::int32_t col) // overload;
{
    if (what.empty())
    {
        return;
    }

    addlinetologfile(gamelog, what, consolelogfilename);

#ifdef SERVER
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
#else
    mainconsole.consoleadd(what, col);
    bigconsole.consoleadd(what, col);
#endif
}

#if 0
void tconsole::console(const std::string &what, std::int32_t col) // overload;
{
    this->console(const std::wstring& (what), col);
}

void tconsole::console(variant what, std::uint32_t col) // overload;
{
  this->console(const std::wstring& (what), col);
}
#endif

#ifdef SERVER
template <>
void tconsole::console<std::string>(const std::string &what, std::int32_t col,
                                    std::uint8_t sender) // overload;
{
    this->console(what, col);
    if ((sender > 0) && (sender < max_players + 1))
        serversendstringmessage(what, sender, 255, msgtype_pub);
}
#endif
