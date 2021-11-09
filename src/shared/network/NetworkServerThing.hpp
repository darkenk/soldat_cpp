#pragma once

#include <cstdint>
#include <steam/isteamnetworkingmessages.h>

#ifdef SERVER
void serverthingsnapshot(std::uint8_t tonum);
void serverthingmustsnapshot(std::uint8_t i);
#endif
void serverthingmustsnapshotonconnect(std::uint8_t tonum);
#ifdef SERVER
void serverthingmustsnapshotonconnectto(std::uint8_t i, std::uint8_t tonum);
void serverthingtaken(std::uint8_t i, std::uint8_t w);
void serverhandlerequestthing(SteamNetworkingMessage_t *netmessage);
#endif
