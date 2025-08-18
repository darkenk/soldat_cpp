#pragma once

#include <cstdint>
#include <string_view>

struct NetworkContext;
class INetwork;

void clientrequestgame(INetwork& network, std::string_view password);
void clientdisconnect(INetwork& client);
void clientsendplayerinfo();
class ClientPongMsg final
{
public:
    explicit ClientPongMsg(INetwork& network):mNetwork{network} {};
    ClientPongMsg() = delete;
    ClientPongMsg(ClientPongMsg&) = default;
    ClientPongMsg(ClientPongMsg&&) = default;
    ~ClientPongMsg() = default;
    void send(const std::uint8_t pingnum);

private:
    INetwork& mNetwork;
};
void clientpong(INetwork& network, std::uint8_t pingnum);
void clienthandleplayerslist(NetworkContext *netmessage);
void clienthandleunaccepted(NetworkContext *netmessage);
void clienthandleserverdisconnect(NetworkContext *netmessage);
void clienthandleping(NetworkContext *netmessage);
void clienthandleservervars(NetworkContext *netmessage);
void clienthandlesynccvars(NetworkContext *netmessage);
