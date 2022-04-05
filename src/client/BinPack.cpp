// automatically converted
#include "BinPack.hpp"

#include <limits>

#include "shared/misc/MemoryUtils.hpp"

// Internal types

class TRectList final
{
private:
  pbprect fdata;
  std::int32_t fsize;
  std::int32_t fcapacity;

  tbprect &GetValue(std::int32_t index);
  void SetValue(std::int32_t index, const tbprect &value);

public:
  TRectList();
  ~TRectList();
  void Push(const tbprect &rect);
  void Remove(std::int32_t index);

  std::int32_t Size()
  {
    return fsize;
  };
  tbprect &operator[](std::int32_t index);
  //      property Items[Index: Integer]: TBPRect read GetValue write SetValue;
  //        default;
};

struct TBinPack final
{
  TRectList used;
  TRectList free;
};

// Forward declarations of internal functions

static tbprect ScoreRect(TBinPack &bp, std::int32_t w, std::int32_t h, std::int32_t &besty,
                         std::int32_t &bestx);

static void PlaceRect(TBinPack &bp, const tbprect rect);

static bool SplitFreeRect(TBinPack &bp, tbprect freerect, const tbprect usedrect);

static void PruneFreeList(TBinPack &bp);

static bool IsContainedIn(const tbprect a, tbprect b);

// PackRects

std::int32_t packrects(std::int32_t w, std::int32_t h, tbprectarray &rects)
{
  TBinPack bp;
  std::size_t i, j;
  tbprect rect;
  tbprect bestrect;
  std::int32_t bestindex;
  std::int32_t bestscore1;
  std::int32_t bestscore2;
  std::int32_t score1;
  std::int32_t score2;

  // todo what to do with default?
  // bestrect = default_(tbprect);
  rect.x = 0;
  rect.y = 0;
  rect.w = w;
  rect.h = h;
  score1 = 0;
  score2 = 0;

  bp.free.Push(rect);

  i = 0;

  while (i < rects.size())
  {
    bestindex = -1;
    bestscore1 = std::numeric_limits<std::int32_t>::max();
    bestscore2 = std::numeric_limits<std::int32_t>::max();

    for (j = i; j < rects.size(); j++)
    {
      rect = ScoreRect(bp, rects[j].w, rects[j].h, score1, score2);

      if ((score1 < bestscore1) || ((score1 == bestscore1) && (score2 < bestscore2)))
      {
        bestscore1 = score1;
        bestscore2 = score2;
        bestrect = rect;
        bestindex = j;
      }
    }

    if ((bestrect.h == 0) || (bestindex == -1))
      break;

    PlaceRect(bp, bestrect);

    bestrect.data = rects[bestindex].data;
    rects[bestindex] = rects[i];
    rects[i] = bestrect;

    i += 1;
  }

  return i;
}

// Internal functions

tbprect ScoreRect(TBinPack &bp, std::int32_t w, std::int32_t h, std::int32_t &besty,
                  std::int32_t &bestx)
{
  tbprect bestrect;
  std::int32_t topsidey;
  std::int32_t i;

  tbprect scorerect_result;
  bestrect.x = 0;
  bestrect.y = 0;
  bestrect.w = 0;
  bestrect.h = 0;

  bestx = std::numeric_limits<std::int32_t>::max();
  besty = std::numeric_limits<std::int32_t>::max();

  for (i = 0; i <= bp.free.Size() - 1; i++)
  {
    if ((bp.free[i].w >= w) && (bp.free[i].h >= h))
    {
      topsidey = bp.free[i].y + h;

      if ((topsidey < besty) || ((topsidey == besty) && (bp.free[i].x < bestx)))
      {
        bestrect.x = bp.free[i].x;
        bestrect.y = bp.free[i].y;
        bestrect.w = w;
        bestrect.h = h;
        besty = topsidey;
        bestx = bp.free[i].x;
      }
    }

    // Note: this part will be enabled later on when I add support for
    // rotated sprites in the texture atlas. That will be done when I get
    // more control over the actual texture coordinates.
    /*
    if (bp.Free[i].w >= h) and (bp.Free[i].h >= w) then
    begin
      TopSideY := bp.Free[i].y + w;

      if (TopSideY < BestY) or ((TopSideY = BestY) and (bp.Free[i].x < BestX))
    then begin BestRect.x := bp.Free[i].x; BestRect.y := bp.Free[i].y;
        BestRect.w := h;
        BestRect.h := w;
        BestY := TopSideY;
        BestX := bp.Free[i].x;
      end;
    end;
    */
  }

  if (bestrect.h == 0)
  {
    bestx = std::numeric_limits<std::int32_t>::max();
    besty = std::numeric_limits<std::int32_t>::max();
  }

  return bestrect;
}

