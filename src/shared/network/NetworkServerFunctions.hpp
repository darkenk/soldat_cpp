#pragma once

#include <cstdint>
#include <steam/isteamnetworkingmessages.h>
#include <string>

void setweaponactive(std::uint8_t id, std::uint8_t weaponnum, bool state);
void forceweapon(std::uint8_t id, std::uint8_t primary, std::uint8_t secondary, std::uint8_t ammo,
                 std::uint8_t secammo);
void moveplayer(const uint8_t id, float x, float y);
void modifyplayervelocity(const uint8_t id, float velx, float vely);
void forwardclient(std::uint8_t id, std::string targetip, std::int32_t targetport,
                   std::string showmsg);
void playsound(std::uint8_t id, std::string name, float x, float y);
void serverhandleclientfreecam(SteamNetworkingMessage_t *netmessage);
