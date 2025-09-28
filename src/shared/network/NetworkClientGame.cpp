// automatically converted
#include "NetworkClientGame.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>

#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/InterfaceGraphics.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../mechanics/Sprites.hpp"
#include "NetworkClient.hpp"
#include "NetworkUtils.hpp"
#include "common/Constants.hpp"
#include "common/GameStrings.hpp"
#include "common/Parts.hpp"
#include "common/Vector.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/RandomGenerator.hpp"
#include "common/misc/SafeType.hpp"
#include "common/network/Net.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sparks.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/Things.hpp"
#include "shared/misc/GlobalSystems.hpp"

void clienthandlenewplayer::Handle(NetworkContext* netmessage)
{
	auto& sprite_system = SpriteSystem::Get();
	tmsg_newplayer* newplayermsg = nullptr;
	tvector2 a;
	std::int32_t i = 0;
	std::int32_t d = 0;

	if (!VerifyPacket(sizeof(tmsg_newplayer), netmessage->size, msgid_newplayer))
	{
		return;
	}

	newplayermsg = reinterpret_cast<pmsg_newplayer>(netmessage->packet);
	i = newplayermsg->num;
	if ((i < 1) || (i > max_sprites))
	{
		return;
	}

	auto player = sprite_system.GetSprite(i).player; // reuse object
	player->name = returnfixedplayername(newplayermsg->name.data());
	player->shirtcolor = newplayermsg->shirtcolor & 0xffffff;
	player->pantscolor = newplayermsg->pantscolor & 0xffffff;
	player->skincolor = newplayermsg->skincolor & 0xffffff;
	player->haircolor = newplayermsg->haircolor & 0xffffff;
	player->jetcolor = newplayermsg->jetcolor;
	player->team = newplayermsg->team;

	player->controlmethod = human;

	player->hairstyle = 0;
	if ((newplayermsg->look & B1) == B1)
	{
		player->hairstyle = 1;
	}
	if ((newplayermsg->look & B2) == B2)
	{
		player->hairstyle = 2;
	}
	if ((newplayermsg->look & B3) == B3)
	{
		player->hairstyle = 3;
	}
	if ((newplayermsg->look & B4) == B4)
	{
		player->hairstyle = 4;
	}

	player->headcap = 0;
	if ((newplayermsg->look & B5) == B5)
	{
		player->headcap = GFX::GOSTEK_HELM;
	}
	if ((newplayermsg->look & B6) == B6)
	{
		player->headcap = GFX::GOSTEK_KAP;
	}

	player->chain = 0;
	if ((newplayermsg->look & B7) == B7)
	{
		player->chain = 1;
	}
	if ((newplayermsg->look & B8) == B8)
	{
		player->chain = 2;
	}

	a = newplayermsg->pos;
	i = createsprite(a, i, player);
	d = 0;

	// The NewPlayer message doubles as confirmation that a player object was
	// allocated for the client. So far the following happened:
	// Client                        Server
	// Msg_RequestGame -->           (ServerHandleRequestGame) "Requesting game..." console message
	// ...                       <-- ... (ServerSyncCvars)
	// (ClientHandlePlayersList) <-- Msg_PlayersList (ServerSendPlayList)
	// Msg_PlayerInfo -->            (ServerHandlePlayerInfo) "...joining game..." console message
	//                               CreateSprite() -- note: also called when switching teams!
	// You are here!             <-- Msg_NewPlayer with AdoptSpriteID=1 (ServerSendNewPlayerInfo)
	if (newplayermsg->adoptspriteid == 1)
	{
		d = 1;
		gGlobalStateClient.mysprite = i;

		if (gGlobalStateDemo.demoplayer.active())
		{
			sprite_system.GetPlayerSprite().player->demoplayer = true;
		}

		// TODO(vscode): wat?
		sprite_system.GetPlayerSprite().bulletcount = Random(std::numeric_limits<std::uint16_t>::max());

		if (player->team == team_spectator)
		{
			gGlobalStateClient.camerafollowsprite = 0;
			gGlobalStateClient.camerafollowsprite = gGlobalStateClientGame.getcameratarget();
			gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.limbomenu, false);
		}
		else
		{
			gGlobalStateClient.camerafollowsprite = gGlobalStateClient.mysprite;
		}

		gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu, false);
		gGlobalStateNetworkClient.clientplayerreceived = true;
		gGlobalStateNetworkClient.clientplayerreceivedcounter = -1;
		gGlobalStateClient.badmapidcount = 2;
		gGlobalStateGame.heartbeattime = GS::GetGame().GetMainTickCounter();
		gGlobalStateGame.heartbeattimewarnings = 0;

		CVar::r_zoom = 0.0; // Reset zoom

		if (GS::GetGame().GetMapchangecounter() < 999999999)
		{
			GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
		}
		gGlobalStateInterfaceGraphics.fragsmenushow = false;
		gGlobalStateInterfaceGraphics.statsmenushow = false;
	}

	sprite_system.SetSpritePartsOldPos(i, newplayermsg->pos);

	auto& spritePartsPos = sprite_system.GetSpritePartsPos(i);
	spritePartsPos = newplayermsg->pos;

	sprite_system.GetSprite(i).respawn();

	auto& guns = GS::GetWeaponSystem().GetGuns();

	if (d == 1)
	{
		sprite_system.GetSprite(i).SetFirstWeapon(guns[noweapon]);
		sprite_system.GetSprite(i).SetSecondWeapon(guns[noweapon]);

		if (sprite_system.IsPlayerSpriteValid())
		{
			gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.limbomenu);
			newplayerweapon();
		}
	}

	if (newplayermsg->jointype != join_silent)
	{
		switch (newplayermsg->team)
		{
			case team_none:
				GS::GetMainConsole().Console(
					wideformat(_("{} has joined the game"), (player->name)), enter_message_color);
				break;
			case team_alpha:
				GS::GetMainConsole().Console(
					wideformat(_("{} has joined alpha team"), (player->name)), alphaj_message_color);
				break;
			case team_bravo:
				GS::GetMainConsole().Console(
					wideformat(_("{} has joined bravo team"), (player->name)), bravoj_message_color);
				break;
			case team_charlie:
				GS::GetMainConsole().Console(
					wideformat(_("{} has joined charlie team"), (player->name)), charliej_message_color);
				break;
			case team_delta:
				GS::GetMainConsole().Console(
					wideformat(_("{} has joined delta team"), (player->name)), deltaj_message_color);
				break;
			case team_spectator:
				GS::GetMainConsole().Console(
					wideformat(_("{} has joined as spectator"), (player->name)), deltaj_message_color);
				break;
		}
	}
}

