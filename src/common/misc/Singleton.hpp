#pragma once

template <class T>
class Singleton
{
public:
  static T &Get()
  {
    static T obj;
    return obj;
  }

protected:
  Singleton()
  {
  }
};
