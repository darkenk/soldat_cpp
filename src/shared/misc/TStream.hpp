#pragma once
#include <string>

class TStream
{
  public:
    virtual ~TStream() = default;
    virtual bool ReadLine(std::string &out) = 0;
    virtual void Reset() = 0;
};
