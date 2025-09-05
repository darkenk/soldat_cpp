// automatically converted
#include "NetworkClientMessages.hpp"

#include <bits/types/mbstate_t.h>
#include <string.h>
#include <locale>
#include <algorithm>
#include <array>
#include <cwchar>
#include <memory>
#include <utility>

#include "../../client/Client.hpp"
#include "../../client/InterfaceGraphics.hpp"
#include "../misc/MemoryUtils.hpp"
#include "NetworkClient.hpp"
#include "common/Console.hpp"
#include "common/GameStrings.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/Constants.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"
#include "common/Logging.hpp"

using namespace std::literals::string_view_literals;

constexpr auto TAG = "network"sv;

static std::u16string utf8_to_utf16(const std::string_view utf8)
{
  char* converted = SDL_iconv_string("UTF-16LE", "UTF-8", utf8.data(), utf8.size());
  if (converted == nullptr)
  {
    LogWarn(TAG, "Cannot convert from utf8_to_utf16 {}. Error {}", utf8.data(), SDL_GetError());
    return std::u16string();
  }

  size_t len = SDL_utf8strlen(utf8.data());
  std::u16string result(reinterpret_cast<char16_t*>(converted), len);

  SDL_free(converted);
  return result;
}

void clientsendstringmessage(const std::string &text, std::uint8_t msgtype)
{
  pmsg_stringmessage pchatmessage;
  std::int32_t size;

  if (length(text) == 0)
  {
    return;
  }

  size = sizeof(pchatmessage->header) + sizeof(pchatmessage->num) + sizeof(pchatmessage->msgtype) +
         2 * length(text) + 2;
  getmem(pchatmessage, size);
  fillchar(pchatmessage, size, 0);
  pchatmessage->header.id = msgid_chatmessage;
  pchatmessage->num = gGlobalStateClient.mysprite;
  pchatmessage->msgtype = msgtype;

  std::u16string text16 = utf8_to_utf16(text);

  memcpy(&pchatmessage->text, text16.data(), text16.size() * 2);
  memset(&pchatmessage->text + text16.size() * 2, 0, 2);

  gGlobalStateNetworkClient.GetNetwork()->SendData(pchatmessage, size, true);
  freemem(pchatmessage);
}

void clienthandlechatmessage::Handle(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  std::string cs;
  std::string prefix;
  std::int32_t i;
  std::int32_t d;
  std::uint8_t msgtype;
  std::uint32_t col;

  NotImplemented("network");
#if 0
    cs = pmsg_stringmessage(netmessage->m_pData)->text.data();
#endif
  i = pmsg_stringmessage(netmessage->packet)->num;
  msgtype = pmsg_stringmessage(netmessage->packet)->msgtype;

  if (msgtype > msgtype_radio)
  {
    return;
  }
  // chat from server
  if (i == 255)
  {
    GS::GetMainConsole().console(_("*SERVER*: ") + cs, server_message_color);
    return;
  }

  if ((i > 0) && (i < max_players))
  {
    if (!sprite_system.GetSprite(i).active)
    {
      return;
    }
  }

  if ((sprite_system.GetSprite(i).muted) or gGlobalStateClient.muteall)
  {
    return;
  }

  gGlobalStateInterfaceGraphics.chatmessage[i] = cs;
  gGlobalStateInterfaceGraphics.chatteam[i] = (msgtype == msgtype_team);
  d = std::count(cs.begin(), cs.end(), ' ');

  if (d == 0)
  {
    gGlobalStateInterfaceGraphics.chatdelay[i] = length(cs) * chardelay;
  }
  else
  {
    gGlobalStateInterfaceGraphics.chatdelay[i] = d * spacechardelay;
  }

  if (gGlobalStateInterfaceGraphics.chatdelay[i] > max_chatdelay)
  {
    gGlobalStateInterfaceGraphics.chatdelay[i] = max_chatdelay;
  }

  col = chat_message_color;

  if (sprite_system.GetSprite(i).player->team == team_spectator)
  {
    col = spectator_c_message_color;
  }
  if ((msgtype == msgtype_team) || (msgtype == msgtype_radio))
  {
    col = teamchat_message_color;
    prefix = iif(msgtype == msgtype_radio, _("(RADIO)"), _("(TEAM)")) + " ";
  }

  if (length(cs) < morechattext)
  {
    GS::GetMainConsole().console(
      prefix + "[" + (sprite_system.GetSprite(i).player->name) + "] " + cs, col);
  }
  else
  {
    GS::GetMainConsole().console(prefix + "[" + (sprite_system.GetSprite(i).player->name) + "] ",
                                 col);
    GS::GetMainConsole().console(std::string(" ") + cs, col);
  }

  /*if Radio and
  SpriteSystem::Get().GetSprite(i).IsInSameTeam(SpriteSystem::Get().GetSprite(MySprite)) then
  begin
    PlayRadioSound(RadioID)
  end;*/
}

