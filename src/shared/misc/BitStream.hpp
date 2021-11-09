#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include <string>

class BitStream
{
  public:
    using BitBuffer = std::vector<std::uint8_t>;

    BitStream(const std::uint8_t *data, const std::size_t size)
    {
        Buffer.reserve(size);
        for (auto i = 0u; i < size; i++)
        {
            Buffer.push_back(data[i]);
        }
    }

    BitStream()
    {
        Buffer.reserve(INITIAL_BUFFER_SIZE);
    }

    ~BitStream()
    {
        Buffer.clear();
    }

    const BitBuffer &Data()
    {
        return Buffer;
    }

    template <typename T>
    void Write(const T& val)
    {
        Write<T, sizeof(T) * 8>(val);
    }

    template<>
    void Write<bool>(const bool& val)
    {
        std::uint8_t v = val ? 0xFF : 0x00;
        Write<std::uint8_t, 1>(v);
    }

    template<>
    void Write<std::string>(const std::string& val)
    {
        for(const auto& v : val)
        {
            Write(v);
        }
        Write('\0');
    }


    template <typename T>
    bool Read(T &val)
    {
        return Read<T, sizeof(T) * 8>(val);
    }

    template<>
    bool Read<bool>(bool &val)
    {
        auto ret = Read<bool, 1>(val);
        val = *reinterpret_cast<std::uint8_t*>(&val) != 0;
        return ret;
    }

    template<>
    bool Read<std::string>(std::string& val)
    {
        char v = 0;
        bool ret = true;
        do
        {
            ret = Read(v);
            if (ret != 0)
            {
                val.push_back(v);
            }
        } while(ret && v != 0);
        return ret;
    }

  private:
    BitBuffer Buffer;
    std::size_t BitPos = 0;
    static constexpr auto INITIAL_BUFFER_SIZE = 100;

    std::int32_t GetBit(const std::uint8_t &b, std::int32_t bitNo)
    {
        constexpr std::array<std::uint8_t, 8> mask = {0x01, 0x02, 0x04, 0x08,
                                                      0x10, 0x20, 0x40, 0x80};
        return b & mask[bitNo] ? 1 : 0;
    }

    void SetBit(std::uint8_t &b, std::int32_t bitNo, std::int32_t bit)
    {
        constexpr std::array<std::uint8_t, 8> mask = {0x01, 0x02, 0x04, 0x08,
                                                      0x10, 0x20, 0x40, 0x80};
        if (bit == 1)
        {
            b |= mask[bitNo];
        }
        else
        {
            b &= ~mask[bitNo];
        }
    }

    template <typename T, std::uint32_t sizeInBits>
    bool Read(T &val)
    {
        val = 0;
        std::uint8_t *outBuffer = reinterpret_cast<std::uint8_t *>(&val);
        for (auto i = 0; i < sizeInBits; i++)
        {
            auto idx = BitPos / 8;
            auto bit = 7 - BitPos % 8;
            auto &tmp = Buffer[idx];
            auto idxOut = (sizeInBits - i - 1) / 8;
            auto bitOut = 7 - i % 8;

            SetBit(outBuffer[idxOut], bitOut, GetBit(tmp, bit));

            BitPos++;
        }
        return true;
    }

    template <typename T, std::uint32_t sizeInBits>
    void Write(const T& val)
    {
        const std::uint8_t *inBuffer = reinterpret_cast<const std::uint8_t *>(&val);
        for (auto i = 0; i < sizeInBits; i++)
        {
            auto idx = BitPos / 8;
            if (idx == Buffer.size())
            {
                Buffer.push_back(0);
            }
            auto bit = 7 - BitPos % 8;
            auto &tmp = Buffer[idx];
            auto idxIn = (sizeInBits - i - 1) / 8;
            auto bitIn = 7 - i % 8;

            SetBit(tmp, bit, GetBit(inBuffer[idxIn], bitIn));

            BitPos++;
        }
    }
};
