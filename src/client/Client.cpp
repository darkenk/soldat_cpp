
/*#include "idcompilerdefines.inc"*/
// #define DEVELOPMENT

#include "Client.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_messagebox.h>
#include <SDL3/SDL_video.h>
#include <Tracy.hpp>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <imgui.h>
#include <ios>
#include <memory>
#include <spdlog/fmt/bundled/core.h>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <libassert/assert.hpp>
#include <dbg.h>

#include "ClientCommands.hpp"
#include "ClientGame.hpp"
#include "ControlGame.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "Gfx.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "client/SdlApp.hpp"
#include "client/debug/DebugWindow.hpp"
#include "common/AnimationSystem.hpp"
#include "common/Console.hpp"
#include "common/FileUtility.hpp"
#include "common/GameStrings.hpp"
#include "common/LogFile.hpp"
#include "common/Logging.hpp"
#include "common/PolyMap.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SHA1Helper.hpp"
#include "common/misc/SafeType.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/misc/TFileStream.hpp"
#include "common/misc/TIniFile.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "shared/Command.hpp"
#include "shared/Constants.cpp.h"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sparks.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClient.hpp"
#include "shared/network/NetworkClientConnection.hpp"

auto FGlobalStateClient::InitBigConsole(
	const std::int32_t InNewMessageWait, const std::int32_t InCountMax, const std::int32_t InScrollTickMax) -> FConsole&
{
	sBigConsole = std::make_shared<FConsole>(InNewMessageWait, InCountMax, InScrollTickMax, true);
	return *sBigConsole;
}

auto FGlobalStateClient::InitKillConsole(
	const std::int32_t InNewMessageWait, const std::int32_t InCountMax, const std::int32_t InScrollTickMax)
	-> FConsoleMain&
{

	sKillConsole = std::make_shared<FConsoleMain>(InNewMessageWait, InCountMax, InScrollTickMax, true);
	return *sKillConsole;
}

auto FGlobalStateClient::GetBigConsole() -> FConsole&
{
	return *sBigConsole;
}

auto FGlobalStateClient::GetKillConsole() -> FConsoleMain&
{
	return *sKillConsole;
}

// Client.cpp variables

FGlobalStateClient gGlobalStateClient{

};

void FGlobalStateClient::RestartGraph()
{
	gGlobalStateGameRendering.dotextureloading(true);

	auto& Map = GS::GetGame().GetMap();

	// Load Map
	Map.loadmap(GS::GetFileSystem(),
		GS::GetGame().GetMapchange(),
		CVar::r_forcebg,
		CVar::r_forcebg_color1,
		CVar::r_forcebg_color2);

	if (!gGlobalStateGameMenus.escmenu->active)
	{
		gGlobalStateClientGame.mx = gGlobalStateGame.gamewidthhalf;
		gGlobalStateClientGame.my = gGlobalStateGame.gameheighthalf;
		gGlobalStateClientGame.mouseprev.x = gGlobalStateClientGame.mx;
		gGlobalStateClientGame.mouseprev.y = gGlobalStateClientGame.my;
	}

	GS::GetMainConsole().Console(("Graphics restart"), debug_message_color);
}

void FGlobalStateClient::LoadWeaponNames(
	FFileUtility& InFileUtility, TGunArray& InGunDisplayName, const std::string_view InModDir)
{
	SoldatAssert(InGunDisplayName.size() == double_weapons);
	std::int32_t i = 0;

	const std::string WeaponNamesFile = std::format("{}txt/weaponnames.txt", InModDir);

	// GS::GetMainConsole().console(std::string("Loading Weapon Names from ") + weaponNamesFile, debug_message_color);
	NotImplemented("console");
	if (!InFileUtility.Exists((WeaponNamesFile)))
	{
		return;
	}
	std::vector<std::byte> Buff;
	std::size_t FileSize = 0;
	{
		auto* f = InFileUtility.Open(WeaponNamesFile, FFileUtility::EFileMode::Read);
		FileSize = FFileUtility::Size(f);
		Buff.resize(FileSize);
		FFileUtility::Read(f, Buff.data(), FileSize);
		FFileUtility::Close(f);
	}
	std::istringstream sd(std::string(reinterpret_cast<char*>(Buff.data()), FileSize));
	for (i = 0; i < double_weapons; i++)
	{
		SoldatAssert(sd.good());
		std::getline(sd, InGunDisplayName[weaponnumexternaltointernal(i)]);
	}
}