void clientvotekick(std::uint8_t num, bool ban, std::string reason)
{
	tmsg_votekick votemsg{};

	votemsg.header.id = msgid_votekick;
	votemsg.ban = static_cast<std::uint8_t>(ban);
	votemsg.num = num;
	stringtoarray(votemsg.reason.data(), std::move(reason));
	gGlobalStateNetworkClient.GetNetwork()->SendData(&votemsg, sizeof(votemsg), true);
}

void clientvotemap(std::uint32_t mapid)
{
	tmsg_votemap votemsg{};

	votemsg.header.id = msgid_votemap;
	votemsg.mapid = mapid;
	gGlobalStateNetworkClient.GetNetwork()->SendData(&votemsg, sizeof(votemsg), true);
}

void clienthandlevoteresponse::Handle(NetworkContext* netmessage)
{
	tmsg_votemapreply* votemsgreply = nullptr;

	if (!VerifyPacket(sizeof(tmsg_votemapreply), netmessage->size, msgid_votemapreply))
	{
		return;
	}

	votemsgreply = reinterpret_cast<pmsg_votemapreply>(netmessage->packet);
	gGlobalStateNetworkClient.votemapname = votemsgreply->mapname.data();
	gGlobalStateNetworkClient.votemapcount = votemsgreply->count;
}

void clientfreecamtarget()
{
	tmsg_clientfreecam freecammsg;

	freecammsg.header.id = msgid_clientfreecam;
	freecammsg.freecamon = 0;
	freecammsg.targetpos.x = gGlobalStateClient.camerax;
	freecammsg.targetpos.y = gGlobalStateClient.cameray;

	gGlobalStateNetworkClient.GetNetwork()->SendData(&freecammsg, sizeof(freecammsg), true);
}

