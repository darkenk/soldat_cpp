#pragma once

#include <cstdint>
#include <string_view>

#include "common/network/Net.hpp"

#include "common/network/Net.hpp"

struct NetworkContext;
class INetwork;
class NetworkMessageCreator;

void clientrequestgame(INetwork& network, std::string_view password);
void clientdisconnect(INetwork& client);
void clientsendplayerinfo();
class ClientPongMsg final
{
public:
    explicit ClientPongMsg(INetwork& network): mNetwork(network) {}
    ClientPongMsg() = delete;
    ClientPongMsg(ClientPongMsg&) = default;
    ClientPongMsg(ClientPongMsg&&) = default;
    ~ClientPongMsg() = default;
    void send(const std::uint8_t pingnum);

private:
    INetwork& mNetwork;
};
void clientpong(INetwork& network, std::uint8_t pingnum);
class clienthandleplayerslist : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleunaccepted : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleserverdisconnect : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleping : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandleservervars : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
class clienthandlesynccvars : public INetMessageHandler
{
public:
  void Handle(NetworkContext *nc) override;
};
