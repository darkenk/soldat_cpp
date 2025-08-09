#pragma once

#include <string>
#include <cinttypes>

struct NetworkContext;

void clientsendstringmessage(const std::string &text, std::uint8_t msgtype);
void clienthandlechatmessage(NetworkContext *netmessage);
void clienthandlespecialmessage(NetworkContext *netmessage);
