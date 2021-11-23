#include "Server.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalVariable.hpp"

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::SERVER_MODULE>();
    InitLogging();
    RunServer(argc, argv);
    return 0;
}
