#include "TMemoryStream.hpp"
#include <sstream>

class MemoryStream : public TStream
{
public:
  MemoryStream(const std::string_view content) : Stream(content.data())
  {
  }

  ~MemoryStream() = default;

  bool ReadLine(std::string &out) override
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

std::unique_ptr<TStream> ReadAsMemoryStream(const std::string_view &content)
{
  return std::make_unique<MemoryStream>(content);
}
