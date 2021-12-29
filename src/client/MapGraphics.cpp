// automatically converted

#include "MapGraphics.hpp"
#include "Client.hpp"
#include "ClientGame.hpp"
#include "GameRendering.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include <array>
#include <cmath>
#include <physfs.h>
#include <set>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

using string = std::string;

tmapgraphics mapgfx;

tgfximage *loadmaptexture(const string &texname, tgfxcolor colorkey)
{
    std::array<string, 3> s;
    std::string filename;

    tgfximage *result;
    result = nullptr;

    s[0] = moddir + "textures/" + texname;
    s[1] = string("current_map/textures/") + texname;

    if (!PHYSFS_exists((pchar)(pngoverride(s[2]))))
        s[1] = string("textures/") + texname;

    if (texname.substr(1, 6) == "edges/")
        s[2] = "textures/edges/default.bmp";
    else
        s[2] = "textures/default.bmp";

    for (const auto &v : s)
    {
        filename = pngoverride(v);

        if (PHYSFS_exists((pchar)(filename)))
        {
            result = new tgfximage(filename, colorkey);

            if (result->getimagedata(0) == nullptr)
                freeandnullptr(result);
            else
                break;
        }
    }

    if (result == nullptr)
        result = new tgfximage(32, 32);

    result->premultiply();
    return result;
}

void settexturefilter(tgfxtexture *texture, bool allowmipmaps)
{
    std::int32_t i;
    std::array<tgfxtexturefilter, 2> filters;

    if (texture == nullptr)
        return;

    i = max(1, min(2, (std::int32_t)CVar::r_texturefilter)) - 1;

    filters[0] = gfx_nearest; // "point"
    filters[1] = gfx_linear;  // "linear"

    if (allowmipmaps && CVar::r_mipmapping)
    {
        filters[0] = gfx_mipmap_nearest;
        filters[1] = gfx_mipmap_linear;
    }

    gfxtexturefilter(texture, filters[i], filters[i]);

    if (allowmipmaps && CVar::r_mipmapping)
        gfxgeneratemipmap(texture);
}

float gettexturetargetscale(tmapfile &mapfile, tgfximage *image);

static void updatescale(const tmapvertex p, tmapvertex q, const float &resolutionx,
                        tgfximage *image, const float &resolutiony, tvector2 &scale)
{
    MyFloat dx, dy;
    MyFloat du, dv;
    MyFloat d1, d2;
    MyFloat sx, sy;

    dx = q.x - p.x;
    dy = q.y - p.y;
    du = q.u - p.u;
    dv = q.v - p.v;
    d1 = sqrt(dx * dx + dy * dy);
    d2 = sqrt(du * du + dv * dv);
    sx = fabs((float)(d1 * resolutionx * du) / d2) / (du * image->width());
    sy = fabs((float)(d1 * resolutiony * dv) / d2) / (dv * image->height());

    if (!std::isnan(sx) && !std::isinf(sx))
        scale.x = max(scale.x, sx);

    if (!std::isnan(sy) && !std::isinf(sy))
        scale.y = max(scale.y, sy);
}

float gettexturetargetscale(tmapfile &mapfile, tgfximage *image)
{
    std::int32_t i;
    tvector2 scale;
    std::uint8_t alpha;
    float area, resolutionx, resolutiony;
    tmapvertex a, b, c;

    float result;
    scale.x = 0;
    scale.y = 0;

    resolutionx = (float)(renderwidth) / gamewidth;
    resolutiony = (float)(renderheight) / gameheight;

    for (i = low(mapfile.polygons); i <= high(mapfile.polygons); i++)
    {
        a = mapfile.polygons[i].vertices[0];
        b = mapfile.polygons[i].vertices[1];
        c = mapfile.polygons[i].vertices[2];

        alpha = a.color[3] | b.color[3] | c.color[3];
        area = 0.5 * fabs((a.x - c.x) * (b.y - a.y) - (a.x - b.x) * (c.y - a.y));

        if ((alpha > 0) && (area > 0))
        {
            updatescale(a, b, resolutionx, image, resolutiony, scale);
            updatescale(b, c, resolutionx, image, resolutiony, scale);
            updatescale(c, a, resolutionx, image, resolutiony, scale);
        }
    }

    result = min(MyFloat(1.f), max(scale.x, scale.y));
    return result;
}

