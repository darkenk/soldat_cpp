#include "Client.hpp"
#include "shared/Logging.hpp"

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::CLIENT_MODULE>();
    InitLogging();
    startgame(argc, argv);
    return 0;
}
