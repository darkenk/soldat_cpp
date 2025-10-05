#pragma once

#include <SDL3/SDL_events.h>
#include <array>
#include <map>
#include <string>
#include <cstdint>

#include "client/SdlApp.hpp"
#include "client/debug/DebugWindow.hpp"
#include "common/Console.hpp"
#include "common/Vector.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/network/Net.hpp"

class FConsole;
class FFileUtility;
class FConsoleMain;
class FConsoleBig;
class FBigConsoleListener;

struct tweaponstat
{
	std::string name;
	std::uint32_t textureid;
	std::uint32_t shots, hits, kills, headshots;
	std::uint8_t accuracy;
};

using TGunArray = std::array<std::string, 17>;

struct FGlobalStateClient
{
	FGlobalStateClient() noexcept = default;
	FConsole& GetBigConsole();
	FConsoleMain& GetKillConsole();
	void JoinServer();
	void StartGame(int argc, char* argv[]);
	void Shutdown();
	void ExitToMenu();
	bool MainLoop();
	void LoadWeaponNames(FFileUtility& InFileUtility, TGunArray& InGunDisplayName, std::string_view InModDir);
	void ShowMessage(const std::string& InMessagetext);
	bool ProcessSDLEvent(SDL_Event* InEvent);
	std::string joinpassword;
	std::string joinport = "23073";
	std::string joinip = "127.0.0.1";
	std::string basedirectory;
	std::string moddir;
	bool usesservermod = {};
	std::string gClientServerIP = "127.0.0.1";
	std::int32_t gClientServerPort = 23073;
	std::uint8_t connection = INTERNET;
	std::uint8_t sniperline_client_hpp = 0;
	std::uint8_t trails = 1;
	std::uint8_t spectator = 0;
	std::uint8_t packetadjusting = 1;
	bool limbolock = {};
	std::uint8_t selteam = {};
	std::uint8_t mysprite = {};
	PascalArray<tweaponstat, 1, 20> wepstats = {};
	std::uint8_t wepstatsnum = 0;
	TGunArray gundisplayname = {};
	std::uint8_t gamethingtarget = {};
	std::int32_t grenadeeffecttimer = 0;
	std::uint8_t badmapidcount = {};
	std::string hwid = {};
	std::uint16_t hitspraycounter = {};
	bool screentaken = {};
	bool targetmode = false;
	bool muteall = false;
	bool redirecttoserver = false;
	std::string redirectip = {};
	std::int32_t redirectport = {};
	std::string redirectmsg = {};
	std::map<std::string, std::string> radiomenu = {};
	std::array<char, 2> rmenustate = {};
	bool showradiomenu = false;
	std::uint8_t radiocooldown = 3;
	tvector2 cameraprev{};
	float camerax = {};
	float cameray = {};
	std::uint8_t camerafollowsprite = {};
	std::uint8_t notexts = 0;
	std::uint8_t freecam = 0;
	std::int32_t shotdistanceshow = {};
	float shotdistance = {};
	float shotlife = {};
	std::int32_t shotricochet = {};

	// should be private, but there are tests written for those methods already
	auto MountAssets(FFileUtility& InFileUtility,
		std::string_view InUserDirectory,
		std::string_view InBaseDirectory,
		tsha1digest& InOutGameModChecksum,
		tsha1digest& InOutCustomModChecksum) -> bool;
	void CreateDirectoryStructure(FFileUtility& InFileUtility);
	void InitConsoles(bool InTest = false);

private:
	enum class EGameState : std::uint8_t
	{
		Loading,
		Game,
		ConnectionTimedOut
	};
	std::shared_ptr<FConsoleBig> BigConsole;
	std::shared_ptr<FConsoleMain> sKillConsole;
	EGameState gGameState{ EGameState::Loading };
	bool gamelooprun{};
	bool progready{};
	friend class ClientFixture;
	auto InitKillConsole(std::int32_t InNewMessageWait, std::int32_t InCountMax, std::int32_t InScrollTickMax)
		-> FConsoleMain&;
	void RedirectDialog();
	void RestartGraph();
	void StartGameLoop();
	std::shared_ptr<FSdlApp> App;
	std::shared_ptr<FDebugWindow> DebugWindow;
	std::shared_ptr<FBigConsoleListener> BigConsoleListener;
	std::shared_ptr<FLogFile> ConsoleLogFile;
};

extern FGlobalStateClient gGlobalStateClient;
