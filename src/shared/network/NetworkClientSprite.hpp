#pragma once

struct NetworkContext;

void clientspritesnapshot();
void clientspritesnapshotmov();
void clientspritesnapshotdead();
void clienthandleserverspritesnapshot(NetworkContext *netmessage);
void clienthandleserverspritesnapshot_major(NetworkContext *netmessage);
void clienthandleserverskeletonsnapshot(NetworkContext *netmessage);

void clienthandlespritedeath(NetworkContext *netmessage);
void clienthandledelta_movement(NetworkContext *netmessage);
void clienthandledelta_mouseaim(NetworkContext *netmessage);
void clienthandledelta_weapons(NetworkContext *netmessage);
void clienthandledelta_helmet(NetworkContext *netmessage);
void clienthandleclientspritesnapshot_dead(NetworkContext *netmessage);
