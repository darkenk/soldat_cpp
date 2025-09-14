#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stb_image.h>
#include <stb_image_write.h>
#include <string>
#include <thread>

#include "client/Client.hpp"
#include "client/ControlGame.hpp"
#include "common/Logging.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/SoldatConfig.hpp"

// NOLINTBEGIN
// clang-format off
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h> // IWYU: pragma keep

#define APPROVALS_DOCTEST_EXISTING_MAIN
#include <ApprovalTests/ApprovalTests.hpp> // IWYU: pragma keep
// clang-format on
// NOLINTEND

namespace
{

	template <int DesiredPrecision>
	class PngFuzzyComparator : public ApprovalTests::ApprovalComparator
	{
	public:
		bool contentsAreEquivalent(std::string receivedPath, std::string approvedPath) const override
		{
			constexpr auto kDesiredChannels = 4;
			int rw = 0;
			int rh = 0;
			int rchannels = 0;
			auto* received = stbi_load(receivedPath.c_str(), &rw, &rh, &rchannels, kDesiredChannels);
			int aw = 0;
			int ah = 0;
			int achannels = 0;
			auto* approved = stbi_load(approvedPath.c_str(), &aw, &ah, &achannels, kDesiredChannels);
			auto testFunction = [&]()
			{
				if (!received || !approved)
				{
					return false;
				}
				if (rw != aw || rh != ah || rchannels != achannels)
				{
					return false;
				}
				for (int i = 0; i < aw * ah * achannels; i++)
				{
					if ((static_cast<int>(approved[i]) - static_cast<int>(received[i])) > DesiredPrecision)
					{
						return false;
					}
				}
				return true;
			};
			auto ret = testFunction();
			stbi_image_free(received);
			stbi_image_free(approved);
			return ret;
		}
	};

	void RunTests(int argc, char** argv)
	{
		auto directoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("approval_tests");
		auto defaultReporterDisposer = ApprovalTests::Approvals::useAsDefaultReporter(
			std::make_shared<ApprovalTests::CrossPlatform::VisualStudioCodeReporter>());
		const auto rootPath = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
		ApprovalTests::TestName::registerRootDirectoryFromMainFile((rootPath / "CMakeLists.txt").string());

		auto defaultNamerDisposer = ApprovalTests::Approvals::useAsDefaultNamer(
			[&rootPath]()
			{
				return ApprovalTests::TemplatedCustomNamer::create((
					rootPath
					/ "{ApprovalsSubdirectory}/{RelativeTestSourceDirectory}/{TestFileName}.{TestCaseName}.{ApprovedOrReceived}.{FileExtension}")
						.string());
			});

		ApprovalTests::EmptyFileCreatorByType::registerCreator(".png",
			[](const std::string& path)
			{
				constexpr auto kWidth = 1;
				constexpr auto kHeight = 1;
				constexpr auto kChannels = 4;
				std::array<std::uint8_t, static_cast<std::size_t>(kWidth * kHeight * kChannels)> data{};
				std::ranges::fill(data, 0x0);

				stbi_write_png(path.c_str(), kWidth, kHeight, kChannels, data.data(), kWidth * kChannels);
			});

		auto disposer = ApprovalTests::FileApprover::registerComparatorForExtension(
			".png", std::make_shared<PngFuzzyComparator<2>>());

		doctest::Context ctx;
		ctx.applyCommandLine(argc, argv);

		int const res = ctx.run();

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

} // namespace

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	InitLogging();
	RunTests(argc, argv);
	GlobalSystems<Config::CLIENT_MODULE>::Init();
	GlobalSystems<Config::SERVER_MODULE>::Init();
	auto* state = new AppState{ .serverThread = std::thread(
									[=]()
									{
										gGlobalStateServer.RunServer(argc, argv);
									}) };
	SetThreadName(state->serverThread, "SoldatServer");
	SetCurrentThreadName("SoldatClient");
	gGlobalStateClient.startgame(argc, argv);
	*appstate = state;
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* /*appstate*/)
{
	auto continueRun = gGlobalStateClient.mainloop();
	return continueRun ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void* /*appstate*/, SDL_Event* event)
{
	gGlobalStateControlGame.gameinput(*event);
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/)
{
	gGlobalStateServer.ShutdownServer();
	auto* state = reinterpret_cast<AppState*>(appstate); // NOLINT
	state->serverThread.join();
	delete state;
	GlobalSystems<Config::SERVER_MODULE>::Deinit();
	GlobalSystems<Config::CLIENT_MODULE>::Deinit();
}