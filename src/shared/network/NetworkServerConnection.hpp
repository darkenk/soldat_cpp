#pragma once

#include "../misc/SoldatConfig.hpp"
#include <cstdint>
#include <steam/steamnetworkingtypes.h>
#include <string>

void serverhandlerequestgame(SteamNetworkingMessage_t *netmessage) requires(Config::IsServer());
void serverhandleplayerinfo(SteamNetworkingMessage_t *netmessage) requires(Config::IsServer());

void serversendplaylist(HSteamNetConnection peer);
void serversendnewplayerinfo(std::uint8_t num, std::uint8_t jointype);

std::string getbanstrforindex(std::int32_t banindex,
                              bool banhw = false) requires(Config::IsServer()); // TODO move?
void serversendunaccepted(HSteamNetConnection peer, std::uint8_t state,
                          std::string message = "") requires(Config::IsServer());
void serverdisconnect() requires(Config::IsServer());
void serverplayerdisconnect(std::uint8_t num, std::uint8_t why);

void serverping(std::uint8_t tonum);
void serversynccvars(std::uint8_t tonum, HSteamNetConnection peer, bool fullsync = false);
void servervars(std::uint8_t tonum);

void serverhandlepong(SteamNetworkingMessage_t *netmessage) requires(Config::IsServer());

void serverplayerdisconnect(std::uint8_t num, std::uint8_t why);
