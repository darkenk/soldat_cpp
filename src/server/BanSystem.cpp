// automatically converted

#include "BanSystem.hpp"

#include "Server.hpp"
#include "ServerHelper.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"

GlobalStateBanSystem gGlobalStateBanSystem{
  .bannediplist{},
  .bannedhwlist{},
  .lastban{},
  .lastbanhw{},
};

static auto matchesmask([[maybe_unused]] const std::string &v,
                        [[maybe_unused]] const std::string &mask) -> bool
{
  NotImplemented();
  return false;
}

void GlobalStateBanSystem::addbannedip(const std::string &ip, std::string reason,
                                       std::int32_t duration)
{
  std::int32_t i;
  std::int32_t findex;

  findex = 0;
  if (checkbannedip(ip))
  {
    return;
  }
  for (i = 1; i <= high(gGlobalStateBanSystem.bannediplist); i++)
  {
    if (gGlobalStateBanSystem.bannediplist[i].ip.empty())
    {
      findex = i;
      break;
    }
  }

  if (findex == 0)
  {
    setlength(gGlobalStateBanSystem.bannediplist, high(gGlobalStateBanSystem.bannediplist) + 2);
    gGlobalStateBanSystem.bannediplist[high(gGlobalStateBanSystem.bannediplist)].ip = ip;
    gGlobalStateBanSystem.bannediplist[high(gGlobalStateBanSystem.bannediplist)].time = duration;
    gGlobalStateBanSystem.bannediplist[high(gGlobalStateBanSystem.bannediplist)].reason = reason;
  }
  else
  {
    gGlobalStateBanSystem.bannediplist[findex].ip = ip;
    gGlobalStateBanSystem.bannediplist[findex].time = duration;
    gGlobalStateBanSystem.bannediplist[findex].reason = reason;
  }
  gGlobalStateBanSystem.lastban = ip;
}

auto GlobalStateBanSystem::delbannedip(const std::string &ip) -> bool
{
  std::int32_t i;

  bool result;
  result = false;
  for (i = 1; i <= high(gGlobalStateBanSystem.bannediplist); i++)
  {
    if ((gGlobalStateBanSystem.bannediplist[i].ip == ip) && (!ip.empty()))
    {
      gGlobalStateBanSystem.bannediplist[i].ip = "";
      gGlobalStateBanSystem.bannediplist[i].time = permanent;
      gGlobalStateBanSystem.bannediplist[i].reason = "";
      result = true;
    }
  }
  return result;
}

auto GlobalStateBanSystem::checkbannedip(const std::string &ip) -> bool
{
  std::int32_t i;

  bool result;
  result = false;
  for (i = 1; i <= high(gGlobalStateBanSystem.bannediplist); i++)
  {
    if (matchesmask(ip, gGlobalStateBanSystem.bannediplist[i].ip))
    {
      result = true;
      break;
    }
  }
  return result;
}

auto GlobalStateBanSystem::findban(const std::string &ip) -> std::int32_t
{
  std::int32_t i;

  std::int32_t result;
  result = -1;
  for (i = 1; i <= high(gGlobalStateBanSystem.bannediplist); i++)
  {
    if (matchesmask(ip, gGlobalStateBanSystem.bannediplist[i].ip))
    {
      result = i;
      break;
    }
  }
  return result;
}

void GlobalStateBanSystem::loadbannedlist(const std::string &filename)
{
  NotImplemented();
#if 0
    std::int32_t i, j;
    tstringlist temparray;
    tstringarray buff;
    std::int32_t time;

    buff = nullptr;
    temparray = tstringlist.create;
    temparray.loadfromfile(filename);
    setlength(bannediplist, 1);
    j = 1;
    for (i = 1; i <= (temparray.count); i++)
    {
        buff = splitstr(temparray[i - 1], ':', 4);
        if (trim(buff[0]) == "")
            continue;
        setlength(bannediplist, j + 1);
        j += 1;

        time = strtointdef(buff[1], low(std::int32_t));

        if (time == low(std::int32_t))
            continue;

        bannediplist[high(bannediplist)].ip = buff[0];
        bannediplist[high(bannediplist)].time = time;
        bannediplist[high(bannediplist)].reason = buff[2];
    }
    temparray.free;
#endif
}

