#pragma once

#include <cstdint>
#include <string_view>

template<typename T>
class NetworkBase;
struct NetworkContext;

template<typename T>
void clientrequestgame(NetworkBase<T>& network, std::string_view password);
template <typename T>
void clientdisconnect(NetworkBase<T> &client);
void clientsendplayerinfo();
template<typename T>
void clientpong(NetworkBase<T>& network, std::uint8_t pingnum);
void clienthandleplayerslist(NetworkContext *netmessage);
void clienthandleunaccepted(NetworkContext *netmessage);
void clienthandleserverdisconnect(NetworkContext *netmessage);
void clienthandleping(NetworkContext *netmessage);
void clienthandleservervars(NetworkContext *netmessage);
void clienthandlesynccvars(NetworkContext *netmessage);