void FGlobalStateClient::RedirectDialog()
{
	std::array<SDL_MessageBoxButtonData, 2> Buttons{};
	SDL_MessageBoxData Data;
	std::int32_t Response = 0;

	gGlobalStateGameRendering.rendergameinfo("Server Redirect");
	Buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
	Buttons[0].buttonID = 0;
	Buttons[0].text = "Yes";
	Buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
	Buttons[1].buttonID = 1;
	Buttons[1].text = "No";

	Data.flags = 0;
	Data.window = gGlobalStateInput.gamewindow;
	Data.title = "Server Redirect";
	auto Msg = (gGlobalStateClient.redirectmsg + "\r\n\r\nRedirect to server " + gGlobalStateClient.redirectip + ":"
				+ inttostr(gGlobalStateClient.redirectport) + "?");
	Data.message = Msg.c_str();
	Data.numbuttons = 2;
	Data.buttons = &Buttons.at(0);
	Data.colorScheme = nullptr;

	if (static_cast<int>(SDL_ShowMessageBox(&Data, &Response)) != 0)
	{
		return;
	}

	gGlobalStateClient.redirecttoserver = false;

	if (Response == 0)
	{
		gGlobalStateClient.joinip = gGlobalStateClient.redirectip;
		gGlobalStateClient.joinport = inttostr(gGlobalStateClient.redirectport);
		JoinServer();
	}
	else
	{
		gGlobalStateClient.redirectip = "";
		gGlobalStateClient.redirectport = 0;
		gGlobalStateClient.redirectmsg = "";
		ExitToMenu();
	}
}

void FGlobalStateClient::ExitToMenu()
{
	auto& SpriteSystem = SpriteSystem::Get();
	std::int32_t i = 0;

	GS::GetGame().ResetGoalTicks();

	// Reset network state and show the status std::string (if any)
	// ShouldRenderFrames := False;
	// NetEncActive := False;

	// resetsynccvars;

	if (GS::GetDemoRecorder().active())
	{
		GS::GetDemoRecorder().stoprecord();
	}

	if (gGlobalStateDemo.demoplayer.active())
	{
		gGlobalStateDemo.demoplayer.stopdemo();
	}

	if (SpriteSystem.IsPlayerSpriteValid())
	{
		clientdisconnect(*gGlobalStateNetworkClient.GetNetwork());
	}
	if (gGlobalStateNetworkClient.GetNetwork() != nullptr)
	{
		gGlobalStateNetworkClient.GetNetwork()->Disconnect(true);
	}

	gGlobalStateSound.stopsound(channel_weather);

	auto& Map = GS::GetGame().GetMap();

	Map.name = "";

	if (gGlobalStateGameMenus.escmenu != nullptr)
	{
		gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.escmenu, false);
	}

	Map.filename = ""; // force reloading next time
	GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
	// WindowReady := False;

	auto& ActiveSprites = SpriteSystem.GetActiveSprites();

	std::ranges::for_each(ActiveSprites,
		[](auto& sprite)
		{
			sprite.kill();
		});
	GS::GetBulletSystem().KillAll();
	for (i = 1; i <= max_sparks; i++)
	{
		gGlobalStateGame.spark[i].kill();
	}
	GS::GetThingSystem().KillAll();

	// Reset World and Big Texts

	// Big Text
	std::ranges::fill(gGlobalStateInterfaceGraphics.bigtext, "");
	std::ranges::fill(gGlobalStateInterfaceGraphics.bigdelay, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.bigscale, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.bigcolor, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.bigposx, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.bigposy, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.bigx, 0);
	// World Text
	std::ranges::fill(gGlobalStateInterfaceGraphics.worldtext, "");
	std::ranges::fill(gGlobalStateInterfaceGraphics.worlddelay, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.worldscale, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.worldcolor, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.worldposx, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.worldposy, 0);
	std::ranges::fill(gGlobalStateInterfaceGraphics.worldx, 0);

	// Reset ABOVE CHAT MESSAGE
	for (i = 1; i < max_sprites; i++)
	{
		gGlobalStateInterfaceGraphics.chatdelay[i] = 0;
		gGlobalStateInterfaceGraphics.chatmessage[i] = "";
		gGlobalStateInterfaceGraphics.chatteam[i] = false;
	}

	mysprite = 0;
	camerafollowsprite = 0;
	gamethingtarget = 0;

	if (redirecttoserver)
	{
		RedirectDialog();
	}
}

void FGlobalStateClient::CreateDirectoryStructure(FFileUtility& fs)
{
	SoldatEnsure(fs.MkDir("/user/configs"));
	SoldatEnsure(fs.MkDir("/user/screens"));
	SoldatEnsure(fs.MkDir("/user/demos"));
	SoldatEnsure(fs.MkDir("/user/logs"));
	SoldatEnsure(fs.MkDir("/user/logs/kills"));
	SoldatEnsure(fs.MkDir("/user/maps"));
	SoldatEnsure(fs.MkDir("/user/mods"));
}