void clienthandlespecialmessage::Handle(NetworkContext *netmessage)
{
  tmsg_serverspecialmessage *specialmessage;
  std::string cs;

  specialmessage = pmsg_serverspecialmessage(netmessage->packet);
  cs = pmsg_serverspecialmessage(netmessage->packet)->text.data();

  if (specialmessage->msgtype == 0) // console
  {
    GS::GetMainConsole().console(cs, specialmessage->color);
  }
  else if (specialmessage->msgtype == 1) // big text
  {
    gGlobalStateInterfaceGraphics.bigtext[specialmessage->layerid] = cs;
    gGlobalStateInterfaceGraphics.bigdelay[specialmessage->layerid] = specialmessage->delay;
    gGlobalStateInterfaceGraphics.bigscale[specialmessage->layerid] = specialmessage->scale;
    gGlobalStateInterfaceGraphics.bigcolor[specialmessage->layerid] = specialmessage->color;
    gGlobalStateInterfaceGraphics.bigposx[specialmessage->layerid] =
      specialmessage->x * gGlobalStateInterfaceGraphics._rscala.x;
    gGlobalStateInterfaceGraphics.bigposy[specialmessage->layerid] =
      specialmessage->y * gGlobalStateInterfaceGraphics._rscala.y;
    gGlobalStateInterfaceGraphics.bigx[specialmessage->layerid] = 100;
  }
  else // world text
  {
    gGlobalStateInterfaceGraphics.worldtext[specialmessage->layerid] = cs;
    gGlobalStateInterfaceGraphics.worlddelay[specialmessage->layerid] = specialmessage->delay;
    gGlobalStateInterfaceGraphics.worldscale[specialmessage->layerid] = specialmessage->scale;
    gGlobalStateInterfaceGraphics.worldcolor[specialmessage->layerid] = specialmessage->color;
    gGlobalStateInterfaceGraphics.worldposx[specialmessage->layerid] =
      specialmessage->x * gGlobalStateInterfaceGraphics._rscala.x;
    gGlobalStateInterfaceGraphics.worldposy[specialmessage->layerid] =
      specialmessage->y * gGlobalStateInterfaceGraphics._rscala.y;
    gGlobalStateInterfaceGraphics.worldx[specialmessage->layerid] = 100;
  }
}

#pragma region tests
#include <doctest/doctest.h>

namespace
{

class NetworkClientMessagesFixture
{
public:
  NetworkClientMessagesFixture() = default;
  ~NetworkClientMessagesFixture() = default;
  NetworkClientMessagesFixture(const NetworkClientMessagesFixture&) = delete;
protected:
};

TEST_SUITE("NetworkClientMessagesSuite")
{

TEST_CASE_FIXTURE(NetworkClientMessagesFixture, "wstring_convert being deprecated" * doctest::skip(false))
{
  auto text = utf8_to_utf16("Żółć!");
  std::u16string result = u"Żółć!";
  CHECK_EQ(text, result);
}

} // end of NetworkClientMessagesSuite
} // end of unnamed namespace
#pragma endregion tests