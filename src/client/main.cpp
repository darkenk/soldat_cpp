#include "Client.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalSystems.hpp"

int main(int argc, const char *argv[])
{
    InitLogging();
    GlobalSystems<Config::CLIENT_MODULE>::Init();
    startgame(argc, argv);
    GlobalSystems<Config::CLIENT_MODULE>::Deinit();
    return 0;
}
