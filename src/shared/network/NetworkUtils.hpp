#pragma once

#include "../mechanics/Sprites.hpp"
#include "../misc/PortUtils.hpp"
#include <string>

bool iswronggameversion(std::string RequestVersion);
bool verifypacket(std::int32_t ValidSize, std::int32_t ReceiveSize, std::int32_t PacketId,
                  const source_location &location = source_location::current());
bool verifypacketlargerorequal(std::int32_t ValidSize, std::int32_t ReceiveSize,
                               std::int32_t PacketId,
                               const source_location &location = source_location::current());

#ifdef SERVER
extern bool isremoteadminip(const std::string &IP);
extern bool isadminip(const std::string &IP);
extern bool isadminpassword(std::string Password);
#endif

void encodekeys(tsprite &SpriteC, std::uint16_t &Keys16);
void decodekeys(tsprite &SpriteC, std::uint16_t Keys16);

std::string fixplayername(const char *);
#ifdef SERVER
bool checkweaponnotallowed(std::uint8_t i);
std::int32_t findfloodid(std::string SrcIP);
std::int32_t addfloodip(std::string SrcIP);
std::int32_t updateantiflood(std::string SrcIP);
bool isfloodid(std::int32_t ID);

bool addiptoremoteadmins(std::string SrcIP);
bool isservertotallyfull();
bool isserverfull();
bool iswronggamepassword(std::string GamePassword);
#endif

#ifndef SERVER
void playerradiosound(std::uint8_t RadioID);
void newplayerweapon();
std::string returnfixedplayername(std::string name);
#endif

void stringtoarray(char *c, std::string s);
