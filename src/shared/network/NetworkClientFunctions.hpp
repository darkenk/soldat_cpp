#include "common/network/Net.hpp"

#pragma once

struct NetworkContext;

class clienthandlevoteon : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandlevoteoff : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleserversyncmsg : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleforceposition : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleforcevelocity : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleforceweapon : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleweaponactivemessage : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleclientfreecam : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandlejoinserver : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleplaysound : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
