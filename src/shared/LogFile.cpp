// automatically converted
#include "LogFile.hpp"
#include "Cvar.hpp"
#include "common/FileUtility.hpp"
#include "common/Logging.hpp"
#include <sstream>

namespace
{
std::mutex loglock;
}

template <Config::Module M>
void newlogfile(FileUtility& fu, tstringlist *f, const std::string &name)
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

  auto logfile = fu.Open(name, FileUtility::FileMode::Write);
  if (logfile != nullptr)
  {
    LogErrorG("File logging error {}", name);
// TODO error logging once mainconsole is ready
#if 0
#ifdef SERVER
        output << string("File logging error (N): ") + inttostr(i) << NL;
#else
        GS::GetMainConsole().console(string("File logging error (N): ") + inttostr(i), debug_message_color);
#endif
#endif
  }
  fu.Close(logfile);
}

template <Config::Module M>
void addlinetologfile(FileUtility& fu, tstringlist *f, const std::string &s, const std::string &name, bool withdate)
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
#if __EMSCRIPTEN__
      NotImplemented("wasm");
#else
      ss << std::put_time(std::localtime(&now), "%y/%m/%d %H:%M:%S");
#endif
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
    writelogfile(fu, f, name);
  }
}

template <Config::Module M>
void writelogfile(FileUtility& fu, tstringlist *f, const std::string &name)
{
  if (not CVar::log_enable)
  {
    return;
  }

  if (not f)
  {
    return;
  }

  if (f->size() > 1000000)
  {
    return;
  }

  auto logfile = fu.Open(name, FileUtility::FileMode::Write);
  {
    std::lock_guard lock(loglock);
    for (auto &line : *f)
    {
      fu.Write(logfile, reinterpret_cast<const std::byte*>(line.c_str()), line.size());
      fu.Write(logfile, reinterpret_cast<const std::byte*>('\n'), 1);
    }
    f->clear();
  }
  fu.Close(logfile);
}

template <Config::Module M>
void newlogfiles(FileUtility &fu)
{
  std::int32_t j;
  std::string s2;

  {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream ss;
#if __EMSCRIPTEN__
    NotImplemented("wasm");
#else
    ss << std::put_time(std::localtime(&now), "%y-%m-%d");
#endif
    s2 = ss.str();
  }

  std::string consolelogfilename = "/user/logs/consolelog-" + s2 + ".txt";
  j = 1;
  for (auto i = 1; fu.Exists(consolelogfilename); i++)
  {
    consolelogfilename = "/user/logs/consolelog-" + s2 + "-" + std::to_string(j) + ".txt";
  }
  if (CVar::log_level == 0)
  {
    consolelogfilename = "/user/logs/consolelog.txt";
  }

  GetGameLogFilename() = consolelogfilename;

  newlogfile(fu, GetGameLog(), consolelogfilename);
  addlinetologfile(fu, GetGameLog(), "   Console Log Started", consolelogfilename);

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

template void newlogfile(FileUtility& fu, tstringlist *f, const std::string &name);
template void writelogfile(FileUtility& fu, tstringlist *f, const std::string &name);
template void addlinetologfile(FileUtility& fu, tstringlist *f, const std::string &s, const std::string &name,
                               bool withdate);
template void newlogfiles(FileUtility& fu);

template tstringlist *&GetGameLog();
template std::string &GetGameLogFilename();
