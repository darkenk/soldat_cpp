#pragma once

#include <array>
#include <cstdint>

#include "../Constants.hpp"
#include "NetworkServer.hpp"
#include "common/Constants.hpp"
#include "common/network/Net.hpp"

class NetworkServer;
class TServerPlayer;

void serverspritesnapshot(std::uint8_t r);
void serverspritesnapshotmajor(std::uint8_t r);
void serverspritesnapshotmajorfloat(const uint8_t who, std::uint8_t r);
void serverskeletonsnapshot(std::uint8_t r);
void serverspritedeltas(const uint8_t i);
void serverspritedeltasmouse(std::uint8_t i);
void serverspritedeath(std::int32_t who, std::int32_t killer, std::int32_t bulletnum,
                       std::int32_t where);
void serverhandleclientspritesnapshot_dead(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
void serverhandleclientspritesnapshot_mov(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);
void serverhandleclientspritesnapshot(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player);

struct GlobalStateNetworkServerSprite
{
  std::array<std::array<tmsg_serverspritedelta_movement, Constants::MAX_SPRITES>,
             Constants::MAX_SPRITES>
    oldmovementmsg;
  std::array<std::array<tmsg_serverspritedelta_mouseaim, Constants::MAX_SPRITES>,
             Constants::MAX_SPRITES>
    oldmouseaimmsg;
  std::array<std::array<tmsg_serverspritedelta_weapons, Constants::MAX_SPRITES>,
             Constants::MAX_SPRITES>
    oldweaponsmsg;
  std::array<std::array<tmsg_serverspritedelta_helmet, Constants::MAX_SPRITES>,
             Constants::MAX_SPRITES>
    oldhelmetmsg;
  std::array<tmsg_serverspritesnapshot, Constants::MAX_SPRITES> oldspritesnapshotmsg;
  std::array<std::int32_t, Constants::MAX_SPRITES> time_spritesnapshot;
  std::array<std::int32_t, Constants::MAX_SPRITES> time_spritesnapshot_mov;
};

extern GlobalStateNetworkServerSprite gGlobalStateNetworkServerSprite;
