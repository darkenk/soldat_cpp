#pragma once
#include <entt/signal/dispatcher.hpp>
#include "Console.hpp"
#include "ConsoleMessage.hpp"

class FBigConsoleListener
{
public:
	FBigConsoleListener(const FBigConsoleListener&) = delete;
	FBigConsoleListener(FBigConsoleListener&&) = delete;
	FBigConsoleListener& operator=(const FBigConsoleListener&) = delete;
	FBigConsoleListener& operator=(FBigConsoleListener&&) = delete;
	explicit FBigConsoleListener(std::shared_ptr<entt::dispatcher> InDispatcher, std::shared_ptr<FConsoleBig> InConsole)
		: Console(std::move(InConsole)), Dispatcher(std::move(InDispatcher))
	{
		Dispatcher->sink<FBigConsoleMessage>().connect<&FBigConsoleListener::OnMessage>(this);
	}
	~FBigConsoleListener() { Dispatcher->disconnect(this); }
	void OnMessage(FBigConsoleMessage& InMessage) { Console->ConsoleAdd(InMessage.Message, InMessage.Color); }

private:
	std::shared_ptr<FConsoleBig> Console;
	std::shared_ptr<entt::dispatcher> Dispatcher;
};