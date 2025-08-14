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
  for (i = 1; i <= high(bannediplist); i++)
  {
    if (bannediplist[i].ip.empty())
    {
      findex = i;
      break;
    }
  }

  if (findex == 0)
  {
    setlength(bannediplist, high(bannediplist) + 2);
    bannediplist[high(bannediplist)].ip = ip;
    bannediplist[high(bannediplist)].time = duration;
    bannediplist[high(bannediplist)].reason = reason;
  }
  else
  {
    bannediplist[findex].ip = ip;
    bannediplist[findex].time = duration;
    bannediplist[findex].reason = reason;
  }
  lastban = ip;
}

auto GlobalStateBanSystem::delbannedip(const std::string &ip) -> bool
{
  std::int32_t i;

  bool result;
  result = false;
  for (i = 1; i <= high(bannediplist); i++)
  {
    if ((bannediplist[i].ip == ip) && (!ip.empty()))
    {
      bannediplist[i].ip = "";
      bannediplist[i].time = permanent;
      bannediplist[i].reason = "";
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
  for (i = 1; i <= high(bannediplist); i++)
  {
    if (matchesmask(ip, bannediplist[i].ip))
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
  for (i = 1; i <= high(bannediplist); i++)
  {
    if (matchesmask(ip, bannediplist[i].ip))
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
  for (auto j = 1; j <= high(bannediplist); j++)
  {
    if (!bannediplist[j].ip.empty())
    {
      if (bannediplist[j].time > 0)
      {
        bannediplist[j].time -= 3600;
        if ((bannediplist[j].time < 0) && !(bannediplist[j].time == permanent))
        {
          bannediplist[j].time = 0;
        }
      }
      if ((bannediplist[j].time == 0) && !(bannediplist[j].time == permanent))
      {
        GS::GetMainConsole().console(std::string("IP number ") + bannediplist[j].ip + " (" +
                                       bannediplist[j].reason + ") unbanned",
                                     client_message_color);
        delbannedip(bannediplist[j].ip);
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
  for (i = 1; i <= high(bannedhwlist); i++)
  {
    if (bannedhwlist[i].hw.empty())
    {
      findex = i;
      break;
    }
  }

  if (findex == 0)
  {
    setlength(bannedhwlist, high(bannedhwlist) + 2);
    bannedhwlist[high(bannedhwlist)].hw = hw;
    bannedhwlist[high(bannedhwlist)].time = duration;
    bannedhwlist[high(bannedhwlist)].reason = reason;
  }
  else
  {
    bannedhwlist[findex].hw = hw;
    bannedhwlist[findex].time = duration;
    bannedhwlist[findex].reason = reason;
  }
  lastbanhw = hw;
}

auto GlobalStateBanSystem::delbannedhw(const std::string &hw) -> bool
{
  std::int32_t i;

  bool result;
  result = false;
  for (i = 1; i <= high(bannedhwlist); i++)
  {
    if ((bannedhwlist[i].hw == hw) && (!hw.empty()))
    {
      bannedhwlist[i].hw = "";
      bannedhwlist[i].time = permanent;
      bannedhwlist[i].reason = "";
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
  for (i = 1; i <= high(bannedhwlist); i++)
  {
    if (matchesmask(hw, bannedhwlist[i].hw))
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
  for (i = 1; i <= high(bannedhwlist); i++)
  {
    if (matchesmask(hw, bannedhwlist[i].hw))
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
  for (auto j = 1; j <= high(bannedhwlist); j++)
  {
    if (!bannedhwlist[j].hw.empty())
    {
      if (bannedhwlist[j].time > 0)
      {
        bannedhwlist[j].time -= 3600;
        if ((bannedhwlist[j].time < 0) && !(bannedhwlist[j].time == permanent))
        {
          bannedhwlist[j].time = 0;
        }
      }
      if ((bannedhwlist[j].time == 0) && !(bannedhwlist[j].time == permanent))
      {
        GS::GetMainConsole().console(std::string("Hardware ID ") + bannedhwlist[j].hw + " (" +
                                       bannedhwlist[j].reason + ") unbanned",
                                     client_message_color);
        delbannedhw(bannedhwlist[j].hw);
        savetxtlists();
      }
    }
  }
}
