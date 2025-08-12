#include "Client.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalSystems.hpp"

auto main(int argc, char *argv[]) -> int
{
  InitLogging();
  GlobalSystems<Config::CLIENT_MODULE>::Init();
  gGlobalStateClient.gClient.startgame(argc, argv);
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  return 0;
}
