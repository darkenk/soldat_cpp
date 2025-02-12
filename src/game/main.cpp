#include "client/Client.hpp"
#include "client/ControlGame.hpp"
#include "common/Logging.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <thread>

// clang-format off
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
// clang-format on

// clang-format off
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
// clang-format on

void RunTests(int argc, char **argv)
{
  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);

  int res = ctx.run();

  SoldatAssert(res == 0);
  if (ctx.shouldExit())
  {
    std::exit(res);
  }
}

struct AppState
{
  std::thread serverThread;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
  InitLogging();
  RunTests(argc, argv);
  GlobalSystems<Config::CLIENT_MODULE>::Init();
  GlobalSystems<Config::SERVER_MODULE>::Init();
  auto *state = new AppState{ .serverThread = std::thread([=]() { RunServer(argc, argv); }) }; 
  SetThreadName(state->serverThread, "Server");
  SetCurrentThreadName("Client");
  startgame(argc, argv);
  *appstate = state;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
  auto continue_run = mainloop();
  return continue_run ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
  gameinput(*event);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
  ShutdownServer();
  auto* state = reinterpret_cast<AppState*>(appstate);
  state->serverThread.join();
  delete state;
  GlobalSystems<Config::SERVER_MODULE>::Deinit();
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
}