#pragma once

#include <cstdint>
#include <steam/isteamnetworkingmessages.h>
#include <string>

void clienthandlenewplayer(SteamNetworkingMessage_t *netmessage);
void clientvotekick(std::uint8_t num, bool ban, std::string reason);
void clientvotemap(uint32_t mapid);
void clienthandlevoteresponse(SteamNetworkingMessage_t *netmessage);
void clientfreecamtarget();
void clienthandleplayerdisconnect(SteamNetworkingMessage_t *netmessage);
void clienthandlemapchange(SteamNetworkingMessage_t *netmessage);
void clienthandleflaginfo(SteamNetworkingMessage_t *netmessage);
void clienthandleidleanimation(SteamNetworkingMessage_t *netmessage);
#ifdef STEAM
void clientsendvoicedata(pointer data, std::uint64_t datasize);
void clienthandlevoicedata(SteamNetworkingMessage_t *netmessage);
#endif
