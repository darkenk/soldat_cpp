#include "FontAtlas.hpp"
#include "common/misc/PortUtils.hpp"

RectangleArea::RectangleArea(std::uint32_t width, std::uint32_t height)
    : Width(width), Height(height)
{
    Skyline.emplace_back(Width, 0, 0);
}

bool RectangleArea::Add(Rectangle &rect)
{
    if (rect.w > Width || rect.h > Height)
    {
        return false;
    }

    auto it = Skyline.end();

    do
    {
        auto fit = [this, rect](const SkylineItem &item) {
            return item.Width >= rect.w && (item.Height + rect.h) < Height;
        };
        it = std::min_element(Skyline.begin(), Skyline.end(), [fit](auto const &l, auto const &r) {
            auto fitL = fit(l);
            auto fitR = fit(r);
            if (fitL && fitR)
            {
                return l.Height <= r.Height;
            }
            return fitL;
        });
        if (!fit(*it))
        {
            it = Skyline.end();
        }

        if (Skyline.end() == it)
        {
            if (Skyline.size() == 1)
            {
                return false;
            }

            RaiseSkylineLevel();

            it = Skyline.end();
        }
    } while (it == Skyline.end());

    if (it->Width != rect.w)
    {
        it = Skyline.emplace(it + 1, it->Width - rect.w, it->Height, it->StartPos + rect.w) - 1;
        it->Width = rect.w;
        Assert(it->Width + it->StartPos <= Width);
    }

    rect.x = it->StartPos;
    rect.y = it->Height;
    it->Height += rect.h;

    MergeWithNeighbours(it);

    return true;
}

void RectangleArea::MergeItems(SkylineItemContainer::iterator mergeTo,
                               SkylineItemContainer::iterator mergeFrom)
{
    if (mergeTo->Height == mergeFrom->Height)
    {
        // take previous start pos
        mergeTo->StartPos = std::min(mergeTo->StartPos, mergeFrom->StartPos);
        mergeTo->Width += mergeFrom->Width;
        Skyline.erase(mergeFrom);
    }
}

void RectangleArea::MergeWithNeighbours(SkylineItemContainer::iterator it)
{
    auto itNext = it + 1;
    if (itNext != Skyline.end())
    {
        MergeItems(it, itNext);
    }

    if (it != Skyline.begin())
    {
        auto itPrev = it - 1;
        MergeItems(it, itPrev);
    }
}

void RectangleArea::RaiseSkylineLevel()
{
    auto it = std::min_element(Skyline.begin(), Skyline.end(),
                               [](const auto &l, const auto &r) { return l.Height < r.Height; });

    auto itLower = Skyline.end();

    if (it != Skyline.begin())
    {
        itLower = it - 1;
    }

    auto itNext = it + 1;
    if (itNext != Skyline.end())
    {
        if (itLower == Skyline.end())
        {
            itLower = itNext;
        }
        else
        {
            if (itLower->Height > itNext->Height)
            {
                itLower = itNext;
            }
        }
    }
    it->Height = itLower->Height;

    MergeWithNeighbours(it);
}
