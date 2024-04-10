#pragma once

#include <cstdint>

class NetworkClient;
struct NetworkContext;

template<typename T_NetworkClient>
void clientrequestgame(T_NetworkClient& network, std::string_view password);
void clientdisconnect();
void clientsendplayerinfo();
void clientpong(std::uint8_t pingnum);
void clienthandleplayerslist(NetworkContext *netmessage);
void clienthandleunaccepted(NetworkContext *netmessage);
void clienthandleserverdisconnect(NetworkContext *netmessage);
void clienthandleping(NetworkContext *netmessage);
void clienthandleservervars(NetworkContext *netmessage);
void clienthandlesynccvars(NetworkContext *netmessage);
