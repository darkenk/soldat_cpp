#pragma once

#include <cstdint>
#include <steam/isteamnetworkingmessages.h>

void clientsendbullet(std::uint8_t i);
void clienthandlebulletsnapshot(SteamNetworkingMessage_t *netmessage);
