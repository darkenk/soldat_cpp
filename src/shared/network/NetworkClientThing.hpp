#include "common/network/Net.hpp"

#pragma once

struct NetworkContext;

class clienthandleserverthingsnapshot : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandleserverthingmustsnapshot : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
class clienthandlethingtaken : public INetMessageHandler
{
public:
	void Handle(NetworkContext* netmessage) override;
};
