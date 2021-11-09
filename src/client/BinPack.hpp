#pragma once
// automatically converted

#include <cstddef>
#include <cstdint>
#include <vector>

typedef struct tbprect *pbprect;
struct tbprect
{
    tbprect(std::int32_t width = 0, std::int32_t height = 0)
        : x(0), y(0), w(width), h(height), data(nullptr)
    {
    }
    std::int32_t x, y;
    std::int32_t w, h;
    std::uint8_t *data;
};

typedef std::vector<tbprect> tbprectarray;

std::int32_t packrects(std::int32_t w, std::int32_t h, tbprectarray &rects);
