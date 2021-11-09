#pragma once

#include <steam/isteamnetworkingmessages.h>

void clienthandleserverthingsnapshot(SteamNetworkingMessage_t *netmessage);
void clienthandleserverthingmustsnapshot(SteamNetworkingMessage_t *netmessage);
void clienthandlethingtaken(SteamNetworkingMessage_t *netmessage);
