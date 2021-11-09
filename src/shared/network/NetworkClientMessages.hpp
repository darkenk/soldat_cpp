#pragma once

#include <steam/isteamnetworkingmessages.h>
#include <string>

void clientsendstringmessage(const std::string &text, std::uint8_t msgtype);
void clienthandlechatmessage(SteamNetworkingMessage_t *netmessage);
void clienthandlespecialmessage(SteamNetworkingMessage_t *netmessage);
