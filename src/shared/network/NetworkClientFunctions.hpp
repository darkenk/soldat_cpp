#include "common/network/Net.hpp"

#pragma once

struct NetworkContext;

class clienthandlevoteon : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandlevoteoff : public INetMessageHandler
{
public:
	void Handle(NetworkContext* nc) override;
};
class clienthandleserversyncmsg : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandleforceposition : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandleforcevelocity : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandleforceweapon : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandleweaponactivemessage : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandleclientfreecam : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandlejoinserver : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandleplaysound : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
