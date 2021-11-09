// automatically converted

#define __fileclient_implementation__

#include "soldat_cpp/soldat/client/FileClient.cpp.h"

/*#include "Client.h"*/
/*#include "Util.h"*/

tdownloadthread *tdownloadthread::create(string downloadurl, std::string name, tsha1digest checksum)
{
    inherited create(false);
    furl = downloadurl;
    ffilename = ansireplacestr(name, "..", "");
    fchecksum = checksum;
    onterminate = doonterminate();
    freeonterminate = false;
    return this;
}

void tdownloadthread::canceldownload()
{
    client.terminate;
}

void tdownloadthread::seterror()
{
    showmessage(string("Download error: ") + const std::wstring &(ferrormsg));
    mainconsole.console(string("Download error: ") + ferrormsg, debug_message_color);
}

void tdownloadthread::setstatus()
{
    rendergameinfo(
        const std::wstring &(format("Downloading %s - %d%% (%s/%s)",
                                    set::of(extractfilename(ffilename), fprogress,
                                            getsize(fdownloadpos), getsize(fdownloadsize), eos))));
}

void tdownloadthread::doprogress(tobject sender, const int64 contentlength, int64 currentpos)
{
    int64 oldprogress;

    if ((contentlength > max_dl_size) || (currentpos > max_dl_size))
    {
        fstatus = 3;
        client.terminate;
        return;
    }
    if (contentlength > 0)
    {
        oldprogress = fprogress;
        fprogress = round(((float)(currentpos) / contentlength) * 100);
        fdownloadsize = contentlength;
        fdownloadpos = currentpos;
        if (fprogress != oldprogress)
            synchronize(setstatus());
    }
}

void tdownloadthread::execute()
{
    client = tfphttpclient.create(nullptr);
    {
        void &with = client;

        //  try
        //    try
        addheader("User-Agent", string("soldatclient/") + soldat_version);
        allowredirect = false;
        iotimeout = 1000;
        ondatareceived = doprogress();
        createdirifmissing(extractfilepath(ffilename));
        get(furl, ffilename);
        //      if FStatus = 3 then
        //        raise Exception.Create('The requested file is too large to download directly');
        //      if not Sha1Match(Sha1File(FFilename, 4096), FCheckSum) then
        //        raise Exception.Create('Checksum mismatch');
        fstatus = 1;
        //    except
        //      on E: Exception do
        //      begin
        deletefile(ffilename);
        ferrormsg = e.message;
        synchronize(seterror());
        //      end;
    }
    //    finally
    //      Client.Terminate;
    //    end;
}

tdownloadthread *tdownloadthread::destroy()
{
    client.free;
    inherited destroy();
    return this;
}

void tdownloadthread::doonterminate(tobject sender)
{
    client.terminate;
    downloadretry += 1;
    if (downloadretry == 1)
        if (fstatus == 1)
            joinserver;
    freeandnullptr(downloadthread);
}