/*$PUSH*/
/*$WARN 4056 off : Conversion between ordinals and pointers is not portable*/

const std::set<std::int32_t> backpoly = {poly_type_background, poly_type_background_transition};
const std::array<std::int32_t, 6> idx = {{0, 1, 2, 2, 3, 0}};

struct tedge
{
    pmapvertex a, b;
    std::int32_t level;
    std::int32_t textureindex;
};

void loadmapgraphics(tmapfile &mapfile, bool bgforce, tmapcolor bgcolortop, tmapcolor bgcolorbtm)
{
    std::int32_t i, j, k, n, d, level;
    std::int32_t imgwidth, imgheight;
    std::int32_t proptotal, animtotal;
    std::int32_t animindex, vbanimindex, vbpropindex, vbindex, ibindex;
    tgfxspritesheet *sheet = nullptr;
    pgfxsprite sprite;
    tgfximage *image, *edgeimage;
    std::vector<tgfxsprite *> edgesprites;
    tgfxcolor color;
    pmapprop prop;
    pmappolygon poly;
    std::int32_t edgecount;
    tedge edge;
    std::vector<tedge *> edges;
    pgfxdrawcommand cmd = nullptr;
    std::array<std::int32_t, 3> animcount;
    std::array<std::int32_t, 3> propcount;
    std::array<std::vector<pmapprop>, 3> proplist;
    std::vector<tgfxvertex> vb;
    std::vector<std::uint16_t> ib;
    float u, v, r, sx, w, h, scale;
    tvector2 a, b;
    tgfxrect bounds;
    std::vector<tgfxvertex> quad{4};
    tgfxtexture *texture;
    //  Str: string;
    bool previsanim;
    std::int32_t scenerycount;
    std::vector<std::int32_t> scenerycounters;
    std::vector<tvector2> scenerymaxsize;
    std::vector<std::int32_t> scenerysheetindex;

    tmapgraphics &mg = mapgfx;
    destroymapgraphics();

    mg.mapinfo = mapfile.mapinfo;
    mg.filename = mapfile.filename;
    mg.bgforce = bgforce;
    mg.bgforcedcolor[0] = bgcolortop;
    mg.bgforcedcolor[1] = bgcolorbtm;

    setlength(edges, 0);
    setlength(scenerysheetindex, 0);
    setlength(scenerymaxsize, 0);
    setlength(scenerycounters, 0);

    // load map textures

    setlength(mg.textures, length(mapfile.textures));
    setlength(mg.edgetextures, length(mapfile.textures));
    setlength(edgesprites, length(mapfile.textures));

    for (i = 0; i <= high(mapfile.textures); i++)
    {
        image = loadmaptexture(mapfile.textures[i], rgba(0, 0, 0, (std::uint8_t)(0)));

        imgwidth = image->width();
        imgheight = image->height();

        if (CVar::r_optimizetextures)
        {
            scale = gettexturetargetscale(mapfile, image);
            imgwidth = max(2, min(image->width(), (std::int32_t)round(scale * image->width())));
            imgheight = max(2, min(image->height(), (std::int32_t)round(scale * image->height())));
        }

        mg.textures[i] = gfxcreatetexture(imgwidth, imgheight, 4, nullptr, mapfile.mapname.c_str());

        if (mg.textures[i]->components() == 0)
        {
            imgwidth = npot(imgwidth);
            imgheight = npot(imgheight);

            gfxdeletetexture(mg.textures[i]);
            mg.textures[i] =
                gfxcreatetexture(imgwidth, imgheight, 4, nullptr, mapfile.mapname.c_str());
        }

        if ((imgwidth != image->width()) || (imgheight != image->height()))
            image->resize(imgwidth, imgheight);

        gfxupdatetexture(mg.textures[i], 0, 0, imgwidth, imgheight, image->getimagedata(0));
        gfxtexturewrap(mg.textures[i], gfx_repeat, gfx_repeat);
        settexturefilter(mg.textures[i], true);
        freeandnullptr(image);

        // load edge texture

        if (CVar::r_smoothedges)
        {
            edgeimage = loadmaptexture(string("edges/") + mapfile.textures[i], rgba(0xff00));
            image = edgeimage;

            imgwidth = npot(edgeimage->width());
            imgheight = npot(edgeimage->height());

            if ((imgwidth != edgeimage->width()) || (imgheight != edgeimage->height()))
            {
                image = new tgfximage(imgwidth, imgheight);
                image->update(0, 0, edgeimage->width(), edgeimage->height(),
                              edgeimage->getimagedata(0));
            }

            mg.edgetextures[i] =
                gfxcreatetexture(image->width(), image->height(), 4, image->getimagedata(0),
                                 mapfile.mapname + "smooth_edghes");
            settexturefilter(mg.edgetextures[i], true);

            edgesprites[i]->scale = 1;
            edgesprites[i]->texture = mg.edgetextures[i];
            edgesprites[i]->x = 0;
            edgesprites[i]->y = 0;
            edgesprites[i]->width = edgeimage->width();
            edgesprites[i]->height = edgeimage->height();
            edgesprites[i]->texcoords.left = 0;
            edgesprites[i]->texcoords.top = 0;
            edgesprites[i]->texcoords.right =
                (float)(edgeimage->width()) / mg.edgetextures[i]->width();
            edgesprites[i]->texcoords.bottom =
                (float)(edgeimage->height()) / mg.edgetextures[i]->height();

            if (image != edgeimage)
                freeandnullptr(image);

            freeandnullptr(edgeimage);
        }
    }

    // filter unused scenery and calculate max size for each one

    scenerycount = 0;

    if (length(mapfile.scenery) > 0)
    {
        n = length(mapfile.scenery);

        setlength(scenerycounters, n);
        setlength(scenerymaxsize, n);
        setlength(scenerysheetindex, n);

        fillchar(&scenerycounters[0], n * sizeof(std::int32_t), 0);
        fillchar(&scenerymaxsize[0], n * sizeof(tvector2), 0);
        fillchar(&scenerysheetindex[0], n * sizeof(std::int32_t), 0);

        for (i = low(mapfile.props); i <= high(mapfile.props); i++)
        {
            if (ispropactive(mapfile, i))
            {
                prop = &mapfile.props[i];
                scenerycounters[prop->style - 1] += 1;
                scenerymaxsize[prop->style - 1].x =
                    max(scenerymaxsize[prop->style - 1].x,
                        (MyFloat)((MyFloat)fabs(prop->scalex * prop->width) *
                                  ((MyFloat)(renderheight) / gameheight)));
                scenerymaxsize[prop->style - 1].y =
                    max(scenerymaxsize[prop->style - 1].y,
                        (MyFloat)((MyFloat)fabs(prop->scaley * prop->height) *
                                  ((MyFloat)(renderheight) / gameheight)));
            }
        }

        for (i = low(mapfile.scenery); i <= high(mapfile.scenery); i++)
        {
            if (scenerycounters[i] > 0)
            {
                scenerysheetindex[i] = scenerycount;
                scenerycount += 1;
            }
        }
    }

    // load scenery spritesheet

    if (scenerycount > 0)
    {
        color = rgba(0xff00);
        sheet = new tgfxspritesheet(scenerycount);

        for (i = low(mapfile.scenery); i <= high(mapfile.scenery); i++)
        {
            if (scenerycounters[i] > 0)
            {

                auto str = pngoverride("current_map/scenery-gfx" + mapfile.scenery[i].filename);
                if (not PHYSFS_exists(str.data()))
                {
                    str = pngoverride("scenery-gfx/" + mapfile.scenery[i].filename);
                }

                if (CVar::r_optimizetextures)
                {
                    scenerymaxsize[i].x = 1.5 * scenerymaxsize[i].x;
                    scenerymaxsize[i].y = 1.5 * scenerymaxsize[i].y;
                    sheet->addimage(str, color, scenerymaxsize[i]);
                }
                else
                {
                    sheet->addimage(str, color, 1);
                }
            }
        }

        sheet->load();

        for (i = 0; i <= scenerycount - 1; i++)
            sheet->getsprite(i)->scale = 1;

        mg.spritesheet = sheet;
        //    Str := '';

        for (i = 0; i <= sheet->texturecount() - 1; i++)
        {
            //      Str := Str + Format('%dx%d ', [Sheet.Texture[i].Width,
            //      Sheet.Texture[i].Height]);
            settexturefilter(sheet->gettexture(i), true);
        }

        //    Str[Length(Str)] := ')';
        //    GfxLog('Loaded map spritesheet (' + Str);
    }

    // calculate prop counts and filter out inactive/invalid ones

    fillchar(&propcount[0], sizeof(propcount), 0);
    fillchar(&animcount[0], sizeof(animcount), 0);

    setlength(proplist[0], length(mapfile.props));
    setlength(proplist[1], length(mapfile.props));
    setlength(proplist[2], length(mapfile.props));

    for (i = low(mapfile.props); i <= high(mapfile.props); i++)
    {
        if (ispropactive(mapfile, i))
        {
            prop = &mapfile.props[i];
            proplist[prop->level][propcount[prop->level]] = prop;
            propcount[prop->level] += 1;

            if (sheet->getsprite(scenerysheetindex[prop->style - 1])->next != nullptr)
                animcount[prop->level] += 1;
        }
    }

    proptotal = propcount[0] + propcount[1] + propcount[2];
    animtotal = animcount[0] + animcount[1] + animcount[2];

    // calculate edge count

    edgecount = 0;

    if (CVar::r_smoothedges)
    {
        setlength(edges, 3 * length(mapfile.polygons));

        for (i = low(mapfile.polygons); i <= high(mapfile.polygons); i++)
        {
            j = 2;

            for (k = 0; k < 3; k++)
            {
                edge.a = &mapfile.polygons[i].vertices[k];
                edge.b = &mapfile.polygons[i].vertices[j];

                a.x = edge.a->x + (edge.b->x - edge.a->x) * 0.5;
                a.y = edge.a->y + (edge.b->y - edge.a->y) * 0.5;

                if ((!map.collisiontestexcept(a, b, i + 1)) and
                    (min(edge.a->color[3], edge.b->color[3]) > 128))
                {
                    edges[edgecount] = &edge;
                    edges[edgecount]->level =
                        1 - ord(backpoly.contains(mapfile.polygons[i].polytype));
                    edges[edgecount]->textureindex = mapfile.polygons[i].textureindex;
                    edgecount += 1;
                }

                j = k;
            }
        }
    }

    /*-----------------------------------------------------------------------------

    Vertex buffer layout
    --------------------

    [              props               ][   edges   ][background][   polys   ]
    [    animated    ][     other      ][back][front]            [back][front]
    [back][mid][front][back][mid][front]

    Props & edges: 4 vertices per sprite (indexed by index buffer)
    Background:    6 vertices (2 triangles)
    Polys:         3 vertices per triangle

    Index buffer layout
    -------------------

    [     props      ][   edges   ]
    [back][mid][front][back][front]

    6 indices per sprite (2 triangles)

    -----------------------------------------------------------------------------*/

    // initialize vertex/index buffers + animations list

    setlength(vb, 4 * proptotal + 4 * edgecount + 6 + 3 * length(mapfile.polygons));
    setlength(ib, 6 * proptotal + 6 * edgecount);
    setlength(mg.animations, animtotal);
    setlength(mg.animationscmd, animtotal);
    setlength(mg.animduration, animtotal);
    setlength(mg.animationsbuffer, 4 * animtotal);

    // setup props

    animindex = 0;
    vbanimindex = 0;
    vbpropindex = 4 * animtotal;
    ibindex = 0;
    previsanim = false;

    // Next loop iterates all props in the order they should be drawn. The order
    // is defined by the index buffer (ib), which points to vertices in the vertex
    // buffer (vb). The reason I'm using this index buffer is so all animated
    // scenery can be kept at the beggining of the vertex buffer while preserving
    // the correct drawing order. Having all animated scenery together in the
    // vertex buffer is convenient because their vertices can be updated with one
    // float GfxUpdateBuffer() call every frame.

    for (level = low(proplist); level <= high(proplist); level++)
    {
        for (i = 0; i <= propcount[level] - 1; i++)
        {
            prop = proplist[level][i];
            sprite = sheet->getsprite(scenerysheetindex[prop->style - 1]);
            n = length(mg.props[level]);

            if (sprite->next != nullptr)
            {
                vbindex = vbanimindex;
                vbanimindex += 4;

                previsanim = true;
                setlength(mg.props[level], n + 1);
                mg.props[level][n].texture = sprite->texture;
                mg.props[level][n].offset = ibindex;
                mg.props[level][n].count = 6;

                mg.animations[animindex] = sprite;
                mg.animationscmd[animindex] = (tgfxdrawcommand *)((level << 28) | (n & 0xfffffff));
                animindex += 1;
            }
            else
            {
                vbindex = vbpropindex;
                vbpropindex += 4;

                if (previsanim || (n == 0) || (mg.props[level][n - 1].texture != sprite->texture))
                {
                    previsanim = false;
                    setlength(mg.props[level], n + 1);
                    mg.props[level][n].texture = sprite->texture;
                    mg.props[level][n].offset = ibindex;
                    mg.props[level][n].count = 6;
                }
                else
                    mg.props[level][n - 1].count += 6;
            }

            color = rgba(prop->color[0], prop->color[1], prop->color[2], prop->alpha);

            gfxspritevertices(sprite, prop->x, prop->y, prop->width, prop->height, prop->scalex,
                              prop->scaley, 0, 1, -prop->rotation, color, &vb[vbindex]);

            for (j = 0; j <= 5; j++)
                ib[ibindex + j] = vbindex + idx[j];

            ibindex += 6;
        }
    }

    // initialize animation data

    if (animtotal > 0)
        std::memcpy(&mg.animationsbuffer[0], &vb[0], 4 * animtotal * sizeof(tgfxvertex));

    for (i = low(mg.animations); i <= high(mg.animations); i++)
    {
        NotImplemented(NITag::GFX, "pointer cast to uint64");
        mg.animationscmd[i] = &mg.props[(std::uint64_t)(mg.animationscmd[i]) >> 28]
                                       [(std::uint64_t)(mg.animationscmd[i]) & 0xfffffff];

        mg.animduration[i] = 0;
        sprite = mg.animations[i];

        while (sprite != nullptr)
        {
            mg.animduration[i] += sprite->delay;
            sprite = sprite->next;
        }
    }

    // edges

    if (CVar::r_smoothedges)
    {
        n = 0;
        vbindex = 4 * proptotal;
        ibindex = 6 * proptotal;

        for (level = 0; level <= 1; level++)
        {
            for (i = 0; i <= edgecount - 1; i++)
            {
                if (edges[i]->level == level)
                {
                    if (length(mg.edges[level]) > 0)
                        cmd = &mg.edges[level][high(mg.edges[level])];

                    if ((length(mg.edges[level]) == 0) ||
                        (cmd->texture != mg.edgetextures[edges[i]->textureindex]))
                    {
                        setlength(mg.edges[level], length(mg.edges[level]) + 1);
                        cmd = &mg.edges[level][high(mg.edges[level])];
                        cmd->texture = mg.edgetextures[edges[i]->textureindex];
                        cmd->offset = ibindex;
                        cmd->count = 0;
                        sprite = edgesprites[edges[i]->textureindex];
                    }

                    a.x = edges[i]->a->x;
                    a.y = edges[i]->a->y;
                    b.x = edges[i]->b->x;
                    b.y = edges[i]->b->y;

                    r = atan2(b.y - a.y, b.x - a.x);
                    sx = sqrt(sqr(b.x - a.x) + sqr(b.y - a.y)) / 90;

                    std::memcpy(&color, &edges[i]->a->color[0], 4);
                    color.a = trunc(color.a * 0.75);

                    gfxspritevertices(sprite, a.x, a.y, sprite->width, sprite->height, sx, 1, 0,
                                      0.5, r, color, &vb[vbindex]);

                    std::memcpy(&color, &edges[i]->b->color[0], 4);
                    color.a = trunc(color.a * 0.75);

                    vb[vbindex + 1].color = color;
                    vb[vbindex + 2].color = color;

                    for (j = 0; j <= 5; j++)
                        ib[ibindex + j] = vbindex + idx[j];

                    n += 1;
                    vbindex += 4;
                    ibindex += 6;
                    cmd->count += 6;
                }
            }
        }
    }

    // background

    i = 4 * (proptotal + edgecount);
    h = gameheight;
    d = max_sector * max((float)mapfile.sectorsdivision, (float)ceil(0.5 * h / max_sector));

    if (bgforce)
    {
        std::memcpy(&mg.bgcolortop, &bgcolortop[0], 4);
        std::memcpy(&mg.bgcolorbtm, &bgcolorbtm[0], 4);
    }
    else
    {
        std::memcpy(&mg.bgcolortop, &mapfile.bgcolortop[0], 4);
        std::memcpy(&mg.bgcolorbtm, &mapfile.bgcolorbtm[0], 4);
    }

    mg.bgcolortop.a = 255;
    mg.bgcolorbtm.a = 255;

    vb[i + 0] = gfxvertex(0, -d, 0, 0, mg.bgcolortop);
    vb[i + 1] = gfxvertex(1, -d, 0, 0, mg.bgcolortop);
    vb[i + 2] = gfxvertex(1, d, 0, 0, mg.bgcolorbtm);
    vb[i + 3] = gfxvertex(1, d, 0, 0, mg.bgcolorbtm);
    vb[i + 4] = gfxvertex(0, d, 0, 0, mg.bgcolorbtm);
    vb[i + 5] = gfxvertex(0, -d, 0, 0, mg.bgcolortop);

    mg.background = i;
    mg.backgroundcount = 6;

    // polygons

    vbindex = mg.background + mg.backgroundcount;

    for (level = 0; level <= 1; level++)
    {
        for (i = low(mapfile.polygons); i <= high(mapfile.polygons); i++)
        {
            poly = &mapfile.polygons[i];

            if (level == iif(backpoly.contains(poly->polytype), 0, 1))
            {
                if (length(mg.polys[level]) > 0)
                    cmd = &mg.polys[level][high(mg.polys[level])];

                if ((length(mg.polys[level]) == 0) ||
                    (cmd->texture != mg.textures[poly->textureindex]))
                {
                    setlength(mg.polys[level], length(mg.polys[level]) + 1);
                    cmd = &mg.polys[level][high(mg.polys[level])];
                    cmd->texture = mg.textures[poly->textureindex];
                    cmd->offset = vbindex;
                    cmd->count = 0;
                }

                for (j = 0; j < 3; j++)
                {
                    vb[vbindex].x = poly->vertices[j].x;
                    vb[vbindex].y = poly->vertices[j].y;
                    vb[vbindex].u = poly->vertices[j].u;
                    vb[vbindex].v = poly->vertices[j].v;
                    std::memcpy(&vb[vbindex].color, &poly->vertices[j].color[0], 4);
                    vbindex += 1;
                }

                cmd->count += 3;
            }
        }
    }

    // create gfx buffers

    mg.vertexbuffer = gfxcreatebuffer(length(vb), true, &vb[0]);
    mg.indexbuffer = gfxcreateindexbuffer(length(ib), true, &ib[0]);

    // calculate map bounds

    if (length(mapfile.polygons) > 0)
    {
        bounds.left = mapfile.polygons[0].vertices[0].x;
        bounds.right = mapfile.polygons[0].vertices[0].x;
        bounds.top = mapfile.polygons[0].vertices[0].y;
        bounds.bottom = mapfile.polygons[0].vertices[0].y;
    }

    for (i = low(mapfile.polygons); i <= high(mapfile.polygons); i++)
    {
        for (j = 0; j < 3; j++)
        {
            bounds.left = min<float>(mapfile.polygons[i].vertices[j].x, bounds.left);
            bounds.right = max<float>(mapfile.polygons[i].vertices[j].x, bounds.right);
            bounds.top = min<float>(mapfile.polygons[i].vertices[j].y, bounds.top);
            bounds.bottom = max<float>(mapfile.polygons[i].vertices[j].y, bounds.bottom);
        }
    }

    // calculate minimap size

    if (CVar::r_scaleinterface)
        w = 260 * ((float)(renderwidth) / gamewidth);
    else
        w = round(130 / (0.5 * gamewidth / renderwidth));

    sx = (float)(w) / ((bounds.right - bounds.left) + (bounds.bottom - bounds.top));

    i = round(sx * (bounds.right - bounds.left)); // width
    j = round(sx * (bounds.bottom - bounds.top)); // height

    mg.minimap.texture = nullptr;
    mg.minimapscale = sx;
    mg.minimapoffset.x = -bounds.right;
    mg.minimapoffset.y = -bounds.bottom;

    // create minimap sprite/texture

    if (gfxframebuffersupported())
    {
        n = 4; // supersampling
        texture = gfxcreaterendertarget(npot(n * i), npot(n * j));

        gfxtarget(texture);
        gfxviewport(0, 0, n * i, n * j);
        gfxclear(0, 0, 0, 0);

        gfxtransform(gfxmat3ortho(0, 1, bounds.top, bounds.bottom));
        gfxbegin();

        quad[0] = gfxvertex(0, min((MyFloat)-d, bounds.top), 0, 0, mg.bgcolortop);
        quad[1] = gfxvertex(1, min((MyFloat)-d, bounds.top), 0, 0, mg.bgcolortop);
        quad[2] = gfxvertex(1, bounds.top, 0, 0, mg.bgcolortop);
        quad[3] = gfxvertex(0, bounds.top, 0, 0, mg.bgcolortop);

        gfxdrawquad(nullptr, quad);

        quad[0] = gfxvertex(0, bounds.bottom, 0, 0, mg.bgcolorbtm);
        quad[1] = gfxvertex(1, bounds.bottom, 0, 0, mg.bgcolorbtm);
        quad[2] = gfxvertex(1, max((MyFloat)d, bounds.bottom), 0, 0, mg.bgcolorbtm);
        quad[3] = gfxvertex(0, max((MyFloat)d, bounds.bottom), 0, 0, mg.bgcolorbtm);

        gfxdrawquad(nullptr, quad);
        gfxend();

        gfxbindtexture(nullptr);
        gfxtransform(gfxmat3ortho(0, 1, bounds.top, bounds.bottom));
        gfxdraw(mg.vertexbuffer, mg.background, mg.backgroundcount);

        gfxtransform(gfxmat3ortho(bounds.left, bounds.right, bounds.top, bounds.bottom));

        if (length(mg.polys[0]) > 0)
            gfxdraw(mg.vertexbuffer, mg.polys[0][0].offset, vbindex - mg.polys[0][0].offset);
        else if (length(mg.polys[1]) > 0)
            gfxdraw(mg.vertexbuffer, mg.polys[1][0].offset, vbindex - mg.polys[1][0].offset);

        // downsample rendered minimap texture to a new texture with original size

        sprite = &mg.minimap;
        sprite->texture = gfxcreaterendertarget(npot(i), npot(j));
        sprite->x = 0;
        sprite->y = 0;
        sprite->width = i;
        sprite->height = j;
        sprite->scale = 1;
        sprite->texcoords.left = 0;
        sprite->texcoords.top = 0;
        sprite->texcoords.right = (float)(i) / sprite->texture->width();
        sprite->texcoords.bottom = (float)(j) / sprite->texture->height();

        if (CVar::r_scaleinterface)
        {
            sprite->scale = (float)(gamewidth) / renderwidth;
            mg.minimapscale = mg.minimapscale * sprite->scale;
        }

        gfxtarget(sprite->texture);
        gfxviewport(0, 0, i, j);
        gfxclear(0, 0, 0, 0);
        gfxtransform(gfxmat3ortho(0, 1, 0, 1));

        u = (float)((n * i)) / texture->width();
        v = (float)((n * j)) / texture->height();
        color = rgba(0xffffff);

        quad[0] = gfxvertex(0, 0, 0, 0, color);
        quad[1] = gfxvertex(1, 0, u, 0, color);
        quad[2] = gfxvertex(1, 1, u, v, color);
        quad[3] = gfxvertex(0, 1, 0, v, color);

        gfxbegin();
        gfxdrawquad(texture, quad);
        gfxend();
        gfxdeletetexture(texture);

        settexturefilter(sprite->texture, false);

        gfxtarget(nullptr);
        gfxviewport(0, 0, renderwidth, renderheight);
    }
}
/*$POP*/

