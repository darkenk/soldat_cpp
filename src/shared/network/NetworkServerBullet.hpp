#pragma once

#include <cstdint>

class NetworkServer;
class TServerPlayer;
struct tmsgheader;

void serverbulletsnapshot(const std::uint8_t i, std::uint8_t tonum, bool forced);
#ifdef SERVER
void serverhandlebulletsnapshot(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
#endif
