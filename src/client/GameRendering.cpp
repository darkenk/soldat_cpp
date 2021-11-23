// automatically converted

#include "GameRendering.hpp"
#include "Client.hpp"
#include "ClientGame.hpp"
#include "GostekGraphics.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "MapGraphics.hpp"
#include "common/PhysFSExt.hpp"
#include "common/Util.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/TIniFile.hpp"
#include "shared/Constants.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "common/Logging.hpp"
#include "shared/Vector.hpp"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sparks.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/mechanics/Things.hpp"

#include <SDL2/SDL.h>
#include <algorithm>
#include <array>
#include <filesystem>
#include <physfs.h>
#include <set>
#include <string>

#include "shared/gfx.hpp"

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

tgamerenderingparams gamerenderingparams;
tgfxspritearray textures;

using string = std::string;

struct ttextureloaddata
{
    std::int32_t id;
    std::int32_t group;
    std::string path;
    std::uint32_t colorkey;
};

struct tinterpolationstate
{
    tvector2 camera;
    tvector2 mouse;
    std::array<PascalArray<tvector2, 1, max_sprites>, 24> spritepos;
    PascalArray<tvector2, 1, max_bullets> bulletpos;
    PascalArray<tvector2, 1, max_bullets> bulletvel;
    PascalArray<float, 1, max_bullets> bullethitmul;
    PascalArray<tvector2, 1, max_sparks> sparkpos;
    PascalArray<PascalArray<tvector2, 1, 4>, 1, max_things> thingpos;
};

struct tfontstyle
{
    tgfxfont font;
    std::int32_t tableindex;
    float size;
    float stretch;
    std::uint32_t flags;
};

// struct GfxDataEntry
//{
//    constexpr GfxDataEntry(const std::int32_t ID, const std::int32_t Group,
//                           const std::string_view Path, const std::uint32_t ColorKey)
//        : ID{ID}, Group{Group}, Path{Path}, ColorKey{ColorKey}
//    {
//    }
//    const std::int32_t ID;
//    const std::int32_t Group;
//    const std::string_view Path;
//    const std::uint32_t ColorKey;
//};

//  GFXData: array[1..GFXID_END] of TTextureLoadData = (
//    {$DEFINE GFXDATA}
//    {$INCLUDE gfx.inc}
//    {$UNDEF GFXDATA}
//  );

bool initialized;
std::string loadedinterfacename;
tgfxspritesheet *mainspritesheet = nullptr;
tgfxspritesheet *interfacespritesheet = nullptr;
std::array<tgfxfont, 2> fonts;
PascalArray<tfontstyle, 0, font_last> fontstyles;
tgfxtexture *actionsnaptexture = nullptr;
tgfxtexture *rendertarget = nullptr;
tgfxtexture *rendertargetaa = nullptr;
std::string screenshotpath;
bool screenshotasync;
PascalArray<float, 1, GFX::END> imagescale;
static TIniFile::Entries gostekdata;
static struct
{
    TIniFile::Entries root;
    TIniFile::Entries currentmod;
    TIniFile::Entries custominterface;
} scaledata;

static SDL_Surface *IconSurface = nullptr;

void loadmodinfo()
{
    std::int32_t i;
    std::unique_ptr<TIniFile> rootini;
    std::unique_ptr<TIniFile> modini;
    std::unique_ptr<TIniFile> interfaceini;
    std::string key;

    // load required ini's

    auto rootinistream = PhysFS_ReadAsStream("mod.ini");

    if (gostekdata.empty() or scaledata.root.empty())
    {
        rootini = std::make_unique<TIniFile>(std::move(rootinistream));
    }

    auto modinistream = PhysFS_ReadAsStream(moddir + "mod.ini");

    if (gostekdata.empty() or scaledata.currentmod.empty())
    {
        modini = std::make_unique<TIniFile>(std::move(modinistream));
    }

    auto interfaceinistream = PhysFS_ReadAsStream(
        (string("custom-interfaces/") + gamerenderingparams.interfacename + "/mod.ini"));

    if (interfaceinistream)
    {
        interfaceini = std::make_unique<TIniFile>(std::move(interfaceinistream));
    }

    // gostek

    if (gostekdata.empty())
    {
        rootini->ReadSectionValues("GOSTEK", gostekdata);

        TIniFile::Entries modgostek;
        modini->ReadSectionValues("GOSTEK", modgostek);
        for (const auto &entry : modgostek)
        {
            const auto &key = entry.first;
            if (!key.empty())
            {
                gostekdata[key] = entry.second;
            }
        }
        loadgostekdata(gostekdata);
    }

    // scale

    if (scaledata.root.empty())
    {
        rootini->ReadSectionValues("SCALE", scaledata.root);
    }

    if (scaledata.currentmod.empty())
    {
        modini->ReadSectionValues("SCALE", scaledata.currentmod);
    }

    if (interfaceini)
    {
        interfaceini->ReadSectionValues("SCALE", scaledata.custominterface);
    }

    // cleanup
}