void GlobalStateBanSystem::savebannedlist(const std::string &filename)
{
  NotImplemented();
#if 0
    std::int32_t i;
    std::string buff;
    textfile savefile;

    buff = "";
    for (i = 1; i <= high(bannediplist); i++)
    {
        if (bannediplist[i].ip != "")
            buff = buff + bannediplist[i].ip + ':' + inttostr(bannediplist[i].time) + ':' +
                   bannediplist[i].reason + chr(13) + chr(10);
    }
    assignfile(savefile, filename);
    rewrite(savefile);
    output << savefile << buff;
    closefile(savefile);
#endif
}

void GlobalStateBanSystem::updateipbanlist()
{
  for (auto j = 1; j <= high(gGlobalStateBanSystem.bannediplist); j++)
  {
    if (!gGlobalStateBanSystem.bannediplist[j].ip.empty())
    {
      if (gGlobalStateBanSystem.bannediplist[j].time > 0)
      {
        gGlobalStateBanSystem.bannediplist[j].time -= 3600;
        if ((gGlobalStateBanSystem.bannediplist[j].time < 0) &&
            !(gGlobalStateBanSystem.bannediplist[j].time == permanent))
        {
          gGlobalStateBanSystem.bannediplist[j].time = 0;
        }
      }
      if ((gGlobalStateBanSystem.bannediplist[j].time == 0) &&
          !(gGlobalStateBanSystem.bannediplist[j].time == permanent))
      {
        GS::GetMainConsole().console(std::string("IP number ") +
                                       gGlobalStateBanSystem.bannediplist[j].ip + " (" +
                                       gGlobalStateBanSystem.bannediplist[j].reason + ") unbanned",
                                     client_message_color);
        delbannedip(gGlobalStateBanSystem.bannediplist[j].ip);
        savetxtlists();
      }
    }
  }
}

// hardware id bans
void GlobalStateBanSystem::addbannedhw(const std::string &hw, std::string reason,
                                       std::int32_t duration)
{
  std::int32_t i;
  std::int32_t findex;

  findex = 0;
  if (checkbannedhw(hw))
  {
    return;
  }
  for (i = 1; i <= high(gGlobalStateBanSystem.bannedhwlist); i++)
  {
    if (gGlobalStateBanSystem.bannedhwlist[i].hw.empty())
    {
      findex = i;
      break;
    }
  }

  if (findex == 0)
  {
    setlength(gGlobalStateBanSystem.bannedhwlist, high(gGlobalStateBanSystem.bannedhwlist) + 2);
    gGlobalStateBanSystem.bannedhwlist[high(gGlobalStateBanSystem.bannedhwlist)].hw = hw;
    gGlobalStateBanSystem.bannedhwlist[high(gGlobalStateBanSystem.bannedhwlist)].time = duration;
    gGlobalStateBanSystem.bannedhwlist[high(gGlobalStateBanSystem.bannedhwlist)].reason = reason;
  }
  else
  {
    gGlobalStateBanSystem.bannedhwlist[findex].hw = hw;
    gGlobalStateBanSystem.bannedhwlist[findex].time = duration;
    gGlobalStateBanSystem.bannedhwlist[findex].reason = reason;
  }
  gGlobalStateBanSystem.lastbanhw = hw;
}

auto GlobalStateBanSystem::delbannedhw(const std::string &hw) -> bool
{
  std::int32_t i;

  bool result;
  result = false;
  for (i = 1; i <= high(gGlobalStateBanSystem.bannedhwlist); i++)
  {
    if ((gGlobalStateBanSystem.bannedhwlist[i].hw == hw) && (!hw.empty()))
    {
      gGlobalStateBanSystem.bannedhwlist[i].hw = "";
      gGlobalStateBanSystem.bannedhwlist[i].time = permanent;
      gGlobalStateBanSystem.bannedhwlist[i].reason = "";
      result = true;
    }
  }
  return result;
}

