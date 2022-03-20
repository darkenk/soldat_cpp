// automatically converted
#include "ServerHelper.hpp"

#include "BanSystem.hpp"
#include "Server.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Command.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkServerMessages.hpp"
#include <filesystem>

// procedure WriteLn1(S: Variant); overload;
// begin
//  if CvarsInitialized and log_timestamp.Value then
//    System.Writeln1(FormatDateTime('[hh:nn:ss] ', Now) + S)
//  else
//    System.Writeln1(S);
// end;

std::string idtoname(std::int32_t id)
{
    std::string result;
    result = "";
    if (id > max_players)
    {
        result = "Server Admin";
        return result;
    }

    if (SpriteSystem::Get().GetSprite(id).IsActive())
        result = SpriteSystem::Get().GetSprite(id).player->name;
    return result;
}

std::string teamtoname(std::int32_t id)
{
    std::string result;
    result = "UNKNOWN";
    if (id > 5)
        return result;

    switch (id)
    {
    case 0:
        result = "NA";
        break;
    case 1:
        result = 'A';
        break;
    case 2:
        result = 'B';
        break;
    case 3:
        result = 'C';
        break;
    case 4:
        result = 'D';
        break;
    case 5:
        result = "Spectator";
        break;
    }
    return result;
}

std::int32_t nametoid(std::string name)
{
    std::int32_t i;

    std::int32_t result;
    result = 0;
    for (i = 1; i <= max_sprites; i++)
        if (SpriteSystem::Get().GetSprite(i).player->name == name)
            result = i;
    return result;
}

std::string nametohw(std::string name)
{
    std::int32_t i;

    std::string result;
    result = "0";
    for (i = 1; i <= max_sprites; i++)
        if (SpriteSystem::Get().GetSprite(i).player->name == name)
            result = SpriteSystem::Get().GetSprite(i).player->hwid;
    return result;
}

std::uint32_t rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
{
    return (r | (g << 8) | (b << 16));
}

std::int32_t findlowestteam(const std::vector<std::int32_t> &arr)
{
    std::int32_t i, tmp;

    std::int32_t result;
    tmp = 1;
    for (i = 1; i <= iif(CVar::sv_gamemode == gamestyle_teammatch, 4, 2); i++)
    {
        if (arr[tmp] > arr[i])
            tmp = i;
    }
    result = tmp;
    return result;
}

std::uint8_t fixteam(std::uint8_t team)
{
    std::uint8_t result;
    result = team_spectator;

    switch (CVar::sv_gamemode)
    {
    case gamestyle_deathmatch:
    case gamestyle_pointmatch:
    case gamestyle_rambo: {
        // Choose to team 0 or leave at team spectator
        if ((team != team_none) && (team != team_spectator))
            result = team_none;
    }
    break;
    case gamestyle_teammatch: {
        if ((team != team_alpha) && (team != team_bravo) && (team != team_charlie) &&
            (team != team_delta) && (team != team_spectator))
            result = Random(4) + 1; // team 1..4
    }
    break;
    case gamestyle_ctf:
    case gamestyle_inf:
    case gamestyle_htf: {
        if ((team != team_alpha) && (team != team_bravo) && (team != team_spectator))
            result = Random(2) + 1; // team 1..2
    }
    break;
    }
    return result;
}
#if 0

std::string weaponnamebynum(std::int32_t num)
{
    std::int32_t weaponindex;

    std::string result;
    trace("WeaponNameByNum");
    result = "USSOCOM";

    if (num == 100)
    {
        result = "Selfkill";
        return result;
    }

    for (weaponindex = low(guns); weaponindex <= high(guns); weaponindex++)
    {
        if (num == guns[weaponindex].num)
        {
            result = guns[weaponindex].name;
            break;
        }
    }
    return result;
}
#endif

std::string checknextmap()
{
    std::int32_t m;

    std::string result;
    LogTraceG("CheckNextMap");
    result = "NOMAP";
    if (mapslist.size() < 1)
    {
        result = "NOMAP";
    }
    else
    {
        m = mapindex + 1;

        if (m >= mapslist.size())
            m = 0;

        result = mapslist[m];
    }
    return result;
}

void savetxtlists()
{
    LogTraceG("SaveTxtLists");

    // save ban files
    savebannedlist(GS::GetGame().GetUserDirectory() + "configs/banned.txt");
    savebannedlisthw(GS::GetGame().GetUserDirectory() + "configs/bannedhw.txt");

    remoteips.savetofile(GS::GetGame().GetUserDirectory() + "configs/remote.txt");
}

void savemaplist()
{
    mapslist.savetofile(GS::GetGame().GetUserDirectory() + "configs/" +
                        std::string(CVar::sv_maplist));
}

