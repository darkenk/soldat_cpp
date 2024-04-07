#pragma once

#include <string>

class NetworkServer;
struct TServerPlayer;

void serverhandleplayerdisconnect(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
void servermapchange(std::uint8_t id);
void serverflaginfo(std::uint8_t style, std::uint8_t who);
void serveridleanimation(std::uint8_t num, std::int16_t style);
void serversendvoteon(std::uint8_t votestyle, std::int32_t voter, std::string targetname,
                      std::string reason);
void serversendvoteoff();
void serverhandlevotekick(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
void serverhandlevotemap(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
void serverhandlechangeteam(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
void serversyncmsg(std::int32_t tonum = 0);
#ifdef STEAM
void serverhandlevoicedata(SteamNetworkingMessage_t *netmessage);
#endif
