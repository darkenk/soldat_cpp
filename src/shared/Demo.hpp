#pragma once

/*#include "GameStrings.h"*/
/*#include "ClientGame.h"*/
/*#include "SysUtils.h"*/
/*#include "Classes.h"*/
/*#include "Vector.h"*/
/*#include "Sprites.h"*/
/*#include "GameNetworkingSockets.h"*/
/*#include "Net.h"*/

#include <array>
#include <cstdint>
#include <string>

struct tmemorystream
{
    enum pos
    {
        sofrombeginning
    };

    std::int32_t position;
    std::int32_t size;

    bool writebuffer(const void *buff, std::int32_t size);

    bool savetofile(const std::string &filename);

    void write1(const void *buff, std::int32_t size);
    void read1(void *buff, std::int32_t size);

    void readbuffer(void *buff, std::int32_t size);

    void loadfromfile(const std::string &filename);

    void seek(std::int32_t bytes, pos position);

    void free();
};

constexpr std::array<char, 6> demo_magic = {'S', 'O', 'L', 'D', 'E', 'M'};

struct tdemoheader
{
    std::array<char, 6> header;
    std::uint64_t version;
    std::array<char, 160> mapname;
    std::int32_t startdate;
    std::int32_t ticksnum;
};

class tdemo
{
  protected:
    tmemorystream fdemofile;
    tdemoheader fdemoheader;
    bool factive;
    std::string fname;
    std::uint8_t foldcam;

  public:
    bool active()
    {
        return factive;
    };
    void setactive(bool active)
    {
        factive = active;
    }
    const std::string &name() const
    {
        return fname;
    }
    const tdemoheader &header() const
    {
        return fdemoheader;
    };
};

class tdemorecorder : public tdemo
{
  private:
    std::int32_t fticksnum;
    std::int32_t createdemoplayer();
#ifndef SERVER
    void savecamera();
#endif
  public:
    bool startrecord(const std::string &filename);
    void stoprecord();
    void saverecord(const void *r, std::int32_t size);
    void savenextframe();
#ifndef SERVER
    void saveposition();
#endif
    //    property Active: Boolean read FActive write FActive;
    std::int32_t ticksnum()
    {
        return fticksnum;
    }
};

#ifndef SERVER
class tdemoplayer : public tdemo
{
  private:
    std::int32_t fskipto;

  public:
    bool opendemo(const std::string &filename);
    void stopdemo();
    void processdemo();
    void position(std::int32_t ticks);
    std::int32_t skipto()
    {
        return fskipto;
    }
};
#endif

extern tdemorecorder demorecorder;
#ifndef SERVER
extern tdemoplayer demoplayer;
#endif
extern std::uint64_t rsize;
extern std::uint8_t freecam;
extern std::uint8_t notexts;
