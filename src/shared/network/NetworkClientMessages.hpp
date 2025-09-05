#pragma once

#include <string>
#include <cinttypes>
#include <cstdint>

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

struct NetworkContext;

void clientsendstringmessage(const std::string &text, std::uint8_t msgtype);
class clienthandlechatmessage : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandlespecialmessage : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