auto FGlobalStateClient::MountAssets(FFileUtility& InFileUtility,
	const std::string_view InUserDirectory,
	const std::string_view InBaseDirectory,
	tsha1digest& InOutGameModChecksum,
	tsha1digest& InOutCustomModChecksum) -> bool
{
	LogDebugG("[FS] Mounting game archive");
	if (CVar::fs_localmount)
	{
		if (!InFileUtility.Mount(InUserDirectory, "/"))
		{
			ShowMessage(("Could not load base game archive (game directory)."));
			return false;
		}
	}
	else
	{
		const std::string AssetPackage = std::format("{}/soldat.smod", InBaseDirectory);
		if (!InFileUtility.Mount(AssetPackage, "/"))
		{
			ShowMessage(("Could not load base game archive (soldat.smod). Try to reinstall the game."));
			return false;
		}

		InOutGameModChecksum = sha1file(AssetPackage);
	}
	gGlobalStateClient.moddir = "";
	if (CVar::fs_mod != "")
	{
		const auto ModPath = std::format("{}mods/{}.smod", InUserDirectory, lowercase(CVar::fs_mod));
		LogDebugG("[FS] Mounting {}", ModPath);

		if (!InFileUtility.Mount(ModPath, (std::string("mods/") + lowercase(CVar::fs_mod) + "/")))
		{
			ShowMessage((std::string("Could not load mod archive (") + std::string(CVar::fs_mod) + ")."));
			return false;
		}
		gGlobalStateClient.moddir = std::string("/mods/") + lowercase(CVar::fs_mod) + '/';
		InOutCustomModChecksum = sha1file(ModPath);
	}
	return true;
}

// TODO(vscode): throw away test variable
void FGlobalStateClient::InitConsoles(bool InTest)
{
	auto CountMax = floor((0.85 * gGlobalStateClientGame.renderheight)
						  / (CVar::font_consolelineheight * gGlobalStateGameRendering.fontstylesize(font_small)));
	if (InTest)
	{
		CountMax = 20;
	}

	InitBigConsole(0, CountMax, 1500000);
	GS::GetMainConsole().SetBigConsole(&GetBigConsole());

	InitKillConsole(70, round(CVar::ui_killconsole_length * gGlobalStateInterfaceGraphics._rscala.y), 240);
}