float getimagescale(const std::string &imagepath)
{
    TIniFile::Entries *data;
    std::string intdir, scale, key;
    std::string path;

    intdir = string("custom-interfaces/") + lowercase(gamerenderingparams.interfacename) + '/';

    data = &scaledata.root;
    path = lowercase(imagepath);
    NotImplemented(NITag::GFX);
#if 0
    if (((moddir != "") && (lowercase(moddir) == path.substr(1, length(moddir)))))
    {
        data = &scaledata.currentmod;
        path = copy(path, length(moddir) + 1, length(path));
    }
    else if (copy(path, 1, length(intdir)) == intdir)
    {
        data = &scaledata.custominterface;
        path = copy(path, length(intdir) + 1, length(path));
    }
#endif

    std::replace(path.begin(), path.end(), '\\', '/');
    key = path;
    auto it = data->find(key);
    if (it != data->end())
    {
        scale = it->second;
    }

    if (scale == "")
    {
        NotImplemented(NITag::GFX);
#if 0
        key = stringreplace(extractfiledir(path), '\\', '/', set::of(rfreplaceall, eos));
        scale = data->at(key);
#endif

        if (scale == "")
            scale = data->at("DefaultScale");
    }

    return strtofloatdef(scale, 1);
}

void takescreenshot(string filename, bool async)
{
    screenshotpath = filename;
    screenshotasync = async;
}

string pngoverride(const std::string_view &filename)
{
    std::string f{filename};
    std::replace(f.begin(), f.end(), '\\', '/');

    return overridefileext(f, ".png");
}

string pngoverride(const std::string &filename)
{
    std::string f{filename};
    std::replace(f.begin(), f.end(), '\\', '/');
    return overridefileext(f, ".png");
}

void loadmaintextures()
{
    std::int32_t i, count;
    std::string path;
    tgfxcolor color;
    float scale;

    count = 0;

    count = std::count_if(GFXData.begin(), GFXData.end(),
                          [](const auto &d) { return d.Group != GFXG::INTERFACE; });

    mainspritesheet = new tgfxspritesheet(count);
    scale = 1.5 * renderheight / gameheight;

    for (i = 0; i < GFXData.size(); i++)
    {
        if (GFXData[i].Group != GFXG::INTERFACE)
        {
            auto id = GFXData[i].ID;
            color.r = (GFXData[i].ColorKey & 0xff) >> 0;
            color.g = (GFXData[i].ColorKey & 0xff00) >> 8;
            color.b = (GFXData[i].ColorKey & 0xff0000) >> 16;
            color.a = (GFXData[i].ColorKey & 0xff000000) >> 24;

            path = pngoverride(moddir + std::string(GFXData[i].Path));

            if (!PHYSFS_exists((pchar)(path)))
                path = pngoverride(GFXData[i].Path);

            imagescale[id] = getimagescale(path);
            Assert(!path.empty());

            if (CVar::r_optimizetextures)
                mainspritesheet->addimage(path, color, (float)(scale) / imagescale[id]);
            else
                mainspritesheet->addimage(path, color, 1);
        }
    }

    mainspritesheet->startloading();
}

void loadinterfacetextures(const std::string interfacename)
{
    const std::int32_t custom_first = GFX::INTERFACE_CURSOR;
    const std::int32_t custom_last = GFX::INTERFACE_TITLE_R;
    std::int32_t i, count;
    std::int32_t cutlength;
    std::string prefix;
    std::string path;
    tgfxcolor color;
    float scale;
    bool iscustom;

    count = 0;
    cutlength = 0;
    iscustom = !isdefaultinterface(interfacename);

    if (iscustom)
    {
        cutlength = length(std::string("interface-gfx/"));
        prefix = moddir + "custom-interfaces/" + interfacename + '/';
    }

    for (i = low(GFXData); i <= high(GFXData); i++)
    {
        if (GFXData[i].Group == GFXG::INTERFACE)
            count += 1;
    }

    if (interfacespritesheet != nullptr)
        freeandnullptr(interfacespritesheet);

    interfacespritesheet = new tgfxspritesheet(count);

    if (CVar::r_scaleinterface)
        scale = (float)(renderheight) / gameheight;
    else
        scale = 1;

    for (i = low(GFXData); i <= high(GFXData); i++)
    {
        if (GFXData[i].Group == GFXG::INTERFACE)
        {
            auto id = GFXData[i].ID;
            color.r = (GFXData[i].ColorKey & 0xff) >> 0;
            color.g = (GFXData[i].ColorKey & 0xff00) >> 8;
            color.b = (GFXData[i].ColorKey & 0xff0000) >> 16;
            color.a = (GFXData[i].ColorKey & 0xff000000) >> 24;

            if (iscustom && (i >= custom_first) && (i <= custom_last))
            {
                path = prefix + std::string(GFXData[i].Path.data() + cutlength + 1);
                path = pngoverride(path);

                if (!PHYSFS_exists((pchar)(path)))
                    path = pngoverride(GFXData[i].Path);
            }
            else
            {
                path = pngoverride(moddir + std::string(GFXData[i].Path));

                if (!PHYSFS_exists((pchar)(path)))
                    path = pngoverride(GFXData[i].Path);
            }

            imagescale[id] = getimagescale(path);

            if (CVar::r_optimizetextures)
                interfacespritesheet->addimage(path, color, (float)(scale) / imagescale[id]);
            else
                interfacespritesheet->addimage(path, color, 1);
        }
    }

    interfacespritesheet->startloading();
}

