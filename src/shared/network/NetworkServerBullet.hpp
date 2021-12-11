#pragma once

#include <cstdint>
#include <steam/isteamnetworkingmessages.h>

void serverbulletsnapshot(std::uint8_t i, std::uint8_t tonum, bool forced);
#ifdef SERVER
void serverhandlebulletsnapshot(SteamNetworkingMessage_t *netmessage);
#endif
