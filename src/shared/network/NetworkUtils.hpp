#pragma once

#include <cstdint>

#include "common/port_utils/SourceLocation.hpp"
#include <string>

#include "common/misc/PortUtils.hpp"
#include "common/misc/SoldatConfig.hpp" // IWYU pragma: keep

template <Config::Module M>
class Sprite;

template <Config::Module M = Config::GetModule()>
bool iswronggameversion(std::string RequestVersion);
template <Config::Module M = Config::GetModule()>
bool verifypacket(std::int32_t ValidSize, std::int32_t ReceiveSize, std::int32_t PacketId,
                  const source_location &location = source_location::current());
template <Config::Module M = Config::GetModule()>
bool verifypacketlargerorequal(std::int32_t ValidSize, std::int32_t ReceiveSize,
                               std::int32_t PacketId,
                               const source_location &location = source_location::current());

#ifdef SERVER
extern bool isremoteadminip(const std::string &IP);
extern bool isadminip(const std::string &IP);
extern bool isadminpassword(std::string Password);
#endif

template <Config::Module M = Config::GetModule()>
void encodekeys(Sprite<M> &SpriteC, std::uint16_t &Keys16);
template <Config::Module M = Config::GetModule()>
void decodekeys(Sprite<M> &SpriteC, std::uint16_t Keys16);

template <Config::Module M = Config::GetModule()>
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

template <Config::Module M = Config::GetModule()>
void stringtoarray(char *c, std::string s);