void loadinterface()
{
    if (loadinterfacedata(gamerenderingparams.interfacename))
        loadinterfacetextures(gamerenderingparams.interfacename);
    else
        loadinterfacetextures("");

    loadedinterfacename = gamerenderingparams.interfacename;
}

string getfontpath(string fontfile)
{
    std::string result;
    auto p = std::filesystem::path(basedirectory + fontfile);
    if (std::filesystem::exists(p) && !std::filesystem::is_directory(p))
    {
        result = basedirectory + fontfile;
    }
    return result;
}

string getfontpath(string fallback, string &fontfile)
{

    std::string result = "";

    if (fontfile != "")
        result = getfontpath(fontfile);

    if (result == "")
    {
        result = getfontpath(fontfile);
    }

    if (result == "")
    {
        fontfile = fallback;
        result = getfontpath(fontfile);
    }
    return result;
}

void loadfonts()
{
    float s;
    std::int32_t i, w, h;
    std::array<string, 2> fontfile, fontpath;

    fontfile[0] = CVar::font_1_filename;
    fontfile[1] = CVar::font_2_filename;

    fontpath[0] = getfontpath(default_font, fontfile[0]);
    fontpath[1] = getfontpath(default_font, fontfile[1]);

    if ((fontpath[0] == "") || (fontpath[1] == ""))
    {
        showmessage(
            ("One of the fonts cannot be found. Please check your installation directory."));
        shutdown();
    }

    w = renderwidth;
    h = renderheight;
    s = iif(CVar::r_scaleinterface, (float)(renderheight) / gameheight, 1.0f);

    fonts[0] = gfxcreatefont(fontpath[0], npot(w / 2), npot(h / 2));
    fonts[1] = gfxcreatefont(fontpath[1], npot(w / 3), npot(h / 3));

    fontstyles[font_small].font = fonts[1];
    fontstyles[font_small].size = s * CVar::font_consolesize;
    fontstyles[font_small].stretch = (float)(CVar::font_2_scale) / 100;
    fontstyles[font_small].flags = 0;

    // bold not supported for now so same as FONT_SMALL
    fontstyles[font_small_bold].font = fonts[1];
    fontstyles[font_small_bold].size = s * CVar::font_consolesize;
    fontstyles[font_small_bold].stretch = (float)(CVar::font_2_scale) / 100;
    fontstyles[font_small_bold].flags = 0;

    fontstyles[font_smallest].font = fonts[1];
    fontstyles[font_smallest].size = s * CVar::font_consolesmallsize;
    fontstyles[font_smallest].stretch = (float)(CVar::font_2_scale) / 100;
    fontstyles[font_smallest].flags = 0;

    fontstyles[font_big].font = fonts[1];
    fontstyles[font_big].size = CVar::font_bigsize;
    fontstyles[font_big].stretch = (float)(CVar::font_1_scale) / 100;
    fontstyles[font_big].flags = 0;

    fontstyles[font_menu].font = fonts[1];
    fontstyles[font_menu].size = s * CVar::font_menusize;
    fontstyles[font_menu].stretch = (float)(CVar::font_1_scale) / 100;
    fontstyles[font_menu].flags = 0;

    fontstyles[font_weapons_menu].font = fonts[1];
    fontstyles[font_weapons_menu].size = s * CVar::font_weaponmenusize;
    ;
    fontstyles[font_weapons_menu].stretch = (float)(CVar::font_2_scale) / 100;
    fontstyles[font_weapons_menu].flags = 0;

    fontstyles[font_world].font = fonts[1];
    fontstyles[font_world].size = 128 * ((float)(renderheight) / gameheight);
    fontstyles[font_world].stretch = (float)(CVar::font_1_scale) / 100;
    fontstyles[font_world].flags = 0;

    for (i = 0; i < fontstyles.size(); i++)
    {
        if (fontstyles[i].size < 10)
            fontstyles[i].flags = gfx_monochrome;

        fontstyles[i].tableindex = gfxsetfont(fontstyles[i].font, fontstyles[i].size,
                                              fontstyles[i].flags, fontstyles[i].stretch);
    }
}

