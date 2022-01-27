#include "Server.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalSystems.hpp"

int main(int argc, const char *argv[])
{
    InitLogging();
    GlobalSystems<Config::SERVER_MODULE>::Init();
    RunServer(argc, argv);
    GlobalSystems<Config::SERVER_MODULE>::Deinit();
    return 0;
}