void writepid()
{
    NotImplemented(NITag::OTHER);
#if 0
    std::uint32_t pid;
    textfile pidfile;

    //  try
    pid = getpid(0);
    assignfile(pidfile, userdirectory + "logs/" + CVar::sv_pidfilename);
    filemode = fmopenwrite;
    rewrite(pidfile);
    output << pidfile << inttostr(pid);
    writeln1(string(" Server PID: ") + inttostr(pid));
    closefile(pidfile);
    //  except
    //    writeln('Error writing PID file');
    //  end;
#endif
}

std::int32_t soldat_getpid()
{
    NotImplemented(NITag::OTHER);
    return 0;
#if 0
    std::int32_t getpid_result;
    result = system.getprocessid;
    return getpid_result;
#endif
}

void writeconsole(std::uint8_t id, std::string text, uint32 colour)
{
    // Write text to the console of ALL Players
    serversendspecialmessage(text, 0, 0, 0, 0, colour, 0, 0, id);
}

void updatewaverespawntime()
{
    waverespawntime = round(GS::GetGame().GetPlayersNum() * waverespawn_time_mulitplier) * 60;
    if (waverespawntime > CVar::sv_respawntime_minwave)
        waverespawntime = CVar::sv_respawntime_maxwave;
    waverespawntime = waverespawntime - CVar::sv_respawntime_minwave;
    if (waverespawntime < 1)
        waverespawntime = 1;
}

std::string randombot()
{
    NotImplemented(NITag::OTHER);
    return {};
#if 0
    tstringlist botlist;
    std::string selectedbot;
    tsearchrec searchresult;

    std::string randombot_result;
    botlist = tstringlist.create;
    if (findfirst(GS::GetGame().GetUserDirectory() + "configs/bots/*.bot", faanyfile, searchresult) == 0)
    {
        do
        {
            botlist.add(searchresult.name);
        } while (!(findnext(searchresult) != 0));
    }
    findclose(searchresult);

    if (botlist.count > 0)
    {
        selectedbot = botlist[Random(botlist.count)];
        selectedbot = ansireplacestr(selectedbot, userdirectory + "configs/bots/", "");
        selectedbot = ansireplacestr(selectedbot, ".bot", "");
    }
    else
    {
        selectedbot = "Dummy";
    }

    if ((selectedbot == "Boogie Man") || (selectedbot == "Dummy"))
        selectedbot = "Sniper";

    result = selectedbot;
    return randombot_result;
#endif
}

void dobalancebots(std::uint8_t leftgame, std::uint8_t newteam)
{
    std::array<std::int32_t, 4> teams;
    std::string thebot;

    if (!CVar::sv_botbalance)
        return;
    if ((CVar::sv_gamemode != gamestyle_ctf) && (CVar::sv_gamemode != gamestyle_htf) &&
        (CVar::sv_gamemode != gamestyle_inf))
        return;

    teams[1] = 0;
    teams[2] = 0;
    teams[3] = 0;
    teams[4] = 0;

    auto &activeSprites = SpriteSystem::Get().GetActiveSprites();
    std::for_each(std::begin(activeSprites), std::end(activeSprites), [&teams](auto &sprite) {
        if (sprite.isnotspectator())
        {
            teams[sprite.player->team] += 1;
        }
    });

    if (leftgame == 1)
    {
        // Player Left Game
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {
            if (sprite.player->controlmethod == bot)
            {
                if ((teams[1] > teams[2]) && (sprite.player->team == team_alpha))
                {
                    kickplayer(sprite.num, false, kick_leftgame, 0);
                    return;
                }
                if ((teams[2] > teams[1]) && (sprite.player->team == team_bravo))
                {
                    kickplayer(sprite.num, false, kick_leftgame, 0);
                    return;
                }
            }
        }
    }
    else
    {
        // Player Joined Game}
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {
            if ((sprite.player->controlmethod == bot) && (sprite.player->team == newteam))
            {
                if (teams[1] > teams[2])
                {
                    kickplayer(sprite.num, false, kick_leftgame, 0);
                    if (sprite.player->team == newteam)
                    {
                        dobalancebots(1, 2);
                    }
                    return;
                }
                if (teams[2] > teams[1])
                {
                    kickplayer(sprite.num, false, kick_leftgame, 0);
                    if (sprite.player->team == newteam)
                    {
                        dobalancebots(1, 1);
                    }
                    return;
                }
            }
        }
    }
    if (teams[1] > teams[2])
    {
        thebot = randombot();
        parseinput(std::string("addbot2 ") + thebot, 1);
        GetServerMainConsole().console(thebot + " has joined bravo team. (Bot Balance)",
                                       enter_message_color);
        return;
    }
    if (teams[2] > teams[1])
    {
        thebot = randombot();
        parseinput(std::string("addbot1 ") + thebot, 1);
        GetServerMainConsole().console(thebot + " has joined alpha team. (Bot Balance)",
                                       enter_message_color);
        return;
    }
}