bool initgamegraphics()
{
    std::uint32_t windowflags;
    SDL_RWops *iconfile;
    PhysFS_Buffer filebuffer;

    bool result;
    result = true;

    if (initialized)
    {
        if (gamerenderingparams.interfacename != loadedinterfacename)
        {
            scaledata.custominterface.clear();
            loadmodinfo();
            loadinterface();
        }

        return result;
    }

    windowflags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

    if (CVar::r_fullscreen == 2)
        windowflags = windowflags | SDL_WINDOW_FULLSCREEN_DESKTOP;
    else if (CVar::r_fullscreen == 1)
        windowflags = windowflags | SDL_WINDOW_FULLSCREEN;
    else
        windowflags = windowflags;

    // TODO: set opengl 4.3 profile for better debugging
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    if (CVar::r_msaa > 0)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, CVar::r_msaa);
    }

    gamewindow = SDL_CreateWindow("Soldat", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  windowwidth, windowheight, windowflags);

    filebuffer = PhysFS_readBuffer("icon.bmp");

    iconfile = SDL_RWFromMem(&filebuffer[0], length(filebuffer));

    IconSurface = SDL_LoadBMP_RW(iconfile, 1);

    SDL_SetWindowIcon(gamewindow, IconSurface);

    if (gamewindow == nullptr)
    {
        showmessage("Error creating sdl2 window");
        result = false;
        return result;
    }

    if (!gfxinitcontext(gamewindow, CVar::r_dithering, CVar::r_compatibility))
    {
        result = false;
        return result;
    }

    startinput();

    if (SDL_GL_SetSwapInterval(CVar::r_swapeffect) == -1)
        gfxlog(string("Error while setting SDL_GL_SetSwapInterval:") + SDL_GetError());

    gfxviewport(0, 0, windowwidth, windowheight);

    textures = new pgfxsprite[GFX::END + 1];
    loadmodinfo();
    loadmaintextures();
    loadinterface();
    loadfonts();

    map.loadgraphics = &loadmapgraphics;
    if (!gfxframebuffersupported())
        CVar::cl_actionsnap = false;

    if (CVar::cl_actionsnap)
        actionsnaptexture = gfxcreaterendertarget(renderwidth, renderheight, 4, true);

    if (gfxframebuffersupported())
    {
        if ((windowwidth != renderwidth) || (windowheight != renderheight))
        {
            rendertarget = gfxcreaterendertarget(renderwidth, renderheight, 4, true);

            if (rendertarget->samples() > 0)
            {
                rendertargetaa = gfxcreaterendertarget(renderwidth, renderheight, 4, false);

                if (CVar::r_resizefilter >= 2)
                    gfxtexturefilter(rendertargetaa, gfx_linear, gfx_linear);
                else
                    gfxtexturefilter(rendertargetaa, gfx_nearest, gfx_nearest);
            }
            else
            {
                if (CVar::r_resizefilter >= 2)
                    gfxtexturefilter(rendertarget, gfx_linear, gfx_linear);
                else
                    gfxtexturefilter(rendertarget, gfx_nearest, gfx_nearest);
            }
        }
    }

    initialized = true;
    return result;
}

void reloadgraphics()
{
    tmapfile mapfile;
    tmapinfo mapinfo;
    bool bgforce;
    std::array<tmapcolor, 2> color;
    tmapgraphics *mg;

    mg = &mapgfx;
    mapinfo = mg->mapinfo;
    bgforce = mg->bgforce;
    color[0] = mg->bgforcedcolor[0];
    color[1] = mg->bgforcedcolor[1];

    freeandnullptr(mainspritesheet);
    freeandnullptr(interfacespritesheet);
    destroymapgraphics();

    gostekdata.clear();
    scaledata.root.clear();
    scaledata.currentmod.clear();
    scaledata.custominterface.clear();

    loadmodinfo();
    loadmaintextures();
    loadinterface();
    dotextureloading(true);

    loadmapfile(mapinfo, mapfile);
    loadmapgraphics(mapfile, bgforce, color[0], color[1]);
}

void destroygamegraphics()
{
    std::int32_t i;

    if (initialized == false)
        return;

    SDL_SetWindowIcon(gamewindow, nullptr);
    SDL_FreeSurface(IconSurface);
    IconSurface = nullptr;

    freeandnullptr(mainspritesheet);
    freeandnullptr(interfacespritesheet);

    for (i = low(fonts); i <= high(fonts); i++)
        gfxdeletefont(fonts[i]);

    if (actionsnaptexture != nullptr)
        gfxdeletetexture(actionsnaptexture);

    if (rendertarget != nullptr)
        gfxdeletetexture(rendertarget);

    if (rendertargetaa != nullptr)
        gfxdeletetexture(rendertargetaa);

    destroymapgraphics();
    gfxdestroycontext();

    initialized = false;
}

