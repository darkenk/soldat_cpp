#include "client/Client.hpp"
#include "common/Logging.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalVariable.hpp"
#include <thread>

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::CLIENT_MODULE>();
    InitAllGlobalVariables<Config::SERVER_MODULE>();
    InitLogging();
    std::thread t1([=]() { RunServer(argc, argv); });
    SetThreadName(t1, "Server");
    SetCurrentThreadName("Client");
    startgame(argc, argv);
    ShutdownServer();
    t1.join();
    return 0;
}
