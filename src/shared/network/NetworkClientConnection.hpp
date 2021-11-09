#pragma once

#include <cstdint>
#include <steam/isteamnetworkingmessages.h>

void clientrequestgame();
void clientdisconnect();
void clientsendplayerinfo();
void clientpong(std::uint8_t pingnum);
void clienthandleplayerslist(SteamNetworkingMessage_t *netmessage);
void clienthandleunaccepted(SteamNetworkingMessage_t *netmessage);
void clienthandleserverdisconnect(SteamNetworkingMessage_t *netmessage);
void clienthandleping(SteamNetworkingMessage_t *netmessage);
void clienthandleservervars(SteamNetworkingMessage_t *netmessage);
void clienthandlesynccvars(SteamNetworkingMessage_t *netmessage);
