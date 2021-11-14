#include "Client.hpp"
#include "shared/Logging.hpp"

int main(int argc, const char *argv[])
{
    InitLogging();
    startgame(argc, argv);
    return 0;
}
