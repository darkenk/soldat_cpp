#include "Client.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalSystems.hpp"

int main(int argc, const char *argv[])
{
  InitLogging();
  GSC::Init();
  GlobalSystems<Config::CLIENT_MODULE>::Init();
  startgame(argc, argv);
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  GSC::Deinit();
  return 0;
}
