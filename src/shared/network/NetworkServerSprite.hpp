#pragma once

#include "../Constants.hpp"
#include "NetworkServer.hpp"
#include <array>
#include <cstdint>
#include <steam/steamnetworkingtypes.h>

void serverspritesnapshot(std::uint8_t r);
void serverspritesnapshotmajor(std::uint8_t r);
void serverspritesnapshotmajorfloat(const uint8_t who, std::uint8_t r);
void serverskeletonsnapshot(std::uint8_t r);
void serverspritedeltas(const uint8_t i);
void serverspritedeltasmouse(std::uint8_t i);
void serverspritedeath(std::int32_t who, std::int32_t killer, std::int32_t bulletnum,
                       std::int32_t where);
void serverhandleclientspritesnapshot_dead(SteamNetworkingMessage_t *netmessage);
void serverhandleclientspritesnapshot_mov(SteamNetworkingMessage_t *netmessage);
void serverhandleclientspritesnapshot(SteamNetworkingMessage_t *netmessage);

extern std::array<std::array<tmsg_serverspritedelta_movement, Constants::MAX_SPRITES>,
                  Constants::MAX_SPRITES>
  oldmovementmsg;
extern std::array<std::array<tmsg_serverspritedelta_mouseaim, Constants::MAX_SPRITES>,
                  Constants::MAX_SPRITES>
  oldmouseaimmsg;
extern std::array<std::array<tmsg_serverspritedelta_weapons, Constants::MAX_SPRITES>,
                  Constants::MAX_SPRITES>
  oldweaponsmsg;
extern std::array<std::array<tmsg_serverspritedelta_helmet, Constants::MAX_SPRITES>,
                  Constants::MAX_SPRITES>
  oldhelmetmsg;
extern std::array<tmsg_serverspritesnapshot, Constants::MAX_SPRITES> oldspritesnapshotmsg;
extern std::array<std::int32_t, Constants::MAX_SPRITES> time_spritesnapshot;
extern std::array<std::int32_t, Constants::MAX_SPRITES> time_spritesnapshot_mov;
