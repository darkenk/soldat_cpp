#pragma once

#include <cstdint>

class NetworkClient;
struct NetworkContext;

void clientrequestgame(NetworkClient& network);
void clientdisconnect();
void clientsendplayerinfo();
void clientpong(std::uint8_t pingnum);
void clienthandleplayerslist(NetworkContext *netmessage);
void clienthandleunaccepted(NetworkContext *netmessage);
void clienthandleserverdisconnect(NetworkContext *netmessage);
void clienthandleping(NetworkContext *netmessage);
void clienthandleservervars(NetworkContext *netmessage);
void clienthandlesynccvars(NetworkContext *netmessage);