void clienthandleplayerdisconnect::Handle(NetworkContext* netmessage)
{
	auto& sprite_system = SpriteSystem::Get();
	tmsg_playerdisconnect* playermsg = nullptr;

	if (!VerifyPacket(sizeof(tmsg_playerdisconnect), netmessage->size, msgid_playerdisconnect))
	{
		return;
	}

	playermsg = reinterpret_cast<pmsg_playerdisconnect>(netmessage->packet);
	if ((playermsg->num < 1) || (playermsg->num > max_sprites))
	{
		return;
	}

	if ((playermsg->why == kick_unknown) || (playermsg->why >= _kick_end) || (playermsg->why == kick_leftgame))
	{
		switch (sprite_system.GetSprite(playermsg->num).player->team)
		{
			case 0:
				GS::GetMainConsole().Console(
					wideformat(_("{} has left the game"), (sprite_system.GetSprite(playermsg->num).player->name)),
					enter_message_color);
				break;
			case 1:
				GS::GetMainConsole().Console(
					wideformat(_("{} has left alpha team"), (sprite_system.GetSprite(playermsg->num).player->name)),
					alphaj_message_color);
				break;
			case 2:
				GS::GetMainConsole().Console(
					wideformat(_("{} has left bravo team"), (sprite_system.GetSprite(playermsg->num).player->name)),
					bravoj_message_color);
				break;
			case 3:
				GS::GetMainConsole().Console(
					wideformat(_("{} has left charlie team"), (sprite_system.GetSprite(playermsg->num).player->name)),
					charliej_message_color);
				break;
			case 4:
				GS::GetMainConsole().Console(
					wideformat(_("{} has left delta team"), (sprite_system.GetSprite(playermsg->num).player->name)),
					deltaj_message_color);
				break;
			case 5:
				GS::GetMainConsole().Console(
					wideformat(_("{} has left spectators"), (sprite_system.GetSprite(playermsg->num).player->name)),
					deltaj_message_color);
				break;
		}
	}

	switch (playermsg->why)
	{
		case kick_noresponse:
			GS::GetMainConsole().Console(
				wideformat(_("{} has disconnected"), (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_nocheatresponse:
			GS::GetMainConsole().Console(
				wideformat(_("{} has been disconnected"), (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_changeteam:
			GS::GetMainConsole().Console(
				wideformat(_("{} is changing teams"), (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_ping:
			GS::GetMainConsole().Console(wideformat(_("{} has been ping kicked (for 15 minutes)"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_flooding:
			GS::GetMainConsole().Console(wideformat(_("{} has been flood kicked (for 5 minutes)"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_console:
			GS::GetMainConsole().Console(wideformat(_("{} has been kicked from console"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_connectcheat:
			GS::GetMainConsole().Console(wideformat(_("{} has been 'connect cheat' kicked"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_cheat:
			GS::GetMainConsole().Console(wideformat(_("{} has been kicked for possible cheat"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_voted:
			GS::GetMainConsole().Console(wideformat(_("{} has been voted to leave the game"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_ac:
			GS::GetMainConsole().Console(wideformat(_("{} has been kicked for Anti-Cheat violation"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
		case kick_steamticket:
			GS::GetMainConsole().Console(wideformat(_("{} has been kicked for invalid Steam ticket"),
											 (sprite_system.GetSprite(playermsg->num).player->name)),
				client_message_color);
			break;
	}
	if (GS::GetGame().IsVoteActive())
	{
		switch (playermsg->why)
		{
			case kick_noresponse:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_nocheatresponse:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_ping:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_flooding:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_console:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_connectcheat:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_cheat:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_voted:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_ac:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
			case kick_silent:
				if (GS::GetGame().GetVoteTarget() == inttostr(playermsg->num))
				{
					GS::GetGame().stopvote();
				}
				break;
		}
	}

	if (playermsg->why != kick_changeteam)
	{
		sprite_system.GetSprite(playermsg->num).kill();
	}

	GS::GetGame().sortplayers();

	if ((sprite_system.IsPlayerSprite(playermsg->num)) && (GS::GetGame().GetMapchangecounter() < 1))
	{
		GS::GetGame().showmapchangescoreboard();
		gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu, false);
	}

	if ((playermsg->why != kick_changeteam) && (playermsg->why != kick_leftgame))
	{
		gGlobalStateInterfaceGraphics.fragsmenushow = false;
		gGlobalStateInterfaceGraphics.statsmenushow = false;
	}
}

void clienthandlemapchange::Handle(NetworkContext* netmessage)
{
	tmsg_mapchange* mapchange = nullptr;

	if (!VerifyPacket(sizeof(tmsg_mapchange), netmessage->size, msgid_mapchange))
	{
		return;
	}

	mapchange = reinterpret_cast<pmsg_mapchange>(netmessage->packet);

	std::string mapchangename;
	mapchangename.resize(mapchange->mapnamelength, '0');
	std::copy(mapchange->mapname.begin(), mapchange->mapname.begin() + mapchange->mapnamelength, mapchangename.begin());
	GS::GetGame().SetMapchangename(mapchangename);
	GS::GetGame().SetMapchangecounter(mapchange->counter);
	GS::GetGame().SetMapchangechecksum(mapchange->mapchecksum);
	gGlobalStateInterfaceGraphics.fragsmenushow = true;
	gGlobalStateInterfaceGraphics.statsmenushow = false;
	gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.limbomenu, false);
	gGlobalStateGame.heartbeattime = GS::GetGame().GetMainTickCounter();
	gGlobalStateGame.heartbeattimewarnings = 0;

	if (CVar::cl_endscreenshot)
	{
		gGlobalStateClient.screentaken = true;
	}

	for (auto& sprite : SpriteSystem::Get().GetActiveSprites())
	{
		gGlobalStateSound.stopsound(sprite.reloadsoundchannel);
		gGlobalStateSound.stopsound(sprite.jetssoundchannel);
		gGlobalStateSound.stopsound(sprite.gattlingsoundchannel);
		gGlobalStateSound.stopsound(sprite.gattlingsoundchannel2);
	}

	if (gGlobalStateDemo.demoplayer.active())
	{
		GS::GetGame().showmapchangescoreboard("");

		gGlobalStateDemo.demoplayer.stopdemo();
		return;
	}

	GS::GetMainConsole().Console(_("Next map:") + ' ' + (GS::GetGame().GetMapchangename()), game_message_color);

	if (!CVar::sv_survivalmode)
	{
		if ((CVar::sv_gamemode == gamestyle_deathmatch) || (CVar::sv_gamemode == gamestyle_pointmatch)
			|| (CVar::sv_gamemode == gamestyle_rambo))
		{
			if (GS::GetGame().GetSortedPlayers(1).playernum > 0)
			{
				gGlobalStateClient.camerafollowsprite = GS::GetGame().GetSortedPlayers(1).playernum;
			}
			if (!gGlobalStateGameMenus.escmenu->active)
			{
				gGlobalStateClientGame.mx = gGlobalStateGame.gamewidthhalf;
				gGlobalStateClientGame.my = gGlobalStateGame.gameheighthalf;
			}
		}
	}
}

void clienthandleflaginfo::Handle(NetworkContext* netmessage)
{
	auto& sprite_system = SpriteSystem::Get();
	std::int32_t j = 0;
	tvector2 a;
	tvector2 b;
	auto& things = GS::GetThingSystem().GetThings();

	if (!VerifyPacket(sizeof(tmsg_serverflaginfo), netmessage->size, msgid_flaginfo))
	{
		return;
	}

	if ((reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who < 1)
		|| (reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who > max_sprites))
	{
		return;
	}

	if (reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->style == returnred)
	{
		if (CVar::sv_gamemode == gamestyle_ctf)
		{
			gGlobalStateSound.playsound(SfxEffect::capture);
			gGlobalStateClientGame.bigmessage(_("Red Flag returned!"), capturemessagewait, alpha_message_color);

			GS::GetMainConsole().Console(
				wideformat(_("{} returned the Red Flag"),
					(sprite_system.GetSprite(reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who)
							.player->name)),
				alpha_message_color);
			if (GS::GetGame().GetTeamFlag(1) > 0)
			{
				things[GS::GetGame().GetTeamFlag(1)].respawn();
			}
		}
	}
	if (reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->style == returnblue)
	{
		if (CVar::sv_gamemode == gamestyle_ctf)
		{
			gGlobalStateSound.playsound(SfxEffect::capture);
			gGlobalStateClientGame.bigmessage(_("Blue Flag returned!"), capturemessagewait, alpha_message_color);

			GS::GetMainConsole().Console(
				wideformat(_("{} returned the Blue Flag"),
					(sprite_system.GetSprite(reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who)
							.player->name)),
				bravo_message_color);
			if (GS::GetGame().GetTeamFlag(2) > 0)
			{
				things[GS::GetGame().GetTeamFlag(2)].respawn();
			}
		}
	}
	if (reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->style == capturered)
	{
		gGlobalStateClientGame.bigmessage(_("Alpha Team Scores!"), capturectfmessagewait, alpha_message_color);
		GS::GetMainConsole().Console(
			wideformat(_("{} scores for Alpha Team"),
				(sprite_system.GetSprite(reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who).player->name)),
			alpha_message_color);

		if (CVar::sv_gamemode == gamestyle_inf)
		{
			gGlobalStateSound.playsound(SfxEffect::infiltmus);

			// flame it
			for (j = 1; j <= 10; j++)
			{
				a.x = things[GS::GetGame().GetTeamFlag(1)].skeleton.pos[2].x - 10 + Random(20);
				a.y = things[GS::GetGame().GetTeamFlag(1)].skeleton.pos[2].y - 10 + Random(20);
				b.x = 0;
				b.y = 0;
				gGlobalStateSparks.createspark(a, b, 36, 0, 35);
				if (Random(2) == 0)
				{
					gGlobalStateSparks.createspark(a, b, 37, 0, 75);
				}
			}
		}
		else
		{
			gGlobalStateSound.playsound(SfxEffect::ctf);
		}
		if (GS::GetGame().GetTeamFlag(2) > 0)
		{
			things[GS::GetGame().GetTeamFlag(2)].respawn();
		}

		// cap spark
		gGlobalStateSparks.createspark(things[GS::GetGame().GetTeamFlag(1)].skeleton.pos[2],
			b,
			61,
			reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who,
			18);

		if (CVar::sv_survivalmode)
		{
			GS::GetGame().SetSurvivalendround(true);
		}
	}
	if (reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->style == captureblue)
	{
		gGlobalStateClientGame.bigmessage(_("Bravo Team Scores!"), capturectfmessagewait, bravo_message_color);
		GS::GetMainConsole().Console(
			wideformat(_("{} scores for Bravo Team"),
				(sprite_system.GetSprite(reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who).player->name)),
			bravo_message_color);
		gGlobalStateSound.playsound(SfxEffect::ctf);
		if (GS::GetGame().GetTeamFlag(1) > 0)
		{
			things[GS::GetGame().GetTeamFlag(1)].respawn();
		}

		// cap spark
		gGlobalStateSparks.createspark(things[GS::GetGame().GetTeamFlag(2)].skeleton.pos[2],
			b,
			61,
			reinterpret_cast<pmsg_serverflaginfo>(netmessage->packet)->who,
			18);

		if (CVar::sv_survivalmode)
		{
			GS::GetGame().SetSurvivalendround(true);
		}
	}
}

void clienthandleidleanimation::Handle(NetworkContext* netmessage)
{
	auto& sprite_system = SpriteSystem::Get();
	std::int32_t i = 0;

	if (!VerifyPacket(sizeof(tmsg_idleanimation), netmessage->size, msgid_idleanimation))
	{
		return;
	}

	i = reinterpret_cast<pmsg_idleanimation>(netmessage->packet)->num;

	if (!sprite_system.GetSprite(i).active)
	{
		return;
	}

	sprite_system.GetSprite(i).idletime = 1;
	sprite_system.GetSprite(i).idlerandom = reinterpret_cast<pmsg_idleanimation>(netmessage->packet)->idlerandom;
}
