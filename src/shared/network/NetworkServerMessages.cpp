// automatically converted
#include "NetworkServerMessages.hpp"

#include "../../server/Server.hpp"
#include "../Command.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "common/Logging.hpp"
#include "../misc/MemoryUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include <clocale>
#include <cuchar>
#include <steam/isteamnetworkingmessages.h>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

void serversendstringmessage(const std::string &text, std::uint8_t tonum, std::uint8_t from,
                             std::uint8_t msgtype)
{
    pmsg_stringmessage pchatmessage;
    std::int32_t i;
    std::int32_t size;

    if (length(text) == 0)
        return;

    size = sizeof(tmsg_stringmessage) + length(text) + 1;

    getmem(pchatmessage, size);

    pchatmessage->header.id = msgid_chatmessage;
    pchatmessage->num = from;
    pchatmessage->msgtype = msgtype;

    memcpy(&pchatmessage->text, text.data(), length(text));
    //*(pchatmessage->text + length(text)) = '\0';

    if (((from > 0) && (from < max_players + 1)) || (from == 255))
    {
        for (i = 1; i <= max_players; i++)
            if (sprite[i].active && (sprite[i].player->controlmethod == human))
                if ((tonum == 0) || (i == tonum))
                {
                    if (!((from == 255) && (tonum == 0))) // TODO: Simplify it.
                        if ((!((msgtype == msgtype_team) || (msgtype == msgtype_radio)) ||
                             (from == 255)) or
                            (((msgtype == msgtype_team) || (msgtype == msgtype_radio)) and
                             sprite[from].isinsameteam(sprite[i])))
                            GetServerNetwork()->senddata(pchatmessage, size, sprite[i].player->peer,
                                                   k_nSteamNetworkingSend_Reliable);
                }
    }
    freemem(pchatmessage);

    if ((from < 1) || (from > max_players))
        return;

    // show text on servers side
    if (sprite[from].player->controlmethod == bot)
    {
        auto msg = iif(msgtype == msgtype_team, std::string("(TEAM)"), std::string(""));
        GetServerMainConsole().console(msg + "[" + sprite[from].player->name + "] " + text,
                                       teamchat_message_color);
    }
}

static inline std::string U16toString(const std::u16string &wstr)
{
    std::string str = "";
    char cstr[3] = "\0";
    mbstate_t mbs;
    for (const auto &it : wstr)
    {
        std::memset(&mbs, 0, sizeof(mbs)); // set shift state to the initial state
        std::memmove(cstr, "\0\0\0", 3);
        std::c16rtomb(cstr, it, &mbs);
        str.append(std::string(cstr));
    } // for
    return str;
}

void serverhandlechatmessage(SteamNetworkingMessage_t *netmessage)
{
    std::string cs, cschat;
    std::uint8_t msgtype;
    tplayer *player;

    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);

    if (player->spritenum == 0)
        return;

    messagesasecnum[player->spritenum] += 1;

    char16_t *v = reinterpret_cast<char16_t *>(
        &(reinterpret_cast<pmsg_stringmessage>(netmessage->m_pData)->text));
    cs = U16toString(v);
    msgtype = pmsg_stringmessage(netmessage->m_pData)->msgtype;

    LogDebug("net_msg", "Message {}", cs);

    if (msgtype > msgtype_radio)
        return;

    if (length(cs) > 100)
    {
        // Fixed DoS Exploit that causes crash on clients.
        kickplayer(player->spritenum, true, kick_flooding, twenty_minutes, "DoS Exploit");
        return;
    }

    player->chatwarnings += 1;

    cschat = cs;

    // command
    if (msgtype == msgtype_cmd)
    {
#ifdef SCRIPT
        if (scrptdispatcher.onplayercommand(player.spritenum, std::string(cs)))
            return;
#endif
        GetServerMainConsole().console(cs + "(" + (player->ip) + "[" + (player->name) + "]" + ")",
                                       default_message_color);
        parseinput(std::string(cs), player->spritenum);
        return;
    }

    cschat = std::string("[") + (player->name) + "] " + cs;

    if (msgtype == msgtype_team)
        cschat = std::string("(TEAM) ") + cschat;
    if (msgtype == msgtype_radio)
        cschat = std::string("(RADIO) ") + cschat;
    if (player->muted == 1)
        cschat = std::string("(MUTED) ") + cschat;

    GetServerMainConsole().console(cschat, chat_message_color);

    if (player->muted == 1)
        serversendstringmessage("(Muted)", all_players, player->spritenum, msgtype_pub);
    else
        serversendstringmessage(cs, all_players, player->spritenum, msgtype);

    if (player->muted == 1)
        return; // cs := "(Muted)";

    if (msgtype == msgtype_team)
        cs = iif(msgtype == msgtype_radio, "*", "^") + cs;

#ifdef SCRIPT
    scrptdispatcher.onplayerspeak(player.spritenum, std::string(cs));
#endif
}

void serversendspecialmessage(std::string text, std::uint8_t msgtype, std::uint8_t layerid,
                              std::int32_t delay, float scale, uint32 color, float x, float y,
                              std::uint8_t tonum)
{
    pmsg_serverspecialmessage pchatmessage;
    std::int32_t size;
    std::int32_t i;

    size = sizeof(tmsg_serverspecialmessage) + length(text) + 1;
    getmem(pchatmessage, size);

    pchatmessage->header.id = msgid_specialmessage;
    pchatmessage->msgtype = msgtype;
    pchatmessage->layerid = layerid;
    pchatmessage->delay = delay;
    pchatmessage->scale = scale;
    pchatmessage->color = color;
    pchatmessage->x = x;
    pchatmessage->y = y;

    strcpy(pchatmessage->text.data(), (pchar)(text));

    for (i = 1; i <= max_players; i++)
        if (sprite[i].active && (sprite[i].player->controlmethod == human))
            if ((tonum == 0) || (i == tonum))
                GetServerNetwork()->senddata(pchatmessage, size, sprite[i].player->peer,
                                       k_nSteamNetworkingSend_Reliable);

    freemem(pchatmessage);
}