void updateprops(double t)
{
    std::int32_t i, vbindex, animtime, accum;
    float duration;
    tmapgraphics *mg;
    pgfxsprite sprite;

    mg = &mapgfx;
    vbindex = 0;

    for (i = low(mg->animations); i <= high(mg->animations); i++)
    {
        if (mg->animduration[i] > 0)
        {
            accum = 0;
            duration = (float)(mg->animduration[i]) / 100;
            animtime = trunc(100 * (t - duration * trunc((float)(t) / duration)));
            sprite = mg->animations[i];

            while ((accum + sprite->delay) < animtime)
            {
                if (sprite->next == nullptr)
                    break;

                accum = accum + sprite->delay;
                sprite = sprite->next;
            }

            mg->animationsbuffer[vbindex + 0].u = sprite->texcoords.left;
            mg->animationsbuffer[vbindex + 0].v = sprite->texcoords.top;
            mg->animationsbuffer[vbindex + 1].u = sprite->texcoords.right;
            mg->animationsbuffer[vbindex + 1].v = sprite->texcoords.top;
            mg->animationsbuffer[vbindex + 2].u = sprite->texcoords.right;
            mg->animationsbuffer[vbindex + 2].v = sprite->texcoords.bottom;
            mg->animationsbuffer[vbindex + 3].u = sprite->texcoords.left;
            mg->animationsbuffer[vbindex + 3].v = sprite->texcoords.bottom;

            mg->animationscmd[i]->texture = sprite->texture;
        }

        vbindex += 4;
    }

    if (vbindex > 0)
        gfxupdatebuffer(mg->vertexbuffer, 0, vbindex, &mg->animationsbuffer[0]);
}

