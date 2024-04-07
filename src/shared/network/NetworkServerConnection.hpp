#pragma once

#include "../misc/SoldatConfig.hpp"
#include <cstdint>
#include <string>

class NetworkServer;
struct tmsg_header;
struct TServerPlayer;

#ifdef SERVER
void serverhandlerequestgame(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
void serverhandleplayerinfo(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
#endif

void serversendplaylist(HSoldatNetConnection peer);
void serversendnewplayerinfo(std::uint8_t num, std::uint8_t jointype);
#ifdef SERVER
std::string getbanstrforindex(std::int32_t banindex,
                              bool banhw = false); // TODO move?
void serversendunaccepted(HSoldatNetConnection peer, std::uint8_t state,
                          const std::string &message = "");
void serverdisconnect();
#endif
void serverplayerdisconnect(std::uint8_t num, std::uint8_t why);

void serverping(std::uint8_t tonum);
void serversynccvars(std::uint8_t tonum, HSoldatNetConnection peer, bool fullsync = false);
void servervars(std::uint8_t tonum);

void serverhandlepong(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);

void serverplayerdisconnect(std::uint8_t num, std::uint8_t why);
