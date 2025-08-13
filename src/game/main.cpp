#include <stb_image_write.h>
#include <stb_image.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <thread>
#include <filesystem>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <string>

#include "client/Client.hpp"
#include "client/ControlGame.hpp"
#include "common/Logging.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/SoldatConfig.hpp"

// clang-format off
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
// clang-format on

// clang-format off
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h> // IWYU: pragma keep

#define APPROVALS_DOCTEST_EXISTING_MAIN
#include <ApprovalTests/ApprovalTests.hpp> // IWYU: pragma keep
// clang-format on

template<int DesiredPrecision>
class PngFuzzyComparator : public ApprovalTests::ApprovalComparator
{
public:
  bool contentsAreEquivalent(std::string receivedPath, std::string approvedPath) const override
  {
    constexpr auto kDesiredChannels = 4;
    int rw, rh, rchannels;
    auto received = stbi_load(receivedPath.c_str(), &rw, &rh, &rchannels, kDesiredChannels);
    int aw, ah, achannels;
    auto approved = stbi_load(approvedPath.c_str(), &aw, &ah, &achannels, kDesiredChannels);
    auto test_function = [&]() {
      if (!received || !approved)
      {
        return false;
      }
      if (rw != aw || rh != ah || rchannels != achannels)
      {
        return false;
      }
      for (std::size_t i = 0; i < aw * ah * achannels; i++)
      {
        if (((int)approved[i] - (int)received[i]) > DesiredPrecision)
        {
          return false;
        }
      }
      return true;
    };
    auto ret = test_function();
    stbi_image_free(received);
    stbi_image_free(approved);
    return ret;
  }
};

void RunTests(int argc, char **argv)
{
  auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("approval_tests");
  auto defaultReporterDisposer = ApprovalTests::Approvals::useAsDefaultReporter(std::make_shared<ApprovalTests::CrossPlatform::VisualStudioCodeReporter>());
  const auto root_path = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  ApprovalTests::TestName::registerRootDirectoryFromMainFile(root_path / "CMakeLists.txt");

  auto default_namer_disposer = ApprovalTests::Approvals::useAsDefaultNamer(
    [&root_path]() { return ApprovalTests::TemplatedCustomNamer::create(
    root_path / "{ApprovalsSubdirectory}/{RelativeTestSourceDirectory}/{TestFileName}.{TestCaseName}.{ApprovedOrReceived}.{FileExtension}");
  });

  ApprovalTests::EmptyFileCreatorByType::registerCreator(".png",  [](std::string path) {
    constexpr auto kWidth = 1;
    constexpr auto kHeight = 1;
    constexpr auto kChannels = 4;
    std::array<std::uint8_t, kWidth * kHeight * kChannels> data;
    std::fill(std::begin(data), std::end(data), 0x0);

    stbi_write_png(path.c_str(), kWidth, kHeight, kChannels, data.data(), kWidth * kChannels);
  });

  auto disposer = ApprovalTests::FileApprover::registerComparatorForExtension(".png", std::make_shared<PngFuzzyComparator<2>>());

  doctest::Context ctx;
  ctx.applyCommandLine(argc, argv);

  int res = ctx.run();

  if (ctx.shouldExit())
  {
    std::exit(res);
  }
  SoldatAssert(res == 0);
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
  auto *state =
    new AppState{.serverThread = std::thread([=]() { gGlobalStateServer.RunServer(argc, argv); })};
  SetThreadName(state->serverThread, "Server");
  SetCurrentThreadName("Client");
  gGlobalStateClient.gClient.startgame(argc, argv);
  *appstate = state;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
  auto continue_run = gGlobalStateClient.gClient.mainloop();
  return continue_run ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
  gGlobalStateControlGame.gameinput(*event);
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
  gGlobalStateServer.ShutdownServer();
  auto* state = reinterpret_cast<AppState*>(appstate);
  state->serverThread.join();
  delete state;
  GlobalSystems<Config::SERVER_MODULE>::Deinit();
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
}