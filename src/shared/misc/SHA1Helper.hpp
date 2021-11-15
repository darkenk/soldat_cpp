#pragma once

#include <array>
#include <cstdint>
#include <string>

struct tsha1digest
{
    tsha1digest()
    {
    }

    bool operator==(const tsha1digest& other) const
    {
        return std::equal(std::begin(Dummy), std::end(Dummy), std::begin(other.Dummy), std::end(other.Dummy));
    }

    tsha1digest(const tsha1digest& other)
    {
        std::copy(std::begin(other.Dummy), std::end(other.Dummy), std::begin(Dummy));
    }

    tsha1digest& operator=(const tsha1digest& other)
    {
        std::copy(std::begin(other.Dummy), std::end(other.Dummy), std::begin(Dummy));
        return *this;
    }

    std::array<std::uint32_t, 5> Dummy = {0};
};
static_assert(sizeof(tsha1digest) == 20, "Pascals tsha1digest is 20 byte");

tsha1digest sha1file(const std::string& file);