auto GlobalStateBanSystem::checkbannedhw(const std::string &hw) -> bool
{
  std::int32_t i;

  bool result;
  result = false;
  for (i = 1; i <= high(gGlobalStateBanSystem.bannedhwlist); i++)
  {
    if (matchesmask(hw, gGlobalStateBanSystem.bannedhwlist[i].hw))
    {
      result = true;
      break;
    }
  }
  return result;
}

auto GlobalStateBanSystem::findbanhw(const std::string &hw) -> std::int32_t
{
  std::int32_t i;

  std::int32_t result;
  result = -1;
  for (i = 1; i <= high(gGlobalStateBanSystem.bannedhwlist); i++)
  {
    if (matchesmask(hw, gGlobalStateBanSystem.bannedhwlist[i].hw))
    {
      result = i;
      break;
    }
  }
  return result;
}

void GlobalStateBanSystem::loadbannedlisthw(const std::string &filename)
{
  NotImplemented();
#if 0
    std::int32_t i, j;
    tstringlist temparray;
    tstringarray buff;
    std::int32_t time;

    buff = nullptr;
    temparray = tstringlist.create;
    temparray.loadfromfile(filename);
    setlength(bannedhwlist, 1);
    j = 1;
    for (i = 1; i <= (temparray.count); i++)
    {
        buff = splitstr(temparray[i - 1], ':', 4);
        if (trim(buff[0]) == "")
            continue;

        time = strtointdef(buff[1], low(std::int32_t));

        if (time == low(std::int32_t))
            continue;

        setlength(bannedhwlist, j + 1);
        j += 1;
        bannedhwlist[high(bannedhwlist)].hw = buff[0];
        bannedhwlist[high(bannedhwlist)].time = time;
        bannedhwlist[high(bannedhwlist)].reason = buff[2];
    }
    temparray.free;
#endif
}

void GlobalStateBanSystem::savebannedlisthw(const std::string &filename)
{
  NotImplemented();
#if 0
    std::int32_t i;
    std::string buff;
    textfile savefile;

    buff = "";
    for (i = 1; i <= high(bannedhwlist); i++)
    {
        if (bannedhwlist[i].hw != "")
            buff = buff + bannedhwlist[i].hw + ':' + inttostr(bannedhwlist[i].time) + ':' +
                   bannedhwlist[i].reason + chr(13) + chr(10);
    }
    assignfile(savefile, filename);
    rewrite(savefile);
    output << savefile << buff;
    closefile(savefile);
#endif
}

void GlobalStateBanSystem::updatehwbanlist()
{
  for (auto j = 1; j <= high(gGlobalStateBanSystem.bannedhwlist); j++)
  {
    if (!gGlobalStateBanSystem.bannedhwlist[j].hw.empty())
    {
      if (gGlobalStateBanSystem.bannedhwlist[j].time > 0)
      {
        gGlobalStateBanSystem.bannedhwlist[j].time -= 3600;
        if ((gGlobalStateBanSystem.bannedhwlist[j].time < 0) &&
            !(gGlobalStateBanSystem.bannedhwlist[j].time == permanent))
        {
          gGlobalStateBanSystem.bannedhwlist[j].time = 0;
        }
      }
      if ((gGlobalStateBanSystem.bannedhwlist[j].time == 0) &&
          !(gGlobalStateBanSystem.bannedhwlist[j].time == permanent))
      {
        GS::GetMainConsole().console(std::string("Hardware ID ") +
                                       gGlobalStateBanSystem.bannedhwlist[j].hw + " (" +
                                       gGlobalStateBanSystem.bannedhwlist[j].reason + ") unbanned",
                                     client_message_color);
        delbannedhw(gGlobalStateBanSystem.bannedhwlist[j].hw);
        savetxtlists();
      }
    }
  }
}