// float lerp(float a, float b, float x)
//{

//    return a + (b - a) * x;
//}

tvector2 lerp(tvector2 a, tvector2 b, float x)
{

    tvector2 lerp_result;
    lerp_result.x = a.x + (b.x - a.x) * x;
    lerp_result.y = a.y + (b.y - a.y) * x;
    return lerp_result;
}

void interpolatestate(float p, tinterpolationstate &s, bool paused)
{
    const std::set<std::int32_t> kit_styles = {
        object_medical_kit, object_grenade_kit, object_flamer_kit, object_predator_kit,
        object_vest_kit,    object_berserk_kit, object_cluster_kit};
    std::int32_t i, j;
    particlesystem *sk;
    tgun *gun;

    s.camera.x = camerax;
    s.camera.y = cameray;
    s.mouse.x = mx;
    s.mouse.y = my;

    camerax = lerp(cameraprev.x, camerax, p);
    cameray = lerp(cameraprev.y, cameray, p);
    mx = lerp(mouseprev.x, mx, p);
    my = lerp(mouseprev.y, my, p);

    if (paused)
        p = 1.0;

    for (i = 1; i <= max_sprites; i++)
    {
        if (sprite[i].active)
        {
            sk = &sprite[i].skeleton;
            gun = &sprite[i].weapon;
            std::memcpy(&s.spritepos[i][1], &sk->pos[1], sizeof(tvector2) * length(s.spritepos[i]));

            for (j = low(s.spritepos[i]); j <= high(s.spritepos[i]); j++)
                sk->pos[j] = lerp(sk->oldpos[j], sk->pos[j], p);

            gun->reloadtimefloat = lerp(gun->reloadtimeprev, gun->reloadtimecount, p);
            gun->fireintervalfloat = lerp(gun->fireintervalprev, gun->fireintervalcount, p);
            sprite[i].jetscountfloat = lerp(sprite[i].jetscountprev, sprite[i].jetscount, p);
        }
    }

    for (i = 1; i <= max_bullets; i++)
    {
        if (bullet[i].active or (bullet[i].pingadd > 0))
        {
            j = bullet[i].num;

            s.bulletpos[i] = bulletparts.pos[j];
            s.bulletvel[i] = bulletparts.velocity[j];
            s.bullethitmul[i] = bullet[i].hitmultiply;

            bulletparts.pos[j] = lerp(bulletparts.oldpos[j], bulletparts.pos[j], p);
            bulletparts.velocity[j] = lerp(bullet[i].velocityprev, bulletparts.velocity[j], p);
            bullet[i].hitmultiply = lerp(bullet[i].hitmultiplyprev, bullet[i].hitmultiply, p);
            bullet[i].timeoutfloat = lerp(bullet[i].timeoutprev, bullet[i].timeout, p);
        }
    }

    for (i = 1; i <= max_sparks; i++)
    {
        if (spark[i].active)
        {
            j = spark[i].num;
            s.sparkpos[i] = sparkparts.pos[j];
            sparkparts.pos[j] = lerp(sparkparts.oldpos[j], sparkparts.pos[j], p);
            spark[i].lifefloat = lerp(spark[i].lifeprev, spark[i].life, p);
        }
    }

    for (i = 1; i <= max_things; i++)
    {
        if (thing[i].active)
        {
            sk = &thing[i].skeleton;
            std::memcpy(&s.thingpos[i][1], &sk->pos[1], sizeof(tvector2) * length(s.thingpos[i]));

            for (j = low(s.thingpos[i]); j <= high(s.thingpos[i]); j++)
                sk->pos[j] = lerp(sk->oldpos[j], sk->pos[j], p);

            if (kit_styles.contains(thing[i].style))
                sk->satisfyconstraints();
        }
    }
}

void restorestate(tinterpolationstate &s)
{
    std::int32_t i;

    camerax = s.camera.x;
    cameray = s.camera.y;
    mx = s.mouse.x;
    my = s.mouse.y;

    for (i = 1; i <= max_sprites; i++)
    {
        if (sprite[i].active)
        {
            std::memcpy(&sprite[i].skeleton.pos[1], &s.spritepos[i][1],
                        sizeof(tvector2) * length(s.spritepos[i]));
        }
    }

    for (i = 1; i <= max_bullets; i++)
    {
        if (bullet[i].active or (bullet[i].pingadd > 0))
        {
            bulletparts.pos[bullet[i].num] = s.bulletpos[i];
            bulletparts.velocity[bullet[i].num] = s.bulletvel[i];
            bullet[i].hitmultiply = s.bullethitmul[i];
        }
    }

    for (i = 1; i <= max_sparks; i++)
    {
        if (spark[i].active)
            sparkparts.pos[spark[i].num] = s.sparkpos[i];
    }

    for (i = 1; i <= max_things; i++)
    {
        if (thing[i].active)
        {
            std::memcpy(&thing[i].skeleton.pos[1], &s.thingpos[i][1],
                        sizeof(tvector2) * length(s.thingpos[i]));
        }
    }
}

