#pragma once

#include <cstdint>
#include <string>

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

struct NetworkContext;

class clienthandlenewplayer : public INetMessageHandler
{
public:
  void Handle(NetworkContext *netmessage) override;
};
void clientvotekick(std::uint8_t num, bool ban, std::string reason);
void clientvotemap(uint32_t mapid);
class clienthandlevoteresponse : public INetMessageHandler
{
public:
  void Handle(NetworkContext *netmessage) override;
};
void clientfreecamtarget();
class clienthandleplayerdisconnect : public INetMessageHandler
{
public:
  void Handle(NetworkContext *netmessage) override;
};
class clienthandlemapchange : public INetMessageHandler
{
public:
  void Handle(NetworkContext *netmessage) override;
};
class clienthandleflaginfo : public INetMessageHandler
{
public:
  void Handle(NetworkContext *netmessage) override;
};
class clienthandleidleanimation : public INetMessageHandler
{
public:
  void Handle(NetworkContext *netmessage) override;
};