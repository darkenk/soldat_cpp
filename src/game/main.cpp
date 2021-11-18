#include "client/Client.hpp"
#include "server/Server.hpp"
#include "shared/Logging.hpp"

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::CLIENT_MODULE>();
    InitAllGlobalVariables<Config::SERVER_MODULE>();
    InitLogging();
    startserver();
    startgame(argc, argv);
    return 0;
}
