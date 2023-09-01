#include "client/Client.hpp"
#include "common/Logging.hpp"
#include "common/FileUtility.hpp"
#include "common/misc/GlobalSystemsCommon.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <thread>

// clang-format off
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
// clang-format on

void RunTests(int argc, const char *argv[])
{
  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);

  int res = ctx.run();

  SoldatAssert(res == 0);
  if (ctx.shouldExit())
  {
    std::exit(res);
  }
}

int main(int argc, const char *argv[])
{
  InitLogging();
  RunTests(argc, argv);
  GSC::Init();
  GlobalSystems<Config::CLIENT_MODULE>::Init();
  GlobalSystems<Config::SERVER_MODULE>::Init();
  std::thread t1([=]() { RunServer(argc, argv); });
  SetThreadName(t1, "Server");
  SetCurrentThreadName("Client");
  startgame(argc, argv);
  ShutdownServer();
  t1.join();
  GlobalSystems<Config::SERVER_MODULE>::Deinit();
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  GSC::Deinit();
  return 0;
}
