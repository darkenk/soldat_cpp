// automatically converted
#include "NetworkClientMessages.hpp"

#include "../../client/Client.hpp"
#include "../../client/InterfaceGraphics.hpp"
#include "../Game.hpp"
#include "../GameStrings.hpp"
#include "../misc/MemoryUtils.hpp"
#include "NetworkUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <codecvt>
#include <locale>

// utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
template <class Facet>
struct deletable_facet : Facet
{
  template <class... Args>
  deletable_facet(Args &&...args) : Facet(std::forward<Args>(args)...)
  {
  }
  ~deletable_facet()
  {
  }
};

void clientsendstringmessage(const std::string &text, std::uint8_t msgtype)
{
  pmsg_stringmessage pchatmessage;
  std::int32_t size;

  if (length(text) == 0)
    return;

  size = sizeof(pchatmessage->header) + sizeof(pchatmessage->num) + sizeof(pchatmessage->msgtype) +
         2 * length(text) + 2;
  getmem(pchatmessage, size);
  fillchar(pchatmessage, size, 0);
  pchatmessage->header.id = msgid_chatmessage;
  pchatmessage->num = mysprite;
  pchatmessage->msgtype = msgtype;

  std::wstring_convert<deletable_facet<std::codecvt<char16_t, char, std::mbstate_t>>, char16_t>
    conv16;
  std::u16string text16 = conv16.from_bytes(text);

  memcpy(&pchatmessage->text, text16.data(), text16.size() * 2);
  memset(&pchatmessage->text + text16.size() * 2, 0, 2);

  GetNetwork()->senddata(pchatmessage, size, k_nSteamNetworkingSend_Reliable);
  freemem(pchatmessage);
}

void clienthandlechatmessage(SteamNetworkingMessage_t *netmessage)
{
  std::string cs = "";
  std::string prefix = "";
  std::int32_t i, d;
  std::uint8_t msgtype;
  std::uint32_t col;

  NotImplemented("network");
#if 0
    cs = pmsg_stringmessage(netmessage->m_pData)->text.data();
#endif
  i = pmsg_stringmessage(netmessage->m_pData)->num;
  msgtype = pmsg_stringmessage(netmessage->m_pData)->msgtype;

  if (msgtype > msgtype_radio)
    return;
  // chat from server
  if (i == 255)
  {
    GS::GetMainConsole().console(_("*SERVER*: ") + cs, server_message_color);
    return;
  }

  if ((i > 0) && (i < max_players))
    if (!SpriteSystem::Get().GetSprite(i).active)
      return;

  if ((SpriteSystem::Get().GetSprite(i).muted == true) or muteall)
    return;

  chatmessage[i] = cs;
  chatteam[i] = (msgtype == msgtype_team);
  d = std::count(cs.begin(), cs.end(), ' ');

  if (d == 0)
    chatdelay[i] = length(cs) * chardelay;
  else
    chatdelay[i] = d * spacechardelay;

  if (chatdelay[i] > max_chatdelay)
    chatdelay[i] = max_chatdelay;

  col = chat_message_color;

  if (SpriteSystem::Get().GetSprite(i).player->team == team_spectator)
    col = spectator_c_message_color;
  if ((msgtype == msgtype_team) || (msgtype == msgtype_radio))
  {
    col = teamchat_message_color;
    prefix = iif(msgtype == msgtype_radio, _("(RADIO)"), _("(TEAM)")) + " ";
  }

  if (length(cs) < morechattext)
    GS::GetMainConsole().console(
      prefix + "[" + (SpriteSystem::Get().GetSprite(i).player->name) + "] " + cs, col);
  else
  {
    GS::GetMainConsole().console(prefix + "[" + (SpriteSystem::Get().GetSprite(i).player->name) + "] ",
                             col);
    GS::GetMainConsole().console(std::string(" ") + cs, col);
  }

  /*if Radio and
  SpriteSystem::Get().GetSprite(i).IsInSameTeam(SpriteSystem::Get().GetSprite(MySprite)) then
  begin
    PlayRadioSound(RadioID)
  end;*/
}

void clienthandlespecialmessage(SteamNetworkingMessage_t *netmessage)
{
  tmsg_serverspecialmessage *specialmessage;
  std::string cs;

  specialmessage = pmsg_serverspecialmessage(netmessage->m_pData);
  cs = pmsg_serverspecialmessage(netmessage->m_pData)->text.data();

  if (specialmessage->msgtype == 0) // console
  {
    GS::GetMainConsole().console(cs, specialmessage->color);
  }
  else if (specialmessage->msgtype == 1) // big text
  {
    bigtext[specialmessage->layerid] = cs;
    bigdelay[specialmessage->layerid] = specialmessage->delay;
    bigscale[specialmessage->layerid] = specialmessage->scale;
    bigcolor[specialmessage->layerid] = specialmessage->color;
    bigposx[specialmessage->layerid] = specialmessage->x * _rscala.x;
    bigposy[specialmessage->layerid] = specialmessage->y * _rscala.y;
    bigx[specialmessage->layerid] = 100;
  }
  else // world text
  {
    worldtext[specialmessage->layerid] = cs;
    worlddelay[specialmessage->layerid] = specialmessage->delay;
    worldscale[specialmessage->layerid] = specialmessage->scale;
    worldcolor[specialmessage->layerid] = specialmessage->color;
    worldposx[specialmessage->layerid] = specialmessage->x * _rscala.x;
    worldposy[specialmessage->layerid] = specialmessage->y * _rscala.y;
    worldx[specialmessage->layerid] = 100;
  }
}
