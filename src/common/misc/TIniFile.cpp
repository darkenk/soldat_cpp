#include "TIniFile.hpp"
#include <regex>

TIniFile::TIniFile(std::unique_ptr<TStream> stream) : Stream(std::move(stream))
{
}

bool TIniFile::ReadSectionValues(const std::string_view section, Entries &out)
{
  std::string line;
  std::regex sectionRegex{R"(^\[(.*)\])"};
  std::regex desiredSectionRegex{std::string("^\\[") + std::string(section.data()) + std::string("\\]")};
  std::regex entryRegex{R"((.*)=(.*))"};
  bool sectionFound = false;

  if (Stream == nullptr)
  {
    return false;
  }

  Stream->Reset();
  while (Stream->ReadLine(line))
  {
    line = trim(line);
    if (!(sectionFound || std::regex_match(line, desiredSectionRegex)))
    {
      continue;
    }
    if (!sectionFound)
    {
      sectionFound = true;
      continue;
    }
    if (std::regex_match(line, sectionRegex))
    {
      break;
    }
    std::smatch match;
    if (std::regex_search(line, match, entryRegex))
    {
      out[match[1]] = match[2];
    }
  }
  return sectionFound;
}
