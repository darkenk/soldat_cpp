#pragma once

#include "common/network/Net.hpp"

struct NetworkContext;

class ClientHandleHeartbeat : public INetMessageHandler
{
public:
    void Handle(NetworkContext *nc) override;
};
