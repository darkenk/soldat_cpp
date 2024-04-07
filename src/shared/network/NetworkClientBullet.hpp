#pragma once

#include <cstdint>

struct NetworkContext;

void clientsendbullet(std::uint8_t i);
void clienthandlebulletsnapshot(NetworkContext *netmessage);
