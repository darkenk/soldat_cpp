#pragma once

#include <cstdint>
class NetworkServer;
class TServerPlayer;
struct tmsgheader;

#ifdef SERVER
void serverthingsnapshot(std::uint8_t tonum);
void serverthingmustsnapshot(const uint8_t i);
#endif
void serverthingmustsnapshotonconnect(const uint8_t tonum);
#ifdef SERVER
void serverthingmustsnapshotonconnectto(const uint8_t i, const uint8_t tonum);
void serverthingtaken(const uint8_t i, const uint8_t w);
void serverhandlerequestthing(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
#endif
