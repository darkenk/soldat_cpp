#pragma once

#include <cstdint>
#include <new>

template <typename T>
void getmem(T *&pointer, std::size_t size)
{
    pointer = reinterpret_cast<T *>(new std::uint8_t[size]);
}

template <typename T>
void freemem(T *&pointer)
{
    delete[] pointer;
    pointer = nullptr;
}
