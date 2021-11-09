#pragma once

/*#include "SysUtils.h"*/
/*#include "Classes.h"*/
/*#include "sha1.h"*/
/*#include "strutils.h"*/
/*#include "fphttpclient.h"*/
/*#include "sslsockets.h"*/
/*#include "fpopenssl.h"*/
/*#include "GameRendering.h"*/
/*#include "Constants.h"*/
/*#include "Version.h"*/

const std::int32_t max_dl_size = 150000000; // max download size in bytes

class tdownloadthread : public tthread
{
  public:
  private
    std::string furl;
    std::string ffilename;
    tsha1digest fchecksum;
    std::string ferrormsg;
    std::uint8_t fstatus;
    std::uint8_t fprogress;
    int64 fdownloadpos;
    int64 fdownloadsize;
    tfphttpclient client;
    //    protected
    void execute();
    procedure setstatus;
    procedure seterror;

  public
    tdownloadthread *create(string downloadurl, std::string name, tsha1digest checksum);
    void doprogress(tobject sender, const int64 contentlength, int64 currentpos);
    void doonterminate(tobject sender);
    void canceldownload();
    tdownloadthread *destroy();
};

#ifdef __fileclient_implementation__
#undef EXTERN
#define EXTERN
#endif

extern std::uint8_t downloadretry;
#undef EXTERN
#define extern extern
= 0;
