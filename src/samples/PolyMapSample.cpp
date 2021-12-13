#include "common/Logging.hpp"
#include "common/MapFile.hpp"
#include "common/PhysFSExt.hpp"
#include "common/PolyMap.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include <SDL.h>
#include <Tracy.hpp>
#include <chrono>
#include <thread>

namespace
{
SDL_Renderer *renderer = nullptr;

void DrawTriangle(const tmappolygon &triangle, const tvector2 &offset)
{
    const auto &v1 = triangle.vertices[0];
    const auto &v2 = triangle.vertices[1];
    const auto &v3 = triangle.vertices[2];

    SDL_RenderDrawLineF(renderer, v1.x + offset.x, v1.y + offset.y, v2.x + offset.x,
                        v2.y + offset.y);
    SDL_RenderDrawLineF(renderer, v2.x + offset.x, v2.y + offset.y, v3.x + offset.x,
                        v3.y + offset.y);
    SDL_RenderDrawLineF(renderer, v3.x + offset.x, v3.y + offset.y, v1.x + offset.x,
                        v1.y + offset.y);
}

struct Color
{
    Color(std::uint8_t r, std::uint8_t g, std::uint8_t b) : r{r}, g{g}, b{b} {};
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
};

Color Red{255, 0, 0};
Color White{255, 255, 255};
Color Green{0, 255, 0};
Color Yellow{255, 255, 0};

void SetColor(const Color &color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, SDL_ALPHA_OPAQUE);
}

struct Line
{
    Line(float xbegin, float ybegin, float xend, float yend)
        : begin{xbegin, ybegin}, end{xend, yend}
    {
    }
    tvector2 begin;
    tvector2 end;
};

void DrawLine(const Line &line, const tvector2 &offset)
{
    SDL_RenderDrawLineF(renderer, line.begin.x + offset.x, line.begin.y + offset.y,
                        line.end.x + offset.x, line.end.y + offset.y);
}

} // namespace

int main(int argc, char *argv[])
{
    InitLogging();
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::abort();
    }
    SDL_Window *window = NULL;

    if (SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer) != 0)
    {
        std::abort();
    }
    SDL_bool done = SDL_FALSE;
    tvector2 lastClick{};

    {
        auto ret = PHYSFS_init(nullptr);
        Assert(0 != ret);
    }
    {
        auto ret = PHYSFS_mount("./soldat.smod", "/", false);
        Assert(0 != ret);
    }
    Polymap map;
    tmapinfo mapinfo;
    auto ret = getmapinfo("ctf_Ash", "", mapinfo);
    Assert(true == ret);
    ret = map.loadmap(mapinfo);
    Assert(true == ret);
    float scale = 1.0f;

    tvector2 offset{600.0f, 600.0f};

    Line raycastLine{-100.0f, -100.0f, 400.0f, 40.0f};

    while (!done)
    {
        SDL_Event event;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_RenderSetScale(renderer, scale, scale);

        SDL_RenderDrawPointF(renderer, lastClick.x, lastClick.y);

        SetColor(Green);
        DrawLine(raycastLine, offset);

        for (auto &s : map.sectors)
        {
            for (auto &sector : s)
            {
                for (auto i = 1U; i < sector.polys.size(); i++)
                {
                    auto polyId = sector.polys[i];
                    tvector2 out;
                    bool lineInsideTriangle =
                        map.lineinpoly(raycastLine.begin, raycastLine.end, map.polys[polyId], out);
                    Color c = lineInsideTriangle ? Yellow : White;

                    bool pointInsideTriangle = map.pointinpoly(lastClick, map.polys[polyId]);
                    SetColor(pointInsideTriangle ? Red : c);
                    DrawTriangle(map.polys[polyId], offset);
                }
            }
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.state & SDL_BUTTON_RMASK)
                {
                    offset.x += event.motion.xrel;
                    offset.y += event.motion.yrel;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == 1)
                {
                    lastClick.x = event.button.x / scale - offset.x;
                    lastClick.y = event.button.y / scale - offset.y;
                }
                break;
            case SDL_MOUSEWHEEL:
                scale += event.wheel.y / 10.f;
                break;
            default:
                break;
            }
        }
        FrameMark;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
