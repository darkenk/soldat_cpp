#include "TFileStream.hpp"
#include "TStream.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

class TFileStream : public TStream
{
  public:
    explicit TFileStream(const std::string_view &filename) : Stream(filename.data())
    {
    }

    ~TFileStream()
    {
    }

    bool ReadLine(std::string &out) override
    {
        std::getline(Stream, out);
        return Stream.good();
    }

    void Reset() override
    {
        Stream.seekg(0);
        Stream.clear();
    }

  private:
    std::ifstream Stream;
};

std::unique_ptr<TStream> ReadAsFileStream(const std::string_view &file)
{
    if (std::filesystem::exists(file))
    {
        return std::make_unique<TFileStream>(file);
    }
    return nullptr;
}