void renderframe(double timeelapsed, double framepercent, bool paused)
{
    tmapgraphics *mg;
    std::int32_t i;
    float dx, dy;
    float w, h, s, u, v;
    tinterpolationstate interpolationstate;
    bool grabactionsnap;
    trect rc;
    tgfxtexture *rt;

    mg = &mapgfx;

    // graphics might be destroyed before end of game loop
    if (mg->vertexbuffer == nullptr)
        return;

    if (rendertarget != nullptr)
    {
        gfxtarget(rendertarget);
        gfxviewport(0, 0, renderwidth, renderheight);
    }

    if (screenshotpath != "")
    {
        gfxsavescreen(screenshotpath, 0, 0, renderwidth, renderheight, screenshotasync);
        screenshotpath = "";
    }

    if (showscreen && actionsnaptaken)
    {
        rc = trect(0, renderheight, renderwidth, 0);
        gfxblit(actionsnaptexture, rendertarget, rc, rc, gfx_nearest);
        gfxtarget(rendertarget);

        w = renderwidth;
        h = renderheight;

        if (CVar::r_scaleinterface)
        {
            w = gamewidth;
            h = gameheight;
        }

        gfxbegin();
        gfxtransform(gfxmat3ortho(0, w, 0, h));
        gfxtextpixelratio(vector2((float)(w) / renderwidth, (float)(h) / renderheight));
        renderactionsnaptext(timeelapsed);
        gfxend();
    }
    else
    {
        grabactionsnap = false;

        if ((CVar::cl_actionsnap) && (capscreen == 0))
        {
            capscreen = 255;
            grabactionsnap = true;
            actionsnaptaken = true;
            gfxtarget(actionsnaptexture);
        }
        else if (capscreen != 255)
        {
            capscreen -= 1;
        }

        interpolatestate(framepercent, interpolationstate, paused);

        w = exp(CVar::r_zoom) * gamewidth;
        h = exp(CVar::r_zoom) * gameheight;

        dx = camerax - (float)(w) / 2;
        dy = cameray - (float)(h) / 2;

        if (cameray > 0)
            gfxclear(mapgfx.bgcolorbtm);
        else
            gfxclear(mapgfx.bgcolortop);

        if (CVar::r_animations)
            updateprops(timeelapsed);

        gfxtransform(gfxmat3ortho(0, 1, dy, h + dy));
        gfxbindtexture(nullptr);
        gfxdraw(mg->vertexbuffer, mg->background, mg->backgroundcount);

        gfxtransform(gfxmat3ortho(dx, w + dx, dy, h + dy));

        if (CVar::r_smoothedges && (length(mg->edges[0]) > 0))
            gfxdraw(mg->vertexbuffer, mg->indexbuffer, &mg->edges[0][0], length(mg->edges[0]));

        if (length(mg->polys[0]) > 0)
            gfxdraw(mg->vertexbuffer, &mg->polys[0][0], length(mg->polys[0]));

        gfxsetmipmapbias(CVar::r_mipmapbias);

        if (CVar::r_renderbackground)
            renderprops(0);

        gfxbegin();

        for (i = 1; i <= max_bullets; i++)
            if (bullet[i].active or (bullet[i].pingadd > 0))
                bullet[i].render(timeelapsed);

        for (i = 1; i <= max_sprites; i++)
            if (sprite[i].active)
                rendergostek(sprite[i]);

        for (i = 1; i <= max_things; i++)
            if (thing[i].active)
                thing[i].render(timeelapsed);

        for (i = 1; i <= max_sparks; i++)
            if (spark[i].active)
                spark[i].render();

        gfxend();
        renderprops(1);
        gfxbegin();

        for (i = 1; i <= max_things; i++)
            if (thing[i].active)
                thing[i].polygonsrender();

        gfxend();
        gfxsetmipmapbias(0);

        if (CVar::r_smoothedges && (length(mg->edges[1]) > 0))
            gfxdraw(mg->vertexbuffer, mg->indexbuffer, &mg->edges[1][0], length(mg->edges[1]));

        if (length(mg->polys[1]) > 0)
            gfxdraw(mg->vertexbuffer, &mg->polys[1][0], length(mg->polys[1]));

        gfxsetmipmapbias(CVar::r_mipmapbias);
        renderprops(2);
        gfxsetmipmapbias(0);

        if (!CVar::r_scaleinterface)
        {
            w = renderwidth;
            h = renderheight;
        }

        if (grabactionsnap)
        {
            rc = trect(0, renderheight, renderwidth, 0);
            gfxblit(actionsnaptexture, rendertarget, rc, rc, gfx_nearest);
            gfxtarget(rendertarget);
        }

        if (CVar::r_renderui)
        {
            gfxbegin();
            gfxtransform(gfxmat3ortho(0, w, 0, h));
            renderinterface(timeelapsed, w, h);
            gfxend();
        }

        restorestate(interpolationstate);
    }

    if (rendertarget != nullptr)
    {
        rt = rendertarget;

        if (rendertargetaa != nullptr)
        {
            rc = trect(0, renderheight, renderwidth, 0);
            gfxblit(rendertarget, rendertargetaa, rc, rc, gfx_nearest);
            rt = rendertargetaa;
        }

        if (((float)(screenwidth) / screenheight) >= ((float)(renderwidth) / renderheight))
        {
            w = screenheight * ((float)(renderwidth) / renderheight);
            h = screenheight;
        }
        else
        {
            w = screenwidth;
            h = screenwidth * ((float)(renderheight) / renderwidth);
        }

        dx = floor(0.5 * (screenwidth - w));
        dy = floor(0.5 * (screenheight - h));

        if (screenwidth != windowwidth)
        {
            s = (float)(windowwidth) / screenwidth;
            w = w * s;
            dx = (dx - (float)(screenwidth) / 2) * s + (float)(windowwidth) / 2;
        }

        if (screenheight != windowheight)
        {
            s = (float)(windowheight) / screenheight;
            h = h * s;
            dy = (dy - (float)(screenheight) / 2) * s + (float)(windowheight) / 2;
        }

        gfxtarget(nullptr);
        gfxviewport(0, 0, windowwidth, windowheight);
        gfxclear(rgba(0));
        gfxtransform(gfxmat3ortho(0, windowwidth, 0, windowheight));

        u = (float)(renderwidth) / rendertarget->width();
        v = (float)(renderheight) / rendertarget->height();

        gfxbegin();
        gfxdrawquad(rt, gfxvertex(dx + 0, dy + 0, 0, v, rgba(0xffffff)),
                    gfxvertex(dx + w, dy + 0, u, v, rgba(0xffffff)),
                    gfxvertex(dx + w, dy + h, u, 0, rgba(0xffffff)),
                    gfxvertex(dx + 0, dy + h, 0, 0, rgba(0xffffff)));
        gfxend();
    }

    gfxpresent(CVar::r_glfinish);
}

