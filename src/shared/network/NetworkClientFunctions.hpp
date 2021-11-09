#pragma once

#include <steam/isteamnetworkingmessages.h>

void clienthandlevoteon(SteamNetworkingMessage_t *netmessage);
void clienthandlevoteoff();
void clienthandleserversyncmsg(SteamNetworkingMessage_t *netmessage);
void clienthandleforceposition(SteamNetworkingMessage_t *netmessage);
void clienthandleforcevelocity(SteamNetworkingMessage_t *netmessage);
void clienthandleforceweapon(SteamNetworkingMessage_t *netmessage);
void clienthandleweaponactivemessage(SteamNetworkingMessage_t *netmessage);
void clienthandleclientfreecam(SteamNetworkingMessage_t *netmessage);
void clienthandlejoinserver(SteamNetworkingMessage_t *netmessage);
void clienthandleplaysound(SteamNetworkingMessage_t *netmessage);
