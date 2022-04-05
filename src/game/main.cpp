#include "client/Client.hpp"
#include "common/Logging.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <thread>

int main(int argc, const char *argv[])
{
  InitLogging();
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
  return 0;
}