void rendergameinfo(const std::string &textstring)
{
    tgfxrect rc;

    gfxtarget(nullptr);
    gfxviewport(0, 0, windowwidth, windowheight);
    gfxtransform(gfxmat3ortho(0, windowwidth, 0, windowheight));
    gfxclear(49, 61, 79, 255);
    setfontstyle(font_menu);
    gfxtextcolor(rgba(0xffffff));
    gfxtextshadow(1, 1, rgba(0));
    gfxtextpixelratio(vector2(1, 1));
    rc = gfxtextmetrics(textstring);
    gfxbegin();
    gfxdrawtext((float)((windowwidth - rc.width())) / 2, (float)((windowheight - rc.height())) / 2);
    setfontstyle(font_small);
    rc = gfxtextmetrics(("Press ESC to quit the game"));
    gfxdrawtext((float)((windowwidth - rc.width())) / 2,
                ((float)((windowheight - rc.height())) / 2) + 100);
    gfxend();
    gfxpresent(true);
}

template <typename T>
bool arraycontains(const T &list, std::int32_t x)
{
    std::int32_t i;

    bool result = false;

    for (i = low(list); i <= high(list); i++)
    {
        if (list[i] == x)
        {
            result = true;
            return result;
        }
    }
    return result;
}

bool getsizeconstraint(std::int32_t id, std::int32_t &w, std::int32_t &h)
{
    const std::array<std::int32_t, 35> weapons_list = {
        {GFX::WEAPONS_AK74,      GFX::WEAPONS_AK74_2,      GFX::WEAPONS_AK74_FIRE,
         GFX::WEAPONS_MINIMI,    GFX::WEAPONS_MINIMI_2,    GFX::WEAPONS_RUGER,
         GFX::WEAPONS_RUGER_2,   GFX::WEAPONS_MP5,         GFX::WEAPONS_MP5_2,
         GFX::WEAPONS_SPAS,      GFX::WEAPONS_SPAS_2,      GFX::WEAPONS_M79,
         GFX::WEAPONS_M79_2,     GFX::WEAPONS_DEAGLES,     GFX::WEAPONS_DEAGLES_2,
         GFX::WEAPONS_N_DEAGLES, GFX::WEAPONS_N_DEAGLES_2, GFX::WEAPONS_STEYR,
         GFX::WEAPONS_STEYR_2,   GFX::WEAPONS_BARRETT,     GFX::WEAPONS_BARRETT_2,
         GFX::WEAPONS_MINIGUN,   GFX::WEAPONS_MINIGUN_2,   GFX::WEAPONS_SOCOM,
         GFX::WEAPONS_SOCOM_2,   GFX::WEAPONS_N_SOCOM,     GFX::WEAPONS_N_SOCOM_2,
         GFX::WEAPONS_BOW,       GFX::WEAPONS_BOW_S,       GFX::WEAPONS_FLAMER,
         GFX::WEAPONS_FLAMER_2,  GFX::WEAPONS_KNIFE,       GFX::WEAPONS_KNIFE2,
         GFX::WEAPONS_CHAINSAW,  GFX::WEAPONS_CHAINSAW2}};

    bool result = false;

    if ((id >= GFX::GOSTEK_STOPA) && (id <= GFX::GOSTEK_TEAM2_LECISTOPA2))
    {
        w = gos_restrict_width;
        h = gos_restrict_height;
        result = true;
    }
    else if (arraycontains(weapons_list, id))
    {
        w = wep_restrict_width;
        h = wep_restrict_height;
        result = true;
    }
    return result;
}

