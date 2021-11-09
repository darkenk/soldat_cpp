#pragma once

#include <cstdint>
#include <string>
#include <vector>

typedef struct tgamebutton *pgamebutton;
struct tgamebutton
{
    bool active;
    std::int32_t x1, y1, x2, y2;
    std::string caption;
};

typedef struct tgamemenu *pgamemenu;
struct tgamemenu
{
    bool active;
    std::int32_t x, y;
    std::int32_t w, h;
    std::vector<tgamebutton> button;
};

extern std::vector<tgamemenu> gamemenu;
extern pgamemenu hoveredmenu;
extern pgamebutton hoveredbutton;
extern std::int32_t hoveredbuttonindex;
extern pgamemenu escmenu;
extern pgamemenu teammenu;
extern pgamemenu limbomenu;
extern pgamemenu kickmenu;
extern pgamemenu mapmenu;
extern std::int32_t kickmenuindex;
extern std::int32_t mapmenuindex;

void initgamemenus();
void gamemenushow(pgamemenu menu, bool show = true);
bool gamemenuaction(pgamemenu menu, std::int32_t buttonindex);
void gamemenumousemove();
bool gamemenuclick();
