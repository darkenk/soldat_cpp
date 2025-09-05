#pragma once

#include <cstdint>

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

struct NetworkContext;

void clientsendbullet(std::uint8_t i);
class clienthandlebulletsnapshot : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