bool dotextureloading(bool finishloading)
{
    std::int32_t i, j;
    std::int32_t w = 0;
    std::int32_t h = 0;
    bool mainloading, interfaceloading;
    std::string s;

    bool dotextureloading_result = true; // return true when not loading

    if ((mainspritesheet == nullptr) || (interfacespritesheet == nullptr))
        return dotextureloading_result;

    mainloading = mainspritesheet->loading();
    interfaceloading = interfacespritesheet->loading();

    if (!(mainloading || interfaceloading))
        return dotextureloading_result;

    if (finishloading)
    {
        mainspritesheet->finishloading();
        interfacespritesheet->finishloading();
    }
    else
    {
        if (mainspritesheet->loading())
            mainspritesheet->continueloading();
        else if (interfacespritesheet->loading())
            interfacespritesheet->continueloading();
    }

    if (mainspritesheet->loading() != mainloading)
    {
        j = 0;

        for (i = low(GFXData); i <= high(GFXData); i++)
        {
            if (GFXData[i].Group != GFXG::INTERFACE)
            {
                auto id = GFXData[i].ID;
                textures[id] = mainspritesheet->getsprite(j);
                textures[id]->scale = textures[id]->scale * ((float)(1) / imagescale[id]);

                if (getsizeconstraint(i, w, h))
                {
                    if (((textures[id]->width * textures[id]->scale) > w) ||
                        ((textures[id]->height * textures[id]->scale) > h))
                    {
                        if (((float)(textures[id]->width) / textures[id]->height) >
                            ((float)(w) / h))
                            textures[id]->scale = (float)(w) / textures[id]->width;
                        else
                            textures[id]->scale = (float)(h) / textures[id]->height;
                    }
                }

                j += 1;
            }
        }

        s = "";

        for (i = 0; i <= mainspritesheet->texturecount() - 1; i++)
        {
            s = s + fmt::format("{}x{} ", mainspritesheet->gettexture(i)->width(),
                                mainspritesheet->gettexture(i)->height());

            settexturefilter(mainspritesheet->gettexture(i), true);
        }

        s[length(s)] = ')';
        gfxlog(string("Loaded main spritesheet (") + s);

        applygostekconstraints();
    }

    if (interfacespritesheet->loading() != interfaceloading)
    {
        j = 0;

        for (i = 0; i < GFXData.size(); i++)
        {
            if (GFXData[i].Group == GFXG::INTERFACE)
            {
                auto id = GFXData[i].ID;
                textures[id] = interfacespritesheet->getsprite(j);
                textures[id]->scale = textures[id]->scale * ((float)(1) / imagescale[id]);
                j += 1;
            }
        }

        s = "";

        for (i = 0; i <= interfacespritesheet->texturecount() - 1; i++)
        {
            s = s + fmt::format("{}x{} ", interfacespritesheet->gettexture(i)->width(),
                                interfacespritesheet->gettexture(i)->height());

            settexturefilter(interfacespritesheet->gettexture(i), false);
        }

        s[length(s)] = ')';
        gfxlog(string("Loaded interface spritesheet (") + s);
    }

    dotextureloading_result = false;

    if (!(mainspritesheet->loading() || interfacespritesheet->loading()))
        dotextureloading_result = true;
    return dotextureloading_result;
}

void setfontstyle(std::int32_t style)
{
    gfxsetfonttable(fontstyles[style].font, fontstyles[style].tableindex);
}

void setfontstyle(std::int32_t style, float scale)
{
    gfxsetfont(fontstyles[style].font, scale * fontstyles[style].size, fontstyles[style].flags,
               fontstyles[style].stretch);
}

std::int32_t fontstylesize(std::int32_t style)
{
    return fontstyles[style].size;
}

void gfxlogcallback(const std::string &s)
{
    LogDebugG("[GFX] {}", s);
}

// initialization
//  GfxLog := @GfxLogCallback;
