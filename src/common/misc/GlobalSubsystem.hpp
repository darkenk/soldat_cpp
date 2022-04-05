#pragma once
#include "PortUtilsSoldat.hpp"
#include <memory>

template <class T>
class GlobalSubsystem
{
public:
  static void Init()
  {
    SoldatAssert(Subsystem == nullptr);
    Subsystem = std::unique_ptr<T>(new T{});
  }

  static void Deinit()
  {
    Subsystem.reset();
  }

  static T &Get()
  {
    return *Subsystem;
  }

protected:
  GlobalSubsystem(){};
  GlobalSubsystem(const GlobalSubsystem &) = delete;

private:
  static std::unique_ptr<T> Subsystem;
};

template <class T>
std::unique_ptr<T> GlobalSubsystem<T>::Subsystem;
