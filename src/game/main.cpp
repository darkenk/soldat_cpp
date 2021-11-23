#include "client/Client.hpp"
#include "common/Logging.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalVariable.hpp"
#include <thread>

int main(int argc, const char *argv[])
{
    InitAllGlobalVariables<Config::CLIENT_MODULE>();
    InitAllGlobalVariables<Config::SERVER_MODULE>();
    InitLogging();
    std::thread t1([=]() { RunServer(argc, argv); });
    std::this_thread::sleep_for(std::chrono::seconds(3));
    startgame(argc, argv);
    return 0;
}
