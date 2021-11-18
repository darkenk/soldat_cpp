#include "Server.hpp"
#include "ServerHelper.hpp"
#include "ServerLoop.hpp"
#include <thread>

#include <shared/Logging.hpp>

void RunServer(int argc, const char *argv[])
{
    ActivateServer(argc, argv);
    writepid();

    if (progready)
    {
        startserver();
    }
    while (progready)
    {
        apponidle();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::SERVER_MODULE>();
    InitLogging();
    RunServer(argc, argv);
    return 0;
}
