#pragma once


/*#include "contnrs.h"*/
/*#include "syncobjs.h"*/
/*#include "IdSocketHandle.h"*/
/*#include "IdGlobal.h"*/
/*#include "IdTCPServer.h"*/
/*#include "IdIOHandlerSocket.h"*/
/*#include "IdTCPConnection.h"*/
/*#include "IdContext.h"*/
/*#include "IdCustomTCPServer.h"*/
/*#include "Classes.h"*/
/*#include "SysUtils.h"*/
/*#include "Version.h"*/
/*#include "Constants.h"*/
/*#include "Net.h"*/


const std::int32_t refreshx_header_chars = 10;

//  TServerConnect = procedure(AThread: TIdContext) of object;
//  TServerDisconnect = procedure(AThread: TIdContext) of object;
//  TServerExecute = procedure(AThread: TIdContext) of object;

struct tadminmessage {
  std::string msg, ip;
  tidport port;
};

class tadminserver : public tidtcpserver {
public:
private
  std::string fpassword;
  tthreadlist fadmins;
  std::string fconnectmessage;
  int64 fbytessent;
  int64 fbytesrecieved;
  tqueue fmessagequeue;
  tcriticalsection fqueuelock;
  void handleconnect(tidcontext athread);
  procedure handledisconnect(athread: tidcontext);
  procedure handleexecute(athread: tidcontext);
public
  void admins() read fadmins;
//property password: std::string read Fpassword write Fpassword;
//property ConnectMessage: std::string read FConnectMessage write FConnectMessage;
  void bytessent() read fbytessent;
  void bytesrecieved() read fbytesrecieved;
  tadminserver* create(string pass, std::string connectmessage = ''); 
  tadminserver* destroy(); 
  void sendtoall(string message);
  void sendtoip(string ip, std::string message);
  void processcommands();
};

// Extra Server Information
struct tmsg_refreshx {
  std::array<char, refreshx_header_chars> header;
  array<1,max_players,varying_string<playername_chars> > name;
  array<1,max_players,varying_string<playerhwid_chars> > hwid;
  std::array<std::uint8_t, max_players> team;
  std::array<std::uint64_t, max_players> kills;
  std::array<std::uint8_t, max_players> caps;
  std::array<std::uint64_t, max_players> deaths;
  array<1,max_players,std::int32_t> ping;
  std::array<std::uint8_t, max_players> number;
  matrix<1,max_players, 1,4,std::uint8_t> ip;
  std::array<float, max_players> x;
  std::array<float, max_players> y;
  float redflagx;
  float redflagy;
  float blueflagx;
  float blueflagy;
  std::array<std::uint64_t, 4> teamscore;
  varying_string<16> mapname;
  std::int32_t timelimit, currenttime;
  std::uint64_t killlimit;
  std::uint8_t gamestyle;
  std::uint8_t maxplayers;
  std::uint8_t maxspectators;
  std::uint8_t passworded;
  varying_string<16> nextmap;
};

  void broadcastmsg(string text);
  void sendmessagetoadmin(string toip, std::string text);


#ifdef __rcon_implementation__
#undef EXTERN
#define EXTERN
#endif

extern tidserverthreadevent fserverconnect;
extern tidserverthreadevent fserverdisconnect;
extern tidserverthreadevent fserverexecute;
#undef EXTERN
#define extern extern


