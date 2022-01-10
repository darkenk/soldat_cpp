#pragma once

#include "../mechanics/Sprites.hpp"
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

extern std::array<std::array<tmsg_serverspritedelta_movement, max_sprites>, max_sprites>
    oldmovementmsg;
extern std::array<std::array<tmsg_serverspritedelta_mouseaim, max_sprites>, max_sprites>
    oldmouseaimmsg;
extern std::array<std::array<tmsg_serverspritedelta_weapons, max_sprites>, max_sprites>
    oldweaponsmsg;
extern std::array<std::array<tmsg_serverspritedelta_helmet, max_sprites>, max_sprites> oldhelmetmsg;
extern std::array<tmsg_serverspritesnapshot, max_sprites> oldspritesnapshotmsg;
extern std::array<std::int32_t, max_sprites> time_spritesnapshot;
extern std::array<std::int32_t, max_sprites> time_spritesnapshot_mov;
