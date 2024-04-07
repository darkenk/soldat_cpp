#pragma once

struct NetworkContext;

void clienthandleserverthingsnapshot(NetworkContext *netmessage);
void clienthandleserverthingmustsnapshot(NetworkContext *netmessage);
void clienthandlethingtaken(NetworkContext *netmessage);
