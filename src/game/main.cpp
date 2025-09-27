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
#include "common/Logging.hpp"
#include "platform/include/Threads.hpp"
#include "server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/SoldatConfig.hpp"

// NOLINTBEGIN(unused-includes)
// clang-format off
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h> // IWYU: pragma keep

#define APPROVALS_DOCTEST_EXISTING_MAIN
#undef FMT_VERSION
#include <ApprovalTests/ApprovalTests.hpp> // IWYU: pragma keep
// clang-format on
// NOLINTEND(unused-includes)

namespace
{
	using std::exit;

	template <int DesiredPrecision>
	class FPngFuzzyComparator : public ApprovalTests::ApprovalComparator
	{
	public:
		bool contentsAreEquivalent(std::string InReceivedPath, std::string InApprovedPath) const override
		{
			constexpr auto kDesiredChannels = 4;
			int Rw = 0;
			int Rh = 0;
			int Rchannels = 0;
			auto* Received = stbi_load(InReceivedPath.c_str(), &Rw, &Rh, &Rchannels, kDesiredChannels);
			int Aw = 0;
			int Ah = 0;
			int Achannels = 0;
			auto* Approved = stbi_load(InApprovedPath.c_str(), &Aw, &Ah, &Achannels, kDesiredChannels);
			auto TestFunction = [&]()
			{
				if (!Received || !Approved)
				{
					return false;
				}
				if (Rw != Aw || Rh != Ah || Rchannels != Achannels)
				{
					return false;
				}
				for (int i = 0; i < Aw * Ah * Achannels; i++)
				{
					if ((static_cast<int>(Approved[i]) - static_cast<int>(Received[i])) > DesiredPrecision)
					{
						return false;
					}
				}
				return true;
			};
			auto Ret = TestFunction();
			stbi_image_free(Received);
			stbi_image_free(Approved);
			return Ret;
		}
	};

	void RunTests(int InArgc, char** InArgv)
	{
		auto DirectoryDisposer = ApprovalTests::Approvals::useApprovalsSubdirectory("approval_tests");
		auto DefaultReporterDisposer = ApprovalTests::Approvals::useAsDefaultReporter(
			std::make_shared<ApprovalTests::CrossPlatform::VisualStudioCodeReporter>());
		const auto RootPath = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
		ApprovalTests::TestName::registerRootDirectoryFromMainFile((RootPath / "CMakeLists.txt").string());

		auto DefaultNamerDisposer = ApprovalTests::Approvals::useAsDefaultNamer(
			[&RootPath]()
			{
				return ApprovalTests::TemplatedCustomNamer::create((
					RootPath
					/ "{ApprovalsSubdirectory}/{RelativeTestSourceDirectory}/{TestFileName}.{TestCaseName}.{ApprovedOrReceived}.{FileExtension}")
						.string());
			});

		ApprovalTests::EmptyFileCreatorByType::registerCreator(".png",
			[](const std::string& InPath)
			{
				constexpr auto kWidth = 1;
				constexpr auto kHeight = 1;
				constexpr auto kChannels = 4;
				std::array<std::uint8_t, static_cast<std::size_t>(kWidth * kHeight * kChannels)> Data{};
				std::ranges::fill(Data, 0x0);

				stbi_write_png(InPath.c_str(), kWidth, kHeight, kChannels, Data.data(), kWidth * kChannels);
			});

		auto Disposer = ApprovalTests::FileApprover::registerComparatorForExtension(
			".png", std::make_shared<FPngFuzzyComparator<2>>());

		doctest::Context Ctx;
		Ctx.applyCommandLine(InArgc, InArgv);

		int const RET = Ctx.run();

		if (Ctx.shouldExit())
		{
			exit(RET); // NOLINT(concurrency-mt-unsafe)
		}
		SoldatAssert(RET == 0);
	}

	struct FAppState
	{
		std::thread ServerThread;
	};

} // namespace

SDL_AppResult SDL_AppInit(void** InAppstate, int InArgc, char** InArgv)
{
	InitLogging();
	RunTests(InArgc, InArgv);
	FGlobalSystems<Config::CLIENT_MODULE>::Init();
	FGlobalSystems<Config::SERVER_MODULE>::Init();
	auto* State = new FAppState{ .ServerThread = std::thread(
									 [=]()
									 {
										 gGlobalStateServer.RunServer(InArgc, InArgv);
									 }) };
	SetThreadName(State->ServerThread, "SoldatServer");
	SetCurrentThreadName("SoldatClient");
	gGlobalStateClient.StartGame(InArgc, InArgv);
	*InAppstate = State;
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* /*appstate*/)
{
	auto ContinueRun = gGlobalStateClient.MainLoop();
	return ContinueRun ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppEvent(void* /*appstate*/, SDL_Event* InEvent)
{
	auto ContinueRun = gGlobalStateClient.ProcessSDLEvent(InEvent);
	return ContinueRun ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
}

void SDL_AppQuit(void* InAppState, SDL_AppResult /*result*/)
{
	gGlobalStateServer.ShutdownServer();
	auto* state = reinterpret_cast<FAppState*>(InAppState); // NOLINT
	state->ServerThread.join();
	delete state;
	FGlobalSystems<Config::SERVER_MODULE>::Deinit();
	FGlobalSystems<Config::CLIENT_MODULE>::Deinit();
}