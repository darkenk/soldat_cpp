#pragma once
#include "PortUtilsSoldat.hpp"
#include <memory>

template <class T>
class FGlobalSubsystem
{
public:
	FGlobalSubsystem(const FGlobalSubsystem&) = delete;
	FGlobalSubsystem(FGlobalSubsystem&&) = delete;
	FGlobalSubsystem& operator=(const FGlobalSubsystem&) = delete;
	FGlobalSubsystem& operator=(FGlobalSubsystem&&) = delete;
	virtual ~FGlobalSubsystem() = default;
	static void Init()
	{
		SoldatAssert(gSubsystem == nullptr);
		gSubsystem = std::unique_ptr<T>(new T{});
	}

	static void Deinit() { gSubsystem.reset(); }

	static T& Get() { return *gSubsystem; }

protected:
	FGlobalSubsystem() = default;

private:
	static std::unique_ptr<T> gSubsystem;
};

template <class T>
std::unique_ptr<T> FGlobalSubsystem<T>::gSubsystem;
