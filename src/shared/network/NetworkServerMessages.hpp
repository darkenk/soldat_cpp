#pragma once

#include <string>

class NetworkServer;
struct TServerPlayer;

void serversendstringmessage(const std::string &text, std::uint8_t tonum, std::uint8_t from,
                             std::uint8_t msgtype);

void serverhandlechatmessage(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);

void serversendspecialmessage(std::string text, std::uint8_t msgtype, std::uint8_t layerid,
                              std::int32_t delay, float scale, std::uint32_t color, float x, float y,
                              std::uint8_t tonum);
