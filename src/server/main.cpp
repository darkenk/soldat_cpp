#include "Server.hpp"
#include "ServerHelper.hpp"
#include "ServerLoop.hpp"
#include <thread>

#include <shared/Logging.hpp>

void RunServer()
{
    ActivateServer();
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

int main(int argc, char *argv[])
{
    InitLogging();
    RunServer();
    return 0;
}
