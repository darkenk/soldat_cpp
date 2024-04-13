// automatically converted
#include "NetworkServerMessages.hpp"

#include "../../server/Server.hpp"
#include "../Command.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../misc/MemoryUtils.hpp"
#include "NetworkServer.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <clocale>
#include <cuchar>

void serversendstringmessage(const std::string &text, std::uint8_t tonum, std::uint8_t from,
                             std::uint8_t msgtype)
{
  pmsg_stringmessage pchatmessage;
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
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if (sprite.player->controlmethod == human)
      {
        if ((tonum == 0) || (sprite.num == tonum))
        {
          if (!((from == 255) && (tonum == 0))) // TODO: Simplify it.
            if ((!((msgtype == msgtype_team) || (msgtype == msgtype_radio)) || (from == 255)) or
                (((msgtype == msgtype_team) || (msgtype == msgtype_radio)) and
                 SpriteSystem::Get().GetSprite(from).isinsameteam(sprite)))
              GetServerNetwork()->SendData(pchatmessage, size, sprite.player->peer,
                                           true);
        }
      }
    }
  }
  freemem(pchatmessage);

  if ((from < 1) || (from > max_players))
    return;

  // show text on servers side
  if (SpriteSystem::Get().GetSprite(from).player->controlmethod == bot)
  {
    auto msg = iif(msgtype == msgtype_team, std::string("(TEAM)"), std::string(""));
    GS::GetMainConsole().console(msg + "[" + SpriteSystem::Get().GetSprite(from).player->name +
                                   "] " + text,
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

void serverhandlechatmessage(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player)
{
  std::string cs, cschat;
  std::uint8_t msgtype;

  if (player->spritenum == 0)
    return;

  messagesasecnum[player->spritenum] += 1;

  char16_t *v = reinterpret_cast<char16_t *>(
    &(reinterpret_cast<pmsg_stringmessage>(netmessage)->text));
  cs = U16toString(v);
  msgtype = pmsg_stringmessage(netmessage)->msgtype;

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
    GS::GetMainConsole().console(cs + "(" + (player->ip) + "[" + (player->name) + "]" + ")",
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

  GS::GetMainConsole().console(cschat, chat_message_color);

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
                              std::int32_t delay, float scale, std::uint32_t color, float x, float y,
                              std::uint8_t tonum)
{
  pmsg_serverspecialmessage pchatmessage;
  std::int32_t size;

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

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
      if ((tonum == 0) || (sprite.num == tonum))
        GetServerNetwork()->SendData(pchatmessage, size, sprite.player->peer,
                                     true);
  }

  freemem(pchatmessage);
}
