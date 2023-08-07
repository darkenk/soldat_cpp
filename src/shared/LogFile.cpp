// automatically converted
#include "LogFile.hpp"
#include "Cvar.hpp"
#include "common/Logging.hpp"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
std::mutex loglock;
}

template <Config::Module M>
void newlogfile(tstringlist *f, const std::string &name)
{
  if (not CVar::log_enable)
  {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(loglock);
    if (f != nullptr)
    {
      delete f;
    }
    else
    {
      f = new tstringlist();
    }
  }

  auto logfile = fopen(name.c_str(), "w");
  if (logfile != nullptr)
  {
    LogErrorG("File logging error {}", name);
// TODO error logging once mainconsole is ready
#if 0
#ifdef SERVER
        output << string("File logging error (N): ") + inttostr(i) << NL;
#else
        GetMainConsole().console(string("File logging error (N): ") + inttostr(i), debug_message_color);
#endif
#endif
  }
  fclose(logfile);
}

template <Config::Module M>
void addlinetologfile(tstringlist *f, const std::string &s, const std::string &name, bool withdate)
{
  LogTraceG("{}", s);

  if (not f)
  {
    return;
  }

  if (not CVar::log_enable)
  {
    return;
  }

  if (s.empty())
  {
    return;
  }

  if (CVar::log_level == 0)
  {
    return;
  }

  {
    std::lock_guard lock(loglock);
    if (withdate)
    {
      std::string s2;

      auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      std::stringstream ss;
      ss << std::put_time(std::localtime(&now), "%y/%m/%d %H:%M:%S");
      s2 = s2 + ' ' + ss.str();
      f->push_back(s2 + ' ' + s);
    }
    else
    {
      f->push_back(s);
    }
  }

  if (CVar::log_level > 1)
  {
    writelogfile(f, name);
  }
}

template <Config::Module M>
void writelogfile(tstringlist *f, const std::string &name)
{
  if (not CVar::log_enable)
  {
    return;
  }

  if (not f)
  {
    return;
  }

  std::ofstream logfile(name, std::ios::in | std::ios::ate);

  if (logfile.bad())
  {

    LogErrorG("File logging error (W1): {}", name);
// TODO error logging once mainconsole is ready
#if 0
#ifdef SERVER
        output << string() + inttostr(io) << NL;
#else
        GetMainConsole().console(string("File logging error (W1): ") + inttostr(io),
                            debug_message_color);
#endif
#endif
    return;
  }

  if (f->size() > 1000000)
  {
    return;
  }

  {
    std::lock_guard lock(loglock);
    for (auto &line : *f)
    {
      logfile << line << std::endl;
    }
    f->clear();
  }

  if (logfile.bad())
  {
    LogErrorG("File logging error (W3): {}", name);
#if 0
#ifdef SERVER
        output << string("File logging error (W3): ") + inttostr(io) << NL;
#else
        GetMainConsole().console(string("File logging error (W3): ") + inttostr(io),
                            debug_message_color);
#endif
#endif
  }
}

template <Config::Module M>
void newlogfiles(const std::string &userdirectory)
{
  std::int32_t j;
  std::string s2;

  {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%y-%m-%d");
    s2 = ss.str();
  }

  std::string consolelogfilename = userdirectory + "logs/consolelog-" + s2 + ".txt";
  j = 1;
  for (auto i = 1; std::filesystem::exists(consolelogfilename); i++)
  {
    consolelogfilename = userdirectory + "logs/consolelog-" + s2 + "-" + std::to_string(j) + ".txt";
  }
  if (CVar::log_level == 0)
  {
    consolelogfilename = userdirectory + "logs/consolelog.txt";
  }

  GetGameLogFilename() = consolelogfilename;

  newlogfile(GetGameLog(), consolelogfilename);
  addlinetologfile(GetGameLog(), "   Console Log Started", consolelogfilename);

// TODO error logging once mainconsole is ready
#ifdef SERVER
  NotImplemented("Missing logging");
#if 0
    killlogfilename = format("%slogs/kills/killlog-%s-01.txt", set::of(userdirectory, s2, eos));
    j = 1;
    while (fileexists(killlogfilename))
    {
        j += 1;
        killlogfilename =
            format("%slogs/kills/killlog-%s-%.2d.txt", set::of(userdirectory, s2, j, eos));
    }
    newlogfile(killlog, killlogfilename);
    addlinetologfile(killlog, "   Kill Log Started", killlogfilename);
#endif
#endif
}

template <Config::Module M>
tstringlist *&GetGameLog()
{
  static tstringlist *gamelog = nullptr;
  return gamelog;
}

template <Config::Module M>
std::string &GetGameLogFilename()
{
  static std::string filename;
  return filename;
}

template void newlogfile(tstringlist *f, const std::string &name);
template void writelogfile(tstringlist *f, const std::string &name);
template void addlinetologfile(tstringlist *f, const std::string &s, const std::string &name,
                               bool withdate);
template void newlogfiles(const std::string &userdirectory);

template tstringlist *&GetGameLog();
template std::string &GetGameLogFilename();