void renderprops(std::int32_t level)
{
    std::int32_t i;

    tmapgraphics &mg = mapgfx;

    for (i = 0; i <= high(mg.props[level]); i++)
    {
        gfxbindtexture(mg.props[level][i].texture);
        gfxdraw(mg.vertexbuffer, mg.indexbuffer, mg.props[level][i].offset,
                mg.props[level][i].count);
    }
}

void renderminimap(float x, float y, std::uint8_t alpha)
{
    if (mapgfx.minimap.texture != nullptr)
        gfxdrawsprite(&mapgfx.minimap, x, y, rgba(0xffffff, alpha));
}

void worldtominimap(float x, float y, MyFloat &ox, MyFloat &oy)
{
    ox = (x - mapgfx.minimapoffset.x) * mapgfx.minimapscale;
    oy = (y - mapgfx.minimapoffset.y) * mapgfx.minimapscale;
}

void destroymapgraphics()
{
    std::int32_t i;

    if (mapgfx.vertexbuffer != nullptr)
        gfxdeletebuffer(mapgfx.vertexbuffer);

    if (mapgfx.indexbuffer != nullptr)
        gfxdeleteindexbuffer(mapgfx.indexbuffer);

    if (mapgfx.spritesheet != nullptr)
        freeandnullptr(mapgfx.spritesheet);

    if (mapgfx.minimap.texture != nullptr)
        gfxdeletetexture(mapgfx.minimap.texture);

    for (i = 0; i <= high(mapgfx.textures); i++)
    {
        if (mapgfx.textures[i] != nullptr)
            gfxdeletetexture(mapgfx.textures[i]);
    }

    for (i = 0; i <= high(mapgfx.edgetextures); i++)
    {
        if (mapgfx.edgetextures[i] != nullptr)
            gfxdeletetexture(mapgfx.edgetextures[i]);
    }

    mapgfx.filename = "";
    mapgfx.animations.clear();
    mapgfx.animationscmd.clear();
    mapgfx.animationsbuffer.clear();
    mapgfx.animduration.clear();
    mapgfx.props[0].clear();
    mapgfx.props[1].clear();
    mapgfx.props[2].clear();
    mapgfx.textures.clear();
    mapgfx.edgetextures.clear();
    mapgfx.edges[0].clear();
    mapgfx.edges[1].clear();
    mapgfx.polys[0].clear();
    mapgfx.polys[1].clear();
}
