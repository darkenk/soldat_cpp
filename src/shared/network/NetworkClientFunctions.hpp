#pragma once

struct NetworkContext;

void clienthandlevoteon(NetworkContext *netmessage);
void clienthandlevoteoff(NetworkContext *netmessage);
void clienthandleserversyncmsg(NetworkContext *netmessage);
void clienthandleforceposition(NetworkContext *netmessage);
void clienthandleforcevelocity(NetworkContext *netmessage);
void clienthandleforceweapon(NetworkContext *netmessage);
void clienthandleweaponactivemessage(NetworkContext *netmessage);
void clienthandleclientfreecam(NetworkContext *netmessage);
void clienthandlejoinserver(NetworkContext *netmessage);
void clienthandleplaysound(NetworkContext *netmessage);
