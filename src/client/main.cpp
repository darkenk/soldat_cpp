#include "Client.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalVariable.hpp"

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::CLIENT_MODULE>();
    InitLogging();
    startgame(argc, argv);
    return 0;
}
