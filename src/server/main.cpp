#include "Server.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalSystems.hpp"

auto main(int argc, const char *argv[]) -> int
{
  InitLogging();
  GlobalSystems<Config::SERVER_MODULE>::Init();
  RunServer(argc, argv);
  GlobalSystems<Config::SERVER_MODULE>::Deinit();
  return 0;
}
