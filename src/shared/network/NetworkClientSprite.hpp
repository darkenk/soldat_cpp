#pragma once

#include <steam/isteamnetworkingmessages.h>

void clientspritesnapshot();
void clientspritesnapshotmov();
void clientspritesnapshotdead();
void clienthandleserverspritesnapshot(SteamNetworkingMessage_t *netmessage);
void clienthandleserverspritesnapshot_major(SteamNetworkingMessage_t *netmessage);
void clienthandleserverskeletonsnapshot(SteamNetworkingMessage_t *netmessage);

void clienthandlespritedeath(SteamNetworkingMessage_t *netmessage);
void clienthandledelta_movement(SteamNetworkingMessage_t *netmessage);
void clienthandledelta_mouseaim(SteamNetworkingMessage_t *netmessage);
void clienthandledelta_weapons(SteamNetworkingMessage_t *netmessage);
void clienthandledelta_helmet(SteamNetworkingMessage_t *netmessage);
void clienthandleclientspritesnapshot_dead(SteamNetworkingMessage_t *netmessage);
