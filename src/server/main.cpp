#include "Server.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/misc/GlobalVariable.hpp"

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::SERVER_MODULE>();
    InitLogging();
    GlobalSystems<Config::SERVER_MODULE>::Init();
    RunServer(argc, argv);
    GlobalSystems<Config::SERVER_MODULE>::Deinit();
    return 0;
}
