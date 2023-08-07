#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

class Rectangle
{
public:
  Rectangle(std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height)
    : x{x}, y{y}, w{width}, h{height}
  {
  }
  std::uint32_t x, y, w, h;
};

class RectangleArea
{
public:
  RectangleArea(std::uint32_t width, std::uint32_t height);

  bool Add(Rectangle &rect);

private:
  struct SkylineItem
  {
    SkylineItem() = default;
    SkylineItem(std::uint32_t width, std::uint32_t height, std::uint32_t startPos)
      : Width(width), Height(height), StartPos(startPos)
    {
    }

    std::uint32_t Width = 0;
    std::uint32_t Height = 0;
    std::uint32_t StartPos = 0;
  };

  using SkylineItemContainer = std::vector<SkylineItem>;

  void MergeItems(SkylineItemContainer::iterator mergeTo, SkylineItemContainer::iterator mergeFrom);

  void MergeWithNeighbours(SkylineItemContainer::iterator it);

  void RaiseSkylineLevel();

  SkylineItemContainer Skyline;
  std::uint32_t Width;
  std::uint32_t Height;
};
