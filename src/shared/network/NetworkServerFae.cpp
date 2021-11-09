// automatically converted

#define __networkserverfae_implementation__

#include "soldat_cpp/soldat/shared/network/NetworkServerFae.cpp.h"

/*#include "SysUtils.h"*/
/*#include "Constants.h"*/
/*#include "Game.h"*/
/*#include "FaeBase.h"*/
/*#include "FaeRemoteAttestation.h"*/
/*#include "Net.h"*/
/*#include "NetworkServerConnection.h"*/
/*#include "NetworkServerMessages.h"*/
/*#include "NetworkUtils.h"*/
/*#include "TraceLog.h"*/
/*#include "Unit1.h"*/

bool isfaegamedatavalid(tfaeresponse response)
{
    // TODO check that Response.GameKey matches Soldat's signature public key
    // TODO check that Response.GameVersion matches the client version we expect
    bool isfaegamedatavalid_result;
    response = response;
    result = true;
    return isfaegamedatavalid_result;
}

void kickforfaeviolation(tplayer player, std::string reason)
{
    const std::wstring &name;
    const std::wstring &msgtext;

    name = const std::wstring & (player.name);
    if (name == "")
        name = string("Unnamed player ") + const std::wstring and
               (player.ip) + ':' + const std::wstring & (inttostr(player.port));
    msgtext = name + " removed by anti-cheat: " + const std::wstring & (reason);
    mainconsole.console(string("[AC] ") + msgtext, client_message_color);
    player.faekicked = true;
    if (player.spritenum != 0)
    {
        serversendstringmessage(string("/say ") + msgtext, all_players, 255, msgtype_pub);
        kickplayer(player.spritenum, false, kick_ac, 0, string("Anti-Cheat: ") + reason);
    }
    else
    {
        serversendunaccepted(player.peer, anticheat_rejected, reason);
    }
}

void serversendfaechallenge(HSteamNetConnection peer, bool inorder)
{
    tplayer player;
    tmsg_faechallenge challengemsg;

    player = tplayer(peer->data);
    faeinitsecret(&player.faesecret);
    faeinitchallenge(&player.faesecret, challengemsg.challenge);

    // The first Fae request's secret is used for deriving a session key.
    // The client recovers this key by calling FaeAuthSync.
    if (!player.encactive)
        faederivekey(&player.faesecret, &player.enckey);

    player.faeresponsepending = true;

    challengemsg.inorder = (std::uint8_t)(inorder);
    challengemsg.header.id = msgid_faedata;
    udp->senddata(&challengemsg, sizeof(challengemsg), peer, k_nSteamNetworkingSend_Reliable);
}

void serverhandlefaeresponse(SteamNetworkingMessage_t* netmessage)
{
    tplayer player;
    pmsg_faeresponse responsemsg;
    std::uint32_t outerstatus;
    std::string reason;
    std::int32_t validationerr;
    tfaeresponse validatedresp;

    trace("[AC] Received response");

    if (!verifypacket(sizeof(tmsg_faeresponse), netmessage->m_cbSize, msgid_faedata))
        return;

    player = tplayer(netmessage->m_nConnUserData);

    if (!player.faeresponsepending)
    {
        debug("[AC] Warning: Discarding unexpected response");
        return;
    }

    trace("[AC] Expected response");

    responsemsg = pmsg_faeresponse(netmessage->m_pData);
    outerstatus = responsemsg->response.outerstatus;

    if (outerstatus != 0)
    {
        // The response is well-formed, but the AC cannot process our secret, for example because it
        // is out-of-date or some kind of initialization failed.
        trace("[Fae] But the client refused to process our secret");
        if ((outerstatus & 0x80000000) != 0)
            reason = string("Ban ID #") + inttostr(outerstatus & 0x7fffffff);
        else
            switch (outerstatus)
            {
            case 1:
                reason = "Test Error";
                break; // test by setting client's env.-var: FAE_TESTMODE=status1
            case 2:
                reason = "Soldat Restart Required";
                break;
            case 3:
                reason = "Anti-Cheat Activation Failed";
                break;
            default:
                reason = string("Initialization Error #") + inttostr(outerstatus);
            }
        kickforfaeviolation(player, reason);
    }
    else
    {
        validationerr = faecheck(&player.faesecret, &responsemsg->response, validatedresp);
        if (validationerr != faecheck_ok)
        {
            switch (validationerr)
            {
            case faecheck_err_invalid:
                reason = "Corrupted Message";
                break;
            case faecheck_err_clock:
                reason = "Client/Server Clock Mismatch";
                break;
            default:
                reason = string("Validation Error #") + inttostr(validationerr);
            }
            kickforfaeviolation(player, reason);
        }
        else
        {
            // This was a valid response for our secret -- we may now read from ValidatedResp
            trace("[Fae] And it was valid, processing");
            if (!isfaegamedatavalid(validatedresp))
            {
                // The client's fae.db doesn't contain the info this server build expects
                kickforfaeviolation(player, "Client Build Info Mismatch");
            }
            else if (validatedresp.status != 0)
            {
                // Fae found some issue with the game client and we should boot the player.
                // You may test this by setting the client's environment variable
                // FAE_TESTMODE=status2, which results in 'Test Code #1000'.
                reason = pansichar(&validatedresp.statusstr[1]);
                if (length(reason) == 0)
                    reason = string("Code #") + inttostr(validatedresp.status);
                else
                    reason = reason + " #" + inttostr(validatedresp.status);
                kickforfaeviolation(player, reason);
            }
            else
            {
                // Everything is fine. Reset the ticks counter so that the player is not kicked.
                trace("[AC] All good, timer reset");
                player.faeresponsepending = false;
                player.faeticks = 0;

                // Send encrypted packets from now on.
                if (!player.encactive)
                {
                    debug(string("[AC] Network encryption enabled for ") + player.ip + ':' +
                          inttostr(player.port));
                    player.encactive = true;
                }
            }
        }
    }
}
