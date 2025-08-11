#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "PortUtilsSoldat.hpp"
#include "TStream.hpp"

class TIniFile
{
public:
  using Entries = std::map<std::string, std::string>;

  TIniFile(std::unique_ptr<TStream> stream);
  bool ReadSectionValues(const std::string_view section, Entries &out);

private:
  std::unique_ptr<TStream> Stream;
};