void FGlobalStateClient::StartGame(int argc, char* argv[])
{
	initclientcommands();
	commandinit();

	parsecommandline(argc, argv);
	if (argc == 1)
	{
		parseinput("join 127.0.0.1 23073");
	}

	auto& fs = GS::GetFileSystem();
	const auto UserDirectory = FFileUtility::GetPrefPath("client");
	const auto BaseDirectory = FFileUtility::GetBasePath();

	LogDebugG("[FS] userDirectory: {}", UserDirectory);
	LogDebugG("[FS] baseDirectory: {}", BaseDirectory);

	fs.Mount(UserDirectory, "/user");

	// Create the basic folder structure
	CreateDirectoryStructure(fs);

	{
		tsha1digest GameSha1;
		tsha1digest ModSha1;

		if (!MountAssets(fs, UserDirectory, BaseDirectory, GameSha1, ModSha1))
		{
			SoldatAssert(false);
			return;
		}
		GS::GetGame().SetCustomModChecksum(ModSha1);
		GS::GetGame().SetGameModChecksum(GameSha1);
	}

	GS::GetConsoleLogFile().Enable(CVar::log_enable);
	GS::GetConsoleLogFile().SetLogLevel(CVar::log_level);
	GS::GetConsoleLogFile().Init("/user/logs/consolelog");

	std::string SystemLang = "en_US";
	// todo this variable is needed when code is refactored
	const std::string SystemFallbackLang = "en_US"; // NOLINT

	// TODO(vscode): remove HWIDs, replace by Fae auth tickets
	hwid = "00000000000";

	LogDebugG("[FS] Initializing system");

	gGlobalStateInterfaceGraphics.loadinterfacearchives(UserDirectory + "/custom-interfaces/");

	fs.Copy("/configs/bindings.cfg", "/user/configs/bindings.cfg");
	fs.Copy("/configs/client.cfg", "/user/configs/client.cfg");
	fs.Copy("/configs/controls.cfg", "/user/configs/controls.cfg");
	fs.Copy("/configs/game.cfg", "/user/configs/game.cfg");
	fs.Copy("/configs/graphics.cfg", "/user/configs/graphics.cfg");
	fs.Copy("/configs/player.cfg", "/user/configs/player.cfg");
	fs.Copy("/configs/sound.cfg", "/user/configs/sound.cfg");

	loadconfig("client.cfg", fs);

	// these might change so keep a backup to avoid changing the settings file
	gGlobalStateClientGame.screenwidth = CVar::r_screenwidth;
	gGlobalStateClientGame.screenheight = CVar::r_screenheight;
	gGlobalStateClientGame.renderheight = CVar::r_renderheight;
	gGlobalStateClientGame.renderwidth = CVar::r_renderwidth;

	ASSERT((gGlobalStateClientGame.screenwidth != 0) && (gGlobalStateClientGame.screenheight != 0),
		"CVars are probably null");
	ASSERT((gGlobalStateClientGame.renderwidth != 0) && (gGlobalStateClientGame.renderheight != 0),
		"CVars are probably null");

	// Calculcate FOV to check for too high/low vision
	float Fov = static_cast<float>(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight;
	if (Fov > max_fov)
	{
		gGlobalStateClientGame.renderwidth = std::ceil(gGlobalStateClientGame.renderheight * max_fov);
		Fov = max_fov;
	}
	else if (Fov < min_fov)
	{
		gGlobalStateClientGame.renderheight =
			std::ceil(static_cast<float>(gGlobalStateClientGame.renderwidth) / min_fov);
		Fov = min_fov;
	}

	// Calulcate internal game width based on the fov and internal height
	gGlobalStateGame.gamewidth = std::round(Fov * gGlobalStateGame.gameheight);
	gGlobalStateGame.gamewidthhalf = static_cast<float>(gGlobalStateGame.gamewidth) / 2;
	gGlobalStateGame.gameheighthalf = static_cast<float>(gGlobalStateGame.gameheight) / 2;

	if (CVar::r_fullscreen == 0)
	{
		// avoid black bars in windowed mode
		if ((static_cast<float>(gGlobalStateClientGame.screenwidth) / gGlobalStateClientGame.screenheight)
			>= (static_cast<float>(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight))
		{
			gGlobalStateClientGame.screenwidth = std::round(
				gGlobalStateClientGame.screenheight
				* (static_cast<float>(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight));
		}
		else
		{
			gGlobalStateClientGame.screenheight = std::round(
				gGlobalStateClientGame.screenwidth
				* (static_cast<float>(gGlobalStateClientGame.renderheight) / gGlobalStateClientGame.renderwidth));
		}
	}

	// window size equals "screen" size except in windowed fullscreen
	gGlobalStateClientGame.windowwidth = gGlobalStateClientGame.screenwidth;
	gGlobalStateClientGame.windowheight = gGlobalStateClientGame.screenheight;

	LogInfo("gfx", "Window size: {}x{}", gGlobalStateClientGame.windowwidth, gGlobalStateClientGame.windowheight);
	LogInfo("gfx", "Target resolution: {}x{}", gGlobalStateClientGame.screenwidth, gGlobalStateClientGame.screenheight);
	LogInfo(
		"gfx", "Internal resolution: {}x{}", gGlobalStateClientGame.renderwidth, gGlobalStateClientGame.renderheight);

	// even windowed mode can behave as fullscreen with the right size
	// IsFullscreen := (WindowWidth = Screen.Width) and (WindowHeight = Screen.Height);

	// interface is hard-coded to work on 4:3 aspect ratio,
	// but luckily for us the interface rendering code
	// translates the points using _RScala scale factor
	// above, so all we floatly need to do to make interace
	// work for widescreens is translate those points to a wider
	// area, which we can do by using the 640/480 as scale factors
	// even in widescreen resolutions. The interface code does NOT
	// use the _RScala to scale the interface, so this won't make
	// it look distorted.
	if (CVar::r_scaleinterface)
	{
		gGlobalStateInterfaceGraphics._rscala.x = 1;
		gGlobalStateInterfaceGraphics._rscala.y = 1;

		gGlobalStateInterfaceGraphics._iscala.x = static_cast<float>(gGlobalStateGame.gamewidth) / default_width;
		gGlobalStateInterfaceGraphics._iscala.y = 1;

		gGlobalStateInterfaceGraphics.fragx = std::floor(gGlobalStateGame.gamewidthhalf - 300) - 25;
	}
	else
	{
		gGlobalStateInterfaceGraphics._rscala.x =
			static_cast<float>(gGlobalStateClientGame.renderwidth) / gGlobalStateGame.gamewidth;
		gGlobalStateInterfaceGraphics._rscala.y =
			static_cast<float>(gGlobalStateClientGame.renderheight) / gGlobalStateGame.gameheight;

		gGlobalStateInterfaceGraphics._iscala.x = static_cast<float>(gGlobalStateClientGame.renderwidth) / 640;
		gGlobalStateInterfaceGraphics._iscala.y = static_cast<float>(gGlobalStateClientGame.renderheight) / 480;

		gGlobalStateInterfaceGraphics.fragx =
			std::floor((static_cast<float>(gGlobalStateClientGame.renderwidth) / 2) - 300) - 25;

		if (gGlobalStateClientGame.renderheight > gGlobalStateGame.gameheight)
		{
			gGlobalStateInterfaceGraphics.fragy = round(10 * gGlobalStateInterfaceGraphics._rscala.y);
		}
	}

	gGlobalStateGameRendering.gamerenderingparams.interfacename = CVar::ui_style;

	gGlobalStateClientGame.resetframetiming();

	gfxlog("Loading game graphics");

	{
		if (CVar::r_fullscreen == 2)
		{
			NotImplemented("sdl2_port");
			// windowflags = windowflags | SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		else if (CVar::r_fullscreen == 1)
		{
			NotImplemented("fullscreen window");
			// WindowFlags = WindowFlags | SDL_WINDOW_FULLSCREEN;
		}

		App = std::make_shared<FSdlApp>(
			"Soldat", gGlobalStateClientGame.windowwidth, gGlobalStateClientGame.windowheight);
		gGlobalStateInput.gamewindow = App->GetWindow();
		extern void gfxSetGpuDevice(SDL_GPUDevice * device); // NOLINT(readability-*)
		gfxSetGpuDevice(App->GetDevice());
		DebugWindow = std::make_shared<FDebugWindow>(*App);
		gGlobalStateGameRendering.SetDebugWindow(DebugWindow);
		App->RegisterEventHandler(SDL_EVENT_QUIT,
			[this](SDL_Event&)
			{
				clientdisconnect(*gGlobalStateNetworkClient.GetNetwork());
				Shutdown();
				return true;
			});
		gGlobalStateControlGame.RegisterEventHandlers(App);
	}

	if (!gGlobalStateGameRendering.initgamegraphics())
	{
		ShowMessage(std::string("The required OpenGL functionality isn't supported. ")
					+ "Please, update your video drivers and try again.");
		// ExitButtonClick(nullptr);
		PANIC("Cannot initialize graphic subsystem");
	}

	if (CVar::cl_lang != "")
	{
		SystemLang = CVar::cl_lang;
	}
	else
	{
		NotImplemented("localization");
#if 0
        getlanguageids(systemlang, systemfallbacklang);
#endif
	}

	if (inittranslation(ReadAsFileStream(fs, moddir + "/txt/" + SystemLang + ".mo").get()))
	{
		LogDebugG("Game captions loaded from {}/txt/{}", moddir, SystemLang);
	}
	else
	{
		LogDebugG("Game captions not found");
	}

	GS::GetConsoleLogFile().Log("Initializing Sound Library.");
	// Init Sound Library
	if (!gGlobalStateSound.initsound())
	{
		GS::GetConsoleLogFile().Log("Failed to initialize Sound Library.");
		// Let the player know that he has no sound (no popup window)
	}

	gGlobalStateSound.loadsounds("");
	if (length(moddir) > 0)
	{
		gGlobalStateSound.loadsounds(moddir);
	}

	GS::GetConsoleLogFile().Log("Creating network interface.");

	InitConsoles();

	GS::GetAnimationSystem().LoadAnimObjects("");
	if (length(moddir) > 0)
	{
		GS::GetAnimationSystem().LoadAnimObjects(moddir);
	}
	SpriteSystem::Get().ResetSpriteParts();
	GetBulletParts().destroy();
	GetBulletParts().timestep = 1;
	GetBulletParts().gravity = grav * 2.25;
	GetBulletParts().edamping = 0.99;

	gGlobalStateSparks.GetSparkParts().destroy();
	gGlobalStateSparks.GetSparkParts().timestep = 1;
	gGlobalStateSparks.GetSparkParts().gravity = grav / 1.4;
	gGlobalStateSparks.GetSparkParts().edamping = 0.998;

	// greet!
	// GS::GetMainConsole().console(("Welcome to Soldat ") + soldat_version, default_message_color);
	GS::GetMainConsole().Console(("Welcome to Soldat "), default_message_color);

	// Load weapon display names
	LoadWeaponNames(fs, gundisplayname, moddir);
	createweaponsbase(GS::GetWeaponSystem().GetGuns());

	GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);

	gGlobalStateInterfaceGraphics.playernamesshow = true;

	gGlobalStateInterfaceGraphics.cursortext = "";

	gGlobalStateGameMenus.initgamemenus();

	{
		TIniFile Ini{ ReadAsFileStream(fs, "txt/radiomenu-default.ini") };
		Ini.ReadSectionValues("OPTIONS", radiomenu);
	}

	// Play demo
	freecam = 1;
	notexts = 0;
	shotdistanceshow = -1;

	if (CVar::r_compatibility)
	{
		CVar::cl_actionsnap = false;
	}

	GS::GetConsoleLogFile().WriteToFile();

	gGlobalStateClientGame.resetframetiming();
	gGlobalStateGameRendering.initgamegraphics();
	gGlobalStateGameRendering.dotextureloading(true);

	gGlobalStateNetworkClient.InitClientNetwork();
	gGlobalStateNetworkClient.GetNetwork()->SetDisconnectionCallback(
		[](const char* msg)
		{
			gGlobalStateGameRendering.rendergameinfo(std::string("Network  error ") + msg);
		});
	gGlobalStateNetworkClient.GetNetwork()->SetConnectionCallback(
		[this](NetworkClientImpl& nc)
		{
			clientrequestgame(nc, joinpassword);
		});

	gamelooprun = true;
	rundeferredcommands();
}

void FGlobalStateClient::Shutdown()
{
	ExitToMenu();

	GS::GetConsoleLogFile().Log("Freeing sprites.");

	// Free GFX
	gGlobalStateGameRendering.destroygamegraphics();

	deinittranslation();

	GS::GetConsoleLogFile().Log("UDP closing.");

	gGlobalStateNetworkClient.DeinitClientNetwork();

	GS::GetConsoleLogFile().Log("Sound closing.");

	gGlobalStateSound.closesound();
	DebugWindow.reset();
	App.reset();

	GS::GetConsoleLogFile().Log("FS closing.");

	commanddeinit();

	GS::GetConsoleLogFile().Log("   End of Log.");

	GS::GetConsoleLogFile().WriteToFile();

	gamelooprun = false;
}

bool FGlobalStateClient::MainLoop()
{
	if (!gamelooprun)
	{
		return gamelooprun;
	}
	auto Begin = std::chrono::system_clock::now();
	gGlobalStateNetworkClient.GetNetwork()->ProcessLoop();
	DebugWindow->Draw(
		[]()
		{
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("Debug"))
				{
					if (ImGui::MenuItem("File"))
					{
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
		});
	switch (gGameState)
	{
		case EGameState::Loading:
			gGlobalStateGameRendering.rendergameinfo(("Loading"));
			break;
		case EGameState::Game:
			if (progready)
			{
				gGlobalStateClientGame.gameloop();
			}
			break;
		case EGameState::ConnectionTimedOut:
			gGlobalStateGameRendering.rendergameinfo(("Connection timed out."));
			break;
	}
	auto End = std::chrono::system_clock::now();
	constexpr auto kFrameTime = std::chrono::seconds(1) / 60.F;
	{
		ZoneScopedN("WaitingForNextFrame");
		std::this_thread::sleep_for(kFrameTime - (End - Begin));
	}
	FrameMarkNamed("ClientFrame");
	return gamelooprun;
}

#if __EMSCRIPTEN__
	#include <emscripten.h>
#endif

void FGlobalStateClient::StartGameLoop()
{
#if __EMSCRIPTEN__
	emscripten_set_main_loop(loop, 30, 1);
#else
	while (gamelooprun)
	{
		MainLoop();
	}
#endif
}

void FGlobalStateClient::JoinServer()
{
	gGlobalStateClientGame.resetframetiming();

	gClientServerIP = trim(joinip);

	NotImplemented("No error checking");
#if 0
    if (!trystrtoint(trim(joinport), serverport))
        return;
#endif

	// DEMO
	if (joinport == "0")
	{
		gGlobalStateDemo.demoplayer.opendemo(GS::GetGame().GetUserDirectory() + "demos/" + joinip + ".sdm");
		tdemoplayer::processdemo();
		progready = true;
		gamelooprun = true;
		gGlobalStateGameRendering.rendergameinfo(("Loading"));
		StartGameLoop();
	}
	else
	{
		gGlobalStateGameRendering.rendergameinfo(
			("Connecting to " + gClientServerIP + ":" + std::to_string(gClientServerPort)));

		if (gGlobalStateNetworkClient.GetNetwork()->Connect(gClientServerIP, gClientServerPort))
		{
			progready = true;
			gamelooprun = true;
			gGlobalStateGameRendering.rendergameinfo(("Loading"));
			clientrequestgame(*gGlobalStateNetworkClient.GetNetwork(), joinpassword);
			gGameState = EGameState::Game;
		}
		else
		{
			GS::GetMainConsole().Console(
				"[NET] Failed to connect to  server" + gGlobalStateNetworkClient.GetNetwork()->GetStringAddress(true),
				warning_message_color);
			return;
		}
	}
}

void FGlobalStateClient::ShowMessage(const std::string& InMessage)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", InMessage.c_str(), nullptr);
};

bool FGlobalStateClient::ProcessSDLEvent(SDL_Event* InEvent)
{
	App->ProcessEvent(InEvent);
	return true;
}

// tests
#include <doctest/doctest.h>
#include <fstream>
#include <filesystem>

namespace
{
	class FClientFixture
	{
	public:
		FClientFixture() = default;
		~FClientFixture() = default;
		FClientFixture(const FClientFixture&) = delete;
		FClientFixture(FClientFixture&&) = delete;
		FClientFixture& operator=(const FClientFixture&) = delete;
		FClientFixture& operator=(FClientFixture&&) = delete;

	protected:
		void T()
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/user");
			FGlobalStateClient Gsc;
			Gsc.CreateDirectoryStructure(fu);
		}
	};

	TEST_SUITE("Client")
	{

		TEST_CASE_FIXTURE(FClientFixture, "Mount memory and write file and later read it")
		{
			FFileUtility fu;
			fu.Mount("tmpfs.memory", "/user");
			FGlobalStateClient Gsc;
			Gsc.CreateDirectoryStructure(fu);
			auto* f = fu.Open("/user/logs/nice_log.txt", FFileUtility::EFileMode::Write);
			CHECK_NE(nullptr, f);
			FFileUtility::Close(f);
		}

		TEST_CASE_FIXTURE(FClientFixture, "Mount soldat.smod test")
		{
			// test soldat.smod, generated with xxd --include soldat.smod
			// contains:
			// client_test.txt
			// server_test.txt
			// shared_test.txt
			// NOLINTBEGIN
			unsigned char soldat_smod[] = { 0x50,
				0x4b,
				0x03,
				0x04,
				0x14,
				0x00,
				0x00,
				0x00,
				0x08,
				0x00,
				0x00,
				0x00,
				0x21,
				0x00,
				0x24,
				0x33,
				0x50,
				0xf5,
				0x0e,
				0x00,
				0x00,
				0x00,
				0x0c,
				0x00,
				0x00,
				0x00,
				0x0f,
				0x00,
				0x00,
				0x00,
				0x63,
				0x6c,
				0x69,
				0x65,
				0x6e,
				0x74,
				0x5f,
				0x74,
				0x65,
				0x73,
				0x74,
				0x2e,
				0x74,
				0x78,
				0x74,
				0x2b,
				0x49,
				0x2d,
				0x2e,
				0x89,
				0x4f,
				0xce,
				0xc9,
				0x4c,
				0xcd,
				0x2b,
				0xe1,
				0x02,
				0x00,
				0x50,
				0x4b,
				0x03,
				0x04,
				0x14,
				0x00,
				0x00,
				0x00,
				0x08,
				0x00,
				0x00,
				0x00,
				0x21,
				0x00,
				0xa7,
				0xe8,
				0x12,
				0xba,
				0x0e,
				0x00,
				0x00,
				0x00,
				0x0c,
				0x00,
				0x00,
				0x00,
				0x0f,
				0x00,
				0x00,
				0x00,
				0x73,
				0x65,
				0x72,
				0x76,
				0x65,
				0x72,
				0x5f,
				0x74,
				0x65,
				0x73,
				0x74,
				0x2e,
				0x74,
				0x78,
				0x74,
				0x2b,
				0x49,
				0x2d,
				0x2e,
				0x89,
				0x2f,
				0x4e,
				0x2d,
				0x2a,
				0x4b,
				0x2d,
				0xe2,
				0x02,
				0x00,
				0x50,
				0x4b,
				0x03,
				0x04,
				0x14,
				0x00,
				0x00,
				0x00,
				0x08,
				0x00,
				0x00,
				0x00,
				0x21,
				0x00,
				0xab,
				0x34,
				0x36,
				0xb2,
				0x0e,
				0x00,
				0x00,
				0x00,
				0x0c,
				0x00,
				0x00,
				0x00,
				0x0f,
				0x00,
				0x00,
				0x00,
				0x73,
				0x68,
				0x61,
				0x72,
				0x65,
				0x64,
				0x5f,
				0x74,
				0x65,
				0x73,
				0x74,
				0x2e,
				0x74,
				0x78,
				0x74,
				0x2b,
				0x49,
				0x2d,
				0x2e,
				0x89,
				0x2f,
				0xce,
				0x48,
				0x2c,
				0x4a,
				0x4d,
				0xe1,
				0x02,
				0x00,
				0x50,
				0x4b,
				0x01,
				0x02,
				0x14,
				0x0a,
				0x14,
				0x00,
				0x00,
				0x00,
				0x08,
				0x00,
				0x00,
				0x00,
				0x21,
				0x00,
				0x24,
				0x33,
				0x50,
				0xf5,
				0x0e,
				0x00,
				0x00,
				0x00,
				0x0c,
				0x00,
				0x00,
				0x00,
				0x0f,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x63,
				0x6c,
				0x69,
				0x65,
				0x6e,
				0x74,
				0x5f,
				0x74,
				0x65,
				0x73,
				0x74,
				0x2e,
				0x74,
				0x78,
				0x74,
				0x50,
				0x4b,
				0x01,
				0x02,
				0x14,
				0x0a,
				0x14,
				0x00,
				0x00,
				0x00,
				0x08,
				0x00,
				0x00,
				0x00,
				0x21,
				0x00,
				0xa7,
				0xe8,
				0x12,
				0xba,
				0x0e,
				0x00,
				0x00,
				0x00,
				0x0c,
				0x00,
				0x00,
				0x00,
				0x0f,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x3b,
				0x00,
				0x00,
				0x00,
				0x73,
				0x65,
				0x72,
				0x76,
				0x65,
				0x72,
				0x5f,
				0x74,
				0x65,
				0x73,
				0x74,
				0x2e,
				0x74,
				0x78,
				0x74,
				0x50,
				0x4b,
				0x01,
				0x02,
				0x14,
				0x0a,
				0x14,
				0x00,
				0x00,
				0x00,
				0x08,
				0x00,
				0x00,
				0x00,
				0x21,
				0x00,
				0xab,
				0x34,
				0x36,
				0xb2,
				0x0e,
				0x00,
				0x00,
				0x00,
				0x0c,
				0x00,
				0x00,
				0x00,
				0x0f,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00,
				0x76,
				0x00,
				0x00,
				0x00,
				0x73,
				0x68,
				0x61,
				0x72,
				0x65,
				0x64,
				0x5f,
				0x74,
				0x65,
				0x73,
				0x74,
				0x2e,
				0x74,
				0x78,
				0x74,
				0x50,
				0x4b,
				0x05,
				0x06,
				0x00,
				0x00,
				0x00,
				0x00,
				0x03,
				0x00,
				0x03,
				0x00,
				0xb7,
				0x00,
				0x00,
				0x00,
				0xb1,
				0x00,
				0x00,
				0x00,
				0x00,
				0x00 };
			unsigned int soldat_smod_len = 382;
			// NOLINTEND

			FFileUtility fu;
			auto TestDir = FFileUtility::GetPrefPath("mount_test", true);
			std::filesystem::remove_all(TestDir);
			// recreate directory
			TestDir = FFileUtility::GetPrefPath("mount_test", true);
			{
				std::ofstream s(TestDir + "/soldat.smod", std::ios_base::binary | std::ios_base::trunc);
				s.write(reinterpret_cast<char*>(soldat_smod), soldat_smod_len);
			}
			tsha1digest CustomMod;
			tsha1digest Mod;
			FGlobalStateClient Gsc;
			CHECK_EQ(true, Gsc.MountAssets(fu, "", TestDir, Mod, CustomMod));
			// std::filesystem::remove_all(testDir);
		}

		TEST_CASE_FIXTURE(FClientFixture, "loadweaponnamesRefactorToUseVirtualFileSystem")
		{
			FFileUtility fs;
			TGunArray ga;
			const auto UserDirectory = FFileUtility::GetPrefPath("client");
			const auto BaseDirectory = FFileUtility::GetBasePath();
			tsha1digest Checksum1;
			tsha1digest Checksum2;
			FGlobalStateClient Gsc;
			auto Ret = Gsc.MountAssets(fs, UserDirectory, BaseDirectory, Checksum1, Checksum2);
			CHECK_EQ(true, Ret);
			Gsc.LoadWeaponNames(fs, ga, Gsc.moddir);
			CHECK_EQ("USSOCOM", ga[0]);
			CHECK_EQ("Desert Eagles", ga[1]);
			CHECK_EQ("HK MP5", ga[2]);
			CHECK_EQ("Ak-74", ga[3]);
			CHECK_EQ("Steyr AUG", ga[4]);
			CHECK_EQ("Spas-12", ga[5]);
			CHECK_EQ("Ruger 77", ga[6]);
			CHECK_EQ("M79", ga[7]);
			CHECK_EQ("Barrett M82A1", ga[8]);
			CHECK_EQ("FN Minimi", ga[9]);
			CHECK_EQ("XM214 Minigun", ga[10]);
			CHECK_EQ("Combat Knife", ga[11]);
			CHECK_EQ("Chainsaw", ga[12]);
			CHECK_EQ("M72 LAW", ga[13]);
			CHECK_EQ("Flamer", ga[14]);
			CHECK_EQ("Rambo Bow", ga[15]);
			CHECK_EQ("Flamed Arrows", ga[16]);
		}

		TEST_CASE_FIXTURE(FClientFixture, "Test console initialization")
		{
			FGlobalSystems<Config::CLIENT_MODULE>::Init();
			auto PrevY = gGlobalStateInterfaceGraphics._rscala.y;
			gGlobalStateInterfaceGraphics._rscala.y = 1;
			FGlobalStateClient Gsc;
			Gsc.InitConsoles(true);
			gGlobalStateInterfaceGraphics._rscala.y = PrevY;
			const auto& Console = GS::GetMainConsole();
			// CHECK_EQ(0, console.countmax);
			// CHECK_EQ(150, console.scrolltickmax);
			CHECK_EQ(150, Console.GetNewMessageWait());
			CHECK_EQ(0, Console.GetCount());

			const auto& Big = Gsc.GetBigConsole();
			// CHECK_EQ(0, big.countmax); todo countmax in tests
			// CHECK_EQ(1500000, big.scrolltickmax);
			CHECK_EQ(0, Big.GetNewMessageWait());
			CHECK_EQ(0, Big.GetCount());

			const auto& Kill = Gsc.GetKillConsole();
			// CHECK_EQ(0, kill.countmax);
			// CHECK_EQ(240, kill.scrolltickmax);
			CHECK_EQ(70, Kill.GetNewMessageWait());
			FGlobalSystems<Config::CLIENT_MODULE>::Deinit();
		}

		TEST_CASE_FIXTURE(FClientFixture, "Start and shutdown" * doctest::skip(false))
		{
			FGlobalSystems<Config::CLIENT_MODULE>::Init();
			std::string Game = { "SoldatGame" };
			std::array<char*, 1> Argv = { Game.data() };
			FGlobalStateClient Gsc;
			Gsc.StartGame(Argv.size(), Argv.data());
			Gsc.Shutdown();
			FGlobalSystems<Config::CLIENT_MODULE>::Deinit();
		}

	} // TEST_SUITE("Client")

} // namespace
