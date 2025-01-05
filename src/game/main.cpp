#include "client/Client.hpp"
#include "common/Logging.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <thread>

// clang-format off
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
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
  GlobalSystems<Config::CLIENT_MODULE>::Init();
  GlobalSystems<Config::SERVER_MODULE>::Init();
  std::thread serverThread([=]() { RunServer(argc, argv); });
  SetThreadName(serverThread, "Server");
  SetCurrentThreadName("Client");
  startgame(argc, argv);
  ShutdownServer();
  serverThread.join();
  GlobalSystems<Config::SERVER_MODULE>::Deinit();
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  return 0;
}
