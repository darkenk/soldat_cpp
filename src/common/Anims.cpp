#include "Anims.hpp"
#include "Logging.hpp"
#include "PhysFSExt.hpp"

// comes from Constants.cpp.h
constexpr auto scale = 3;

void tanimation::doanimation()
{
  count += 1;
  if (count == speed)
  {
    count = 0;
    currframe += 1;
    if (currframe > numframes)
    {
      if (loop)
      {
        currframe = 1;
      }
      else
      {
        currframe = numframes;
      }
    }
  }
}

void tanimation::loadfromfile(TStream &stream)
{
  std::string r1;
  std::string r2;
  std::string r3;
  std::string r4;
  std::int32_t p;

  numframes = 1;

  // default settings
  loop = false;
  speed = 1;
  count = 0;

  stream.ReadLine(r1);
  while (r1 != "ENDFILE")
  {
    if (r1 == "NEXTFRAME")
    {
      if (numframes == max_frames_index)
      {
        LogWarnG("Corrupted frame index");
        break;
      }

      numframes += 1;
    }
    else
    {
      stream.ReadLine(r2); // X
      stream.ReadLine(r3); // Y
      stream.ReadLine(r4); // Z

      p = strtointdef(r1, 0);
      if ((p >= 1) && (p <= max_pos_index))
      {
        // TODO: check if this is correct
        frames[numframes].pos[p].x = -scale * strtofloat(r2) / 1.1;
        frames[numframes].pos[p].y = -scale * strtofloat(r4);
      }
      else
      {
        LogWarnG("Corrupted Index ({})", r1);
      }
    }
    stream.ReadLine(r1);
  }
  currframe = 1;
}

std::int32_t tanimation::checksum()
{
  float chk;
  std::int32_t i, j;

  chk = 0.5;

  for (i = 1; i <= numframes; i++)
    for (j = 1; j <= 20; j++)
    {
      chk = chk + frames[i].pos[j].x;
      chk = chk + frames[i].pos[j].y;
      chk = chk + frames[i].pos[j].z;
    }

  return trunc(chk);
}
