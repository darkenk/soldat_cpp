// automatically converted

#include "Waypoints.hpp"
#include "Calc.hpp"
#include "Logging.hpp"
#include <Tracy.hpp>
#include <fstream>

auto operator>>(std::ifstream &in, twaypoint &waypoint) -> std::ifstream &
{
  twaypoint &w = waypoint;
  in >> w.active;
  in >> w.id;
  in >> w.x >> w.y;
  in >> w.left >> w.right >> w.up >> w.down >> w.m2;
  in >> w.pathnum;
  in >> w.c1 >> w.c2 >> w.c3;
  in >> w.connectionsnum;
  for (auto i = w.connections.StartIdx(); i <= w.connections.EndIdx(); i++)
  {
    in >> w.connections[i];
  }
  return in;
}

auto operator<<(std::ofstream &in, twaypoint &waypoint) -> std::ofstream &
{
  twaypoint &w = waypoint;
  in << w.active;
  in << w.id;
  in << w.x << w.y;
  in << w.left << w.right << w.up << w.down << w.m2;
  in << w.pathnum;
  in << w.c1 << w.c2 << w.c3;
  in << w.connectionsnum;
  for (auto i = w.connections.StartIdx(); i <= w.connections.EndIdx(); i++)
  {
    in << w.connections[i];
  }
  return in;
}

void twaypoints::loadfromfile(const std::string &filename)
{
  // file<tpathrec> addrfile;
  std::int32_t i;
  std::int32_t j;

  std::ifstream addrfile(filename, std::ios::binary);
  i = 0;
  while (!addrfile.eof())
  {
    addrfile >> waypoint[i];
    i++;
  }

  if (addrfile.bad())
  {
    LogErrorG("Cannot load waypoint file {}", filename);
  }

  for (i = 0; i < max_waypoints; i++)
  {
    if (!waypoint[i].active)
    {
      waypoint[i].connectionsnum = 0;
      for (j = 0; j < max_connections; j++)
      {
        waypoint[i].connections[j] = 0;
      }
    }
  }
}

void twaypoints::savetofile(const std::string &filename)
{
  std::ofstream addrfile(filename, std::ios::binary | std::ios::trunc);

  for (auto i = 0; i < max_waypoints; i++)
  {
    addrfile << waypoint[i];
  }

  if (addrfile.bad())
  {
    LogDebugG("Cannot save waypoints to {}", filename);
  }
}

auto twaypoints::findclosest(float x, float y, std::int32_t radius,
                             std::int32_t currwaypoint) const -> std::int32_t

{
  ZoneScopedN("TWaypoints::FindClosest");
  float d;
  std::int32_t i;

  std::int32_t result = 0;

  i = 1;
  for (const auto &w : waypoint)
  {
    if (w.active && currwaypoint != i)
    {
      d = distance(x, y, w.x, w.y);
      if (d < radius)
      {
        result = i;
        return result;
      }
    }
    i++;
  }
  return result;
}

auto twaypoints::createwaypoint(std::int32_t sx, std::int32_t sy,
                                std::int32_t spath) -> std::int32_t
{
  std::int32_t i;

  for (i = 0; i < max_waypoints + 1; i++)
  {
    if (i == max_waypoints + 1)
    {
      return -1;
    }
    if (!waypoint[i].active)
    {
      break;
    }
  }
  // i is now the active waypoint

  waypoint[i].active = true;
  waypoint[i].x = sx;
  waypoint[i].y = sy;
  waypoint[i].id = i;
  waypoint[i].connectionsnum = 0;
  waypoint[i].pathnum = spath;

  // activate waypoint
  return i;
}
