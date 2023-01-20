/*
    sha1.hpp - source code of

    ============
    SHA-1 in C++
    ============

    100% Public Domain.

    Original C Code
        -- Steve Reid <steve@edmweb.com>
    Small changes to fit into bglibs
        -- Bruce Guenter <bruce@untroubled.org>
    Translation to simpler C++ Code
        -- Volker Diels-Grabsch <v@njh.eu>
    Safety fixes
        -- Eugene Hopkinson <slowriot at voxelstorm dot com>
    Header-only library
        -- Zlatko Michailov <zlatko@michailov.org>
*/

#pragma once

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <array>

class SHA1
{
public:
  SHA1();
  void update(const std::string &s);
  void update(std::istream &is);
  std::array<std::uint32_t, 5> final();
  static std::array<std::uint32_t, 5> from_file(const std::string &filename);

private:
  uint32_t digest[5];
  std::string buffer;
  uint64_t transforms;
};
