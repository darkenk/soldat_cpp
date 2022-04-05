// automatically converted

#define __networkclientfae_implementation__

#include "soldat_cpp/soldat/shared/network/NetworkClientFae.cpp.h"

/*#include "SysUtils.h"*/
/*#include "Constants.h"*/
/*#include "Game.h"*/
/*#include "LogFile.h"*/
/*#include "Net.h"*/
/*#include "NetworkUtils.h"*/
/*#include "TraceLog.h"*/
/*#include "Unit1.h"*/

void clientsendfaeresponse(tfaeresponsebox response)
{
  tmsg_faeresponse responsemsg;

  responsemsg.header.id = msgid_faedata;
  responsemsg.response = response;
  udp->senddata(&responsemsg, sizeof(responsemsg), k_nSteamNetworkingSend_Reliable);
}

void clienthandlefaechallenge(SteamNetworkingMessage_t *netmessage)
{
  pmsg_faechallenge msg;
  pfaechallenge challenge;
  tfaeresponsebox response;

  if (!verifypacket(sizeof(tmsg_faechallenge), netmessage->m_cbSize, msgid_faedata))
    return;

  if (faependingauth != nullptr)
    faeauthcancel(faependingauth);

  msg = pmsg_faechallenge(netmessage->m_pData);
  challenge = &msg->challenge;

  if (msg.inorder == 1)
  {
    // Synchronously process request (we must reply before sending our player info).
    // This might add a few milliseconds of delay while Fae does its thing, but the user won't
    // notice this as no frames are being rendered yet.
    if (!netencactive)
    {
      // NOTE that FaeAuthSync may fail, in which case NetEncKey is zeroed out. However, we're
      // not going to send any encrypted packets anyway if the server rejects us, hence we
      // ignore failure of FaeAuthSync.
      faeauthsync(challenge, response, &netenckey);
      netencactive = true;
    }
    else
    {
      faeauthsync(challenge, response, nullptr);
    }
    clientsendfaeresponse(response);
    // Debug('Sending sync. Fae response');
  }
  else
  {
    faependingauth = faeauthsubmit(challenge, 0);
    // Debug('Pending async. Fae response');
  }
}
