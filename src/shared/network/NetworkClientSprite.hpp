#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

#pragma once

struct NetworkContext;

void clientspritesnapshot();
void clientspritesnapshotmov();
void clientspritesnapshotdead();
class clienthandleserverspritesnapshot : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleserverspritesnapshot_major : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleserverskeletonsnapshot : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};

class clienthandlespritedeath : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandledelta_movement : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandledelta_mouseaim : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandledelta_weapons : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandledelta_helmet : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleclientspritesnapshot_dead : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
