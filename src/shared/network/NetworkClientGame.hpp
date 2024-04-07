#pragma once

#include <cstdint>
#include <string>

struct NetworkContext;

void clienthandlenewplayer(NetworkContext *netmessage);
void clientvotekick(std::uint8_t num, bool ban, std::string reason);
void clientvotemap(uint32_t mapid);
void clienthandlevoteresponse(NetworkContext *netmessage);
void clientfreecamtarget();
void clienthandleplayerdisconnect(NetworkContext *netmessage);
void clienthandlemapchange(NetworkContext *netmessage);
void clienthandleflaginfo(NetworkContext *netmessage);
void clienthandleidleanimation(NetworkContext *netmessage);
#ifdef STEAM
void clientsendvoicedata(pointer data, std::uint64_t datasize);
void clienthandlevoicedata(NetworkContext *netmessage);
#endif
