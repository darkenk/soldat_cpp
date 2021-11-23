#pragma once

#include "common/misc/PortUtilsSoldat.hpp"
#include <array>
#include <cstdint>
#include <string>

constexpr std::int32_t max_waypoints = 5000;
constexpr std::int32_t max_connections = 20;

struct twaypoint
{
    bool active;
    std::int32_t id;
    std::int32_t x, y;
    bool left, right, up, down, m2;
    // PathNum: Integer;
    std::int8_t pathnum;
    std::int8_t c1, c2, c3;
    std::int32_t connectionsnum;
    PascalArray<std::int32_t, 1, max_connections> connections;
};

class twaypoints
{
  public:
    PascalArray<twaypoint, 1, max_waypoints> waypoint;

    void loadfromfile(const std::string &filename);
    void savetofile(const std::string &filename);
    std::int32_t findclosest(float x, float y, std::int32_t radius, std::int32_t currwaypoint);
    std::int32_t createwaypoint(std::int32_t sx, std::int32_t sy, std::int32_t spath);
};

// for file use
struct tpathrec
{
    PascalArray<twaypoint, 1, max_waypoints> waypoint;
};