void PlaceRect(TBinPack &bp, const tbprect rect)
{
  std::int32_t i, n;

  i = 0;
  n = bp.free.Size();

  while (i < n)
  {
    if (SplitFreeRect(bp, bp.free[i], rect))
    {
      bp.free.Remove(i);
      i -= 1;
      n -= 1;
    }

    i += 1;
  }

  PruneFreeList(bp);
  bp.used.Push(rect);
}

bool SplitFreeRect(TBinPack &bp, tbprect freerect, const tbprect usedrect)
{
  tbprect rect;

  if ((usedrect.x >= (freerect.x + freerect.w)) || ((usedrect.x + usedrect.w) <= freerect.x) ||
      (usedrect.y >= (freerect.y + freerect.h)) || ((usedrect.y + usedrect.h) <= freerect.y))
  {
    return false;
  }

  if ((usedrect.x < (freerect.x + freerect.w)) && ((usedrect.x + usedrect.w) > freerect.x))
  {
    if ((usedrect.y > freerect.y) && (usedrect.y < (freerect.y + freerect.h)))
    {
      rect = freerect;
      rect.h = usedrect.y - rect.y;
      bp.free.Push(rect);
    }

    if ((usedrect.y + usedrect.h) < (freerect.y + freerect.h))
    {
      rect = freerect;
      rect.y = usedrect.y + usedrect.h;
      rect.h = freerect.y + freerect.h - (usedrect.y + usedrect.h);
      bp.free.Push(rect);
    }
  }

  if ((usedrect.y < (freerect.y + freerect.h)) && ((usedrect.y + usedrect.h) > freerect.y))
  {
    if ((usedrect.x > freerect.x) && (usedrect.x < (freerect.x + freerect.w)))
    {
      rect = freerect;
      rect.w = usedrect.x - rect.x;
      bp.free.Push(rect);
    }

    if ((usedrect.x + usedrect.w) < (freerect.x + freerect.w))
    {
      rect = freerect;
      rect.x = usedrect.x + usedrect.w;
      rect.w = freerect.x + freerect.w - (usedrect.x + usedrect.w);
      bp.free.Push(rect);
    }
  }

  return true;
}

void PruneFreeList(TBinPack &bp)
{
  std::int32_t i, j, n;

  i = 0;
  n = bp.free.Size();

  while (i < n)
  {
    j = i + 1;

    while (j < n)
    {
      if (IsContainedIn(bp.free[i], bp.free[j]))
      {
        bp.free.Remove(i);
        i -= 1;
        n -= 1;
        break;
      }

      if (IsContainedIn(bp.free[j], bp.free[i]))
      {
        bp.free.Remove(j);
        j -= 1;
        n -= 1;
      }

      j += 1;
    }

    i += 1;
  }
}

bool IsContainedIn(const tbprect a, tbprect b)
{
  return (a.x >= b.x) && (a.y >= b.y) && ((a.x + a.w) <= (b.x + b.w)) &&
         ((a.y + a.h) <= (b.y + b.h));
}

// TRectList class

void RectMemCopy(pbprect a, pbprect b, std::int32_t n)
{
  std::int32_t i;

  for (i = 0; i <= n - 1; i++)
  {
    *a = *b;
    a += 1;
    b += 1;
  }
}

TRectList::TRectList()
{
  fsize = 0;
  fcapacity = 16;
  getmem(fdata, fcapacity * sizeof(tbprect));
}

TRectList::~TRectList()
{
  freemem(fdata);
}

tbprect &TRectList::GetValue(std::int32_t index)
{
  pbprect p;

  p = fdata;
  p += index;
  return *p;
}

void TRectList::SetValue(std::int32_t index, const tbprect &value)
{
  pbprect p;

  p = fdata;
  p += index;
  *p = value;
}

void TRectList::Push(const tbprect &rect)
{
  pbprect p;

  if (fsize == fcapacity)
  {
    fcapacity = 2 * fcapacity;
    getmem(p, fcapacity * sizeof(tbprect));
    RectMemCopy(p, fdata, fsize);
    freemem(fdata);
    fdata = p;
  }

  p = fdata;
  p += fsize;
  fsize += 1;
  *p = rect;
}

void TRectList::Remove(std::int32_t index)
{
  pbprect a, b;

  a = fdata;
  b = fdata;

  a += index;
  b += index + 1;

  RectMemCopy(a, b, fsize - index - 1);
  fsize -= 1;
}

tbprect &TRectList::operator[](int32_t index)
{
  return GetValue(index);
}
