#include "TMemoryStream.hpp"
#include <sstream>

class MemoryStream : public TStream
{
public:
  MemoryStream(const std::string_view content) : Stream(content.data())
  {
  }

  ~MemoryStream() override = default;

  auto ReadLine(std::string &out) -> bool override
  {
    if (Stream.eof())
    {
      return false;
    }
    std::getline(Stream, out);
    return true;
  }
  void Reset() override
  {
    Stream.clear(std::istringstream::goodbit);
    Stream.seekg(0);
  }

private:
  std::istringstream Stream;
};

auto ReadAsMemoryStream(const std::string_view &content) -> std::unique_ptr<TStream>
{
  return std::make_unique<MemoryStream>(content);
}
