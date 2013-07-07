/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    SUBSYSTEM:    Stars.exe
    FILE:         StarSystem.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Various Heavenly Bodies
*/

#include "MemDebug.h"
#include "StarSystem.h"
#include "Galaxy.h"
#include "Sky.h"
#include "Starshatter.h"
#include "TerrainRegion.h"
#include "TerrainHaze.h"
#include "Weather.h"

#include "Game.h"
#include "Sound.h"
#include "Solid.h"
#include "Light.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Scene.h"
#include "ParseUtil.h"

const double epoch = 0.5e9;
double StarSystem::stardate = 0;

// +====================================================================+

static double base_time = 0;
static WORD   oldcw = 0;
static WORD   fpcw  = 0;

void FPU2Extended()
{
    _asm { fstcw oldcw }
    fpcw = oldcw | 0x0300;  // set 80-bit precision mode
    _asm { fldcw fpcw }
}

void FPURestore()
{
    // well, I don't actually WANT single-precision mode
    // so I think I'll just ignore this...

    //_asm { fldcw oldcw }
}

void StarSystem::SetBaseTime(double t, bool absolute)
{
    FPU2Extended();

    if (absolute) {
        base_time = t;
        CalcStardate();
    }

    else if (t > 0) {
        if (t > epoch) t -= epoch;
        base_time = t;
        CalcStardate();
    }
}

double StarSystem::GetBaseTime()
{
    return base_time;
}

void StarSystem::CalcStardate()
{
    if (base_time < 1) {
        time_t clock_seconds;
        time(&clock_seconds);

        base_time = clock_seconds;

        while (base_time < 0)
        base_time += epoch;
    }

    FPU2Extended();

    double gtime = (double) Game::GameTime() / 1000.0;
    double sdate = gtime + base_time + epoch;

    stardate = sdate;

    FPURestore();
}

static const double GRAV    = 6.673e-11;
static const int    NAMELEN = 64;

// +====================================================================+

StarSystem::StarSystem(const char* sys_name, Point l, int iff, int s)
: name(sys_name), affiliation(iff), sky_stars(0), sky_dust(0), loc(l), seq(s),
active_region(0), instantiated(false), ambient(0,0,0),
sun_color(255,255,255), sun_scale(1), point_stars(0), poly_stars(0), 
nebula(0), haze(0)
{
    center = new(__FILE__,__LINE__) Orbital(this, "CG", Orbital::NOTHING, 1.0e35f, 0.0f, 0.0f, 0);
    radius = 0.0f;
}

// +--------------------------------------------------------------------+

StarSystem::~StarSystem()
{
    Print("   Destroying Star System %s\n", (const char*) name);

    if (instantiated) {
        Deactivate();
        Destroy();
    }

    bodies.destroy();
    regions.destroy();
    all_regions.clear(); // do not destroy these!

    delete center;
}

// +--------------------------------------------------------------------+

static OrbitalBody*   primary_star   = 0;
static OrbitalBody*   primary_planet = 0;
static OrbitalBody*   primary_moon   = 0;

void
StarSystem::Load()
{
    CalcStardate();
    active_region = 0;

    BYTE*       block  = 0;
    DataLoader* loader = DataLoader::GetLoader();
    datapath = loader->GetDataPath();

    sprintf_s(filename, "%s/%s.def", (const char*) name, (const char*) name);

    Print("Loading StarSystem: %s\n", filename);
    loader->LoadBuffer(filename, block, true);

    if (!block) {
        Print("ERROR: invalid star system file '%s'\n", filename);
        exit(-2);
        return;
    }

    Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

    Term*  term = parser.ParseTerm();

    if (!term) {
        Print("ERROR: could not parse '%s'\n", filename);
        exit(-3);
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "STARSYSTEM") {
            Print("ERROR: invalid star system file '%s'\n", filename);
            term->print(10);
            exit(-4);
            return;
        }
    }

    // parse the system:
    do {
        delete term;
        term = parser.ParseTerm();
        
        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value() == "name") {
                    char namebuf[NAMELEN];
                    namebuf[0] = 0;
                    GetDefText(namebuf, def, filename);

                    if (namebuf[0])
                    name = namebuf;
                }

                else if (def->name()->value() == "sky") {
                    if (!def->term() || !def->term()->isStruct()) {
                        Print("WARNING: sky struct missing in '%s'\n", filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();

                        char  imgname[NAMELEN];
                        char  magname[NAMELEN];
                        char  hazname[NAMELEN];

                        imgname[0] = 0;
                        magname[0] = 0;
                        hazname[0] = 0;
                        
                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (pdef) {
                                if (pdef->name()->value() == "poly_stars")
                                GetDefText(imgname, pdef, filename);

                                else if (pdef->name()->value() == "nebula")
                                GetDefText(magname, pdef, filename);

                                else if (pdef->name()->value() == "haze")
                                GetDefText(hazname, pdef, filename);
                            }
                        }

                        if (imgname[0])
                        sky_poly_stars = imgname;

                        if (magname[0])
                        sky_nebula = magname;

                        if (hazname[0])
                        sky_haze = hazname;
                    }
                }

                else if (def->name()->value() == "stars") {
                    GetDefNumber(sky_stars, def, filename);
                }

                else if (def->name()->value() == "ambient") {
                    Vec3 a;
                    GetDefVec(a, def, filename);

                    ambient = Color((BYTE) a.x, (BYTE) a.y, (BYTE) a.z) * 2.5;
                }

                else if (def->name()->value() == "dust") {
                    GetDefNumber(sky_dust, def, filename);
                }

                else if (def->name()->value() == "star") {
                    if (!def->term() || !def->term()->isStruct()) {
                        Print("WARNING: star struct missing in '%s'\n", filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();
                        ParseStar(val);
                    }
                }

                else if (def->name()->value() == "planet") {
                    if (!def->term() || !def->term()->isStruct()) {
                        Print("WARNING: planet struct missing in '%s'\n", filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();
                        ParsePlanet(val);
                    }
                }

                else if (def->name()->value() == "moon") {
                    if (!def->term() || !def->term()->isStruct()) {
                        Print("WARNING: moon struct missing in '%s'\n", filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();
                        ParseMoon(val);
                    }
                }

                else if (def->name()->value() == "region") {
                    if (!def->term() || !def->term()->isStruct()) {
                        Print("WARNING: region struct missing in '%s'\n", filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();
                        ParseRegion(val);
                    }
                }

                else if (def->name()->value() == "terrain") {
                    if (!def->term() || !def->term()->isStruct()) {
                        Print("WARNING: terrain struct missing in '%s'\n", filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();
                        ParseTerrain(val);
                    }
                }
                
            }
        }
    }
    while (term);

    loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
StarSystem::ParseStar(TermStruct* val)
{
    char  star_name[NAMELEN];
    char  img_name[NAMELEN];
    char  map_name[NAMELEN];
    double light  = 0.0;
    double radius = 0.0;
    double rot    = 0.0;
    double mass   = 0.0;
    double orbit  = 0.0;
    double tscale = 1.0;
    bool   retro  = false;
    Color  color;
    Color  back;

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value() == "name")
            GetDefText(star_name, pdef, filename);

            else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon")
            GetDefText(map_name, pdef, filename);

            else if (pdef->name()->value() == "image")
            GetDefText(img_name, pdef, filename);

            else if (pdef->name()->value() == "mass")
            GetDefNumber(mass, pdef, filename);

            else if (pdef->name()->value() == "orbit")
            GetDefNumber(orbit, pdef, filename);

            else if (pdef->name()->value() == "radius")
            GetDefNumber(radius, pdef, filename);

            else if (pdef->name()->value() == "rotation")
            GetDefNumber(rot, pdef, filename);

            else if (pdef->name()->value() == "tscale")
            GetDefNumber(tscale, pdef, filename);

            else if (pdef->name()->value() == "light")
            GetDefNumber(light, pdef, filename);

            else if (pdef->name()->value() == "retro")
            GetDefBool(retro, pdef, filename);

            else if (pdef->name()->value() == "color") {
                Vec3 a;
                GetDefVec(a, pdef, filename);
                color = Color((BYTE) a.x, (BYTE) a.y, (BYTE) a.z);
            }

            else if (pdef->name()->value() == "back" || pdef->name()->value() == "back_color") {
                Vec3 a;
                GetDefVec(a, pdef, filename);
                back = Color((BYTE) a.x, (BYTE) a.y, (BYTE) a.z);
            }
        }
    }

    OrbitalBody* star = new(__FILE__,__LINE__) OrbitalBody(this, star_name, Orbital::STAR, mass, radius, orbit, center);
    star->map_name       = map_name;
    star->tex_name       = img_name;
    star->light          = light;
    star->tscale         = tscale;
    star->subtype        = Star::G;
    star->retro          = retro;
    star->rotation       = rot * 3600;
    star->color          = color;
    star->back           = back;

    // map icon:
    if (*map_name) {
        DataLoader::GetLoader()->LoadBitmap(map_name, star->map_icon, Bitmap::BMP_TRANSLUCENT, true);
    }

    bodies.append(star);
    primary_star   = star;
    primary_planet = 0;
    primary_moon   = 0;

    if (orbit > StarSystem::radius)
    StarSystem::radius = orbit;
}

// +--------------------------------------------------------------------+

void
StarSystem::ParsePlanet(TermStruct* val)
{
    char   pln_name[NAMELEN];
    char   img_name[NAMELEN];
    char   map_name[NAMELEN];
    char   hi_name[NAMELEN];
    char   img_ring[NAMELEN];
    char   glo_name[NAMELEN];
    char   glo_hi_name[NAMELEN];
    char   gloss_name[NAMELEN];

    double radius = 0.0;
    double mass   = 0.0;
    double orbit  = 0.0;
    double rot    = 0.0;
    double minrad = 0.0;
    double maxrad = 0.0;
    double tscale = 1.0;
    double tilt   = 0.0;
    bool   retro  = false;
    bool   lumin  = false;
    Color  atmos  = Color::Black;

    ZeroMemory(pln_name,    NAMELEN);
    ZeroMemory(hi_name,     NAMELEN);
    ZeroMemory(img_ring,    NAMELEN);
    ZeroMemory(glo_name,    NAMELEN);
    ZeroMemory(glo_hi_name, NAMELEN);
    ZeroMemory(gloss_name,  NAMELEN);
    ZeroMemory(map_name,    NAMELEN);

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value() == "name")
            GetDefText(pln_name, pdef, filename);

            else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon")
            GetDefText(map_name, pdef, filename);

            else if (pdef->name()->value() == "image")
            GetDefText(img_name, pdef, filename);

            else if (pdef->name()->value() == "image_west")
            GetDefText(img_name, pdef, filename);

            else if (pdef->name()->value() == "image_east")
            GetDefText(img_name, pdef, filename);

            else if (pdef->name()->value() == "glow")
            GetDefText(glo_name, pdef, filename);

            else if (pdef->name()->value() == "gloss")
            GetDefText(gloss_name, pdef, filename);

            else if (pdef->name()->value() == "high_res")
            GetDefText(hi_name, pdef, filename);

            else if (pdef->name()->value() == "high_res_west")
            GetDefText(hi_name, pdef, filename);

            else if (pdef->name()->value() == "high_res_east")
            GetDefText(hi_name, pdef, filename);

            else if (pdef->name()->value() == "glow_high_res")
            GetDefText(glo_hi_name, pdef, filename);

            else if (pdef->name()->value() == "mass")
            GetDefNumber(mass, pdef, filename);

            else if (pdef->name()->value() == "orbit")
            GetDefNumber(orbit, pdef, filename);

            else if (pdef->name()->value() == "retro")
            GetDefBool(retro, pdef, filename);

            else if (pdef->name()->value() == "luminous")
            GetDefBool(lumin, pdef, filename);

            else if (pdef->name()->value() == "rotation")
            GetDefNumber(rot, pdef, filename);

            else if (pdef->name()->value() == "radius")
            GetDefNumber(radius, pdef, filename);

            else if (pdef->name()->value() == "ring")
            GetDefText(img_ring, pdef, filename);

            else if (pdef->name()->value() == "minrad")
            GetDefNumber(minrad, pdef, filename);

            else if (pdef->name()->value() == "maxrad")
            GetDefNumber(maxrad, pdef, filename);

            else if (pdef->name()->value() == "tscale")
            GetDefNumber(tscale, pdef, filename);

            else if (pdef->name()->value() == "tilt")
            GetDefNumber(tilt, pdef, filename);

            else if (pdef->name()->value() == "atmosphere") {
                Vec3 a;
                GetDefVec(a, pdef, filename);
                atmos = Color((BYTE) a.x, (BYTE) a.y, (BYTE) a.z);
            }
        }
    }

    OrbitalBody* planet = new(__FILE__,__LINE__) OrbitalBody(this, pln_name, Orbital::PLANET, mass, radius, orbit, primary_star);
    planet->map_name = map_name;
    planet->tex_name = img_name;
    planet->tex_high_res = hi_name;
    planet->tex_ring = img_ring;
    planet->tex_glow = glo_name;
    planet->tex_glow_high_res = glo_hi_name;
    planet->tex_gloss = gloss_name;
    planet->ring_min = minrad;
    planet->ring_max = maxrad;
    planet->tscale   = tscale;
    planet->tilt     = tilt;
    planet->retro    = retro;
    planet->luminous = lumin;
    planet->rotation = rot * 3600;
    planet->atmosphere = atmos;

    if (primary_star)
    primary_star->satellites.append(planet);
    else
    bodies.append(planet);

    primary_planet = planet;
    primary_moon   = 0;

    if (orbit > StarSystem::radius)
    StarSystem::radius = orbit;

    // map icon:
    if (map_name[0]) {
        DataLoader::GetLoader()->LoadBitmap(map_name, planet->map_icon, Bitmap::BMP_TRANSLUCENT, true);
    }
}

// +--------------------------------------------------------------------+

void
StarSystem::ParseMoon(TermStruct* val)
{
    char   map_name[NAMELEN];
    char   pln_name[NAMELEN];
    char   img_name[NAMELEN];
    char   hi_name[NAMELEN];
    char   glo_name[NAMELEN];
    char   glo_hi_name[NAMELEN];
    char   gloss_name[NAMELEN];

    double radius = 0.0;
    double mass   = 0.0;
    double orbit  = 0.0;
    double rot    = 0.0;
    double tscale = 1.0;
    double tilt   = 0.0;
    bool   retro  = false;
    Color  atmos  = Color::Black;

    ZeroMemory(map_name, NAMELEN);
    ZeroMemory(pln_name, NAMELEN);
    ZeroMemory(hi_name, NAMELEN);
    ZeroMemory(img_name, NAMELEN);
    ZeroMemory(glo_name, NAMELEN);
    ZeroMemory(glo_hi_name, NAMELEN);
    ZeroMemory(gloss_name,  NAMELEN);

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value() == "name")
            GetDefText(pln_name, pdef, filename);

            else if (pdef->name()->value() == "map" || pdef->name()->value() == "icon")
            GetDefText(map_name, pdef, filename);

            else if (pdef->name()->value() == "image")
            GetDefText(img_name, pdef, filename);

            else if (pdef->name()->value() == "glow")
            GetDefText(glo_name, pdef, filename);

            else if (pdef->name()->value() == "high_res")
            GetDefText(hi_name, pdef, filename);

            else if (pdef->name()->value() == "glow_high_res")
            GetDefText(glo_hi_name, pdef, filename);

            else if (pdef->name()->value() == "gloss")
            GetDefText(gloss_name, pdef, filename);

            else if (pdef->name()->value() == "mass")
            GetDefNumber(mass, pdef, filename);

            else if (pdef->name()->value() == "orbit")
            GetDefNumber(orbit, pdef, filename);

            else if (pdef->name()->value() == "rotation")
            GetDefNumber(rot, pdef, filename);

            else if (pdef->name()->value() == "retro")
            GetDefBool(retro, pdef, filename);

            else if (pdef->name()->value() == "radius")
            GetDefNumber(radius, pdef, filename);

            else if (pdef->name()->value() == "tscale")
            GetDefNumber(tscale, pdef, filename);

            else if (pdef->name()->value() == "inclination")
            GetDefNumber(tilt, pdef, filename);

            else if (pdef->name()->value() == "atmosphere") {
                Vec3 a;
                GetDefVec(a, pdef, filename);
                atmos = Color((BYTE) a.x, (BYTE) a.y, (BYTE) a.z);
            }
        }
    }

    OrbitalBody* moon = new(__FILE__,__LINE__) OrbitalBody(this, pln_name, Orbital::MOON, mass, radius, orbit, primary_planet);
    moon->map_name = map_name;
    moon->tex_name = img_name;
    moon->tex_high_res = hi_name;
    moon->tex_glow = glo_name;
    moon->tex_glow_high_res = glo_hi_name;
    moon->tex_gloss = gloss_name;
    moon->tscale   = tscale;
    moon->retro    = retro;
    moon->rotation = rot * 3600;
    moon->tilt     = tilt;
    moon->atmosphere = atmos;

    if (primary_planet)
    primary_planet->satellites.append(moon);
    else {
        Print("WARNING: no planet for moon %s in '%s', deleted.\n", pln_name, filename);
        delete moon;
        moon = 0;
    }

    primary_moon = moon;

    // map icon:
    if (map_name[0]) {
        DataLoader::GetLoader()->LoadBitmap(map_name, moon->map_icon, Bitmap::BMP_TRANSLUCENT, true);
    }
}

// +--------------------------------------------------------------------+

void
StarSystem::ParseRegion(TermStruct* val)
{
    char  rgn_name[NAMELEN];
    char  lnk_name[NAMELEN];
    double size  = 1.0e6;
    double orbit = 0.0;
    double grid  = 25000;
    double inclination = 0.0;
    int    asteroids = 0;

    List<Text> links;

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value() == "name")
            GetDefText(rgn_name, pdef, filename);

            else if (pdef->name()->value() == "link") {
                GetDefText(lnk_name, pdef, filename);
                if (lnk_name[0]) {
                    links.append(new(__FILE__,__LINE__) Text(lnk_name));
                }
            }

            else if (pdef->name()->value() == "orbit")
            GetDefNumber(orbit, pdef, filename);

            else if (pdef->name()->value() == "size")
            GetDefNumber(size, pdef, filename);

            else if (pdef->name()->value() == "radius")
            GetDefNumber(size, pdef, filename);

            else if (pdef->name()->value() == "grid")
            GetDefNumber(grid, pdef, filename);

            else if (pdef->name()->value() == "inclination")
            GetDefNumber(inclination, pdef, filename);

            else if (pdef->name()->value() == "asteroids")
            GetDefNumber(asteroids, pdef, filename);
        }
    }

    Orbital*      primary = primary_moon;
    if (!primary) primary = primary_planet;
    if (!primary) primary = primary_star;

    OrbitalRegion* region = new(__FILE__,__LINE__) OrbitalRegion(this, rgn_name, 0, size, orbit, primary);
    region->grid        = grid;
    region->inclination = inclination;
    region->asteroids   = asteroids;
    region->links.append(links);

    if (primary)
    primary->regions.append(region);
    else
    regions.append(region);

    all_regions.append(region);

    if (orbit > StarSystem::radius)
    StarSystem::radius = orbit;
}

// +--------------------------------------------------------------------+

void
StarSystem::ParseTerrain(TermStruct* val)
{
    Orbital*      primary = primary_moon;
    if (!primary) primary = primary_planet;

    if (!primary) {
        Print("WARNING: Terrain region with no primary ignored in '%s'\n", filename);
        return;
    }

    TerrainRegion* region = 0;

    Text   rgn_name;
    Text   patch_name;
    Text   patch_texture;
    Text   noise_tex0;
    Text   noise_tex1;
    Text   apron_name;
    Text   apron_texture;
    Text   water_texture;
    Text   env_texture_positive_x;
    Text   env_texture_negative_x;
    Text   env_texture_positive_y;
    Text   env_texture_negative_y;
    Text   env_texture_positive_z;
    Text   env_texture_negative_z;
    Text   haze_name;
    Text   sky_name;
    Text   clouds_high;
    Text   clouds_low;
    Text   shades_high;
    Text   shades_low;

    double size          = 1.0e6;
    double grid          = 25000;
    double inclination   = 0.0;
    double scale         = 10e3;
    double mtnscale      = 1e3;
    double fog_density   = 0;
    double fog_scale     = 0;
    double haze_fade     = 0;
    double clouds_alt_high= 0;
    double clouds_alt_low= 0;
    double w_period      = 0;
    double w_chances[Weather::NUM_STATES];

    ZeroMemory(w_chances, sizeof(w_chances));

    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value() == "name")
            GetDefText(rgn_name, pdef, filename);

            else if (pdef->name()->value() == "patch")
            GetDefText(patch_name, pdef, filename);

            else if (pdef->name()->value() == "patch_texture")
            GetDefText(patch_texture, pdef, filename);

            else if (pdef->name()->value() == "detail_texture_0")
            GetDefText(noise_tex0, pdef, filename);

            else if (pdef->name()->value() == "detail_texture_1")
            GetDefText(noise_tex1, pdef, filename);

            else if (pdef->name()->value() == "apron")
            GetDefText(apron_name, pdef, filename);

            else if (pdef->name()->value() == "apron_texture")
            GetDefText(apron_texture, pdef, filename);

            else if (pdef->name()->value() == "water_texture")
            GetDefText(water_texture, pdef, filename);

            else if (pdef->name()->value() == "env_texture_positive_x")
            GetDefText(env_texture_positive_x, pdef, filename);

            else if (pdef->name()->value() == "env_texture_negative_x")
            GetDefText(env_texture_negative_x, pdef, filename);

            else if (pdef->name()->value() == "env_texture_positive_y")
            GetDefText(env_texture_positive_y, pdef, filename);

            else if (pdef->name()->value() == "env_texture_negative_y")
            GetDefText(env_texture_negative_y, pdef, filename);

            else if (pdef->name()->value() == "env_texture_positive_z")
            GetDefText(env_texture_positive_z, pdef, filename);

            else if (pdef->name()->value() == "env_texture_negative_z")
            GetDefText(env_texture_negative_z, pdef, filename);

            else if (pdef->name()->value() == "clouds_high")
            GetDefText(clouds_high, pdef, filename);

            else if (pdef->name()->value() == "shades_high")
            GetDefText(shades_high, pdef, filename);

            else if (pdef->name()->value() == "clouds_low")
            GetDefText(clouds_low, pdef, filename);

            else if (pdef->name()->value() == "shades_low")
            GetDefText(shades_low, pdef, filename);

            else if (pdef->name()->value() == "haze")
            GetDefText(haze_name, pdef, filename);

            else if (pdef->name()->value() == "sky_color")
            GetDefText(sky_name, pdef, filename);

            else if (pdef->name()->value() == "size" ||
                    pdef->name()->value() == "radius")
            GetDefNumber(size, pdef, filename);

            else if (pdef->name()->value() == "grid")
            GetDefNumber(grid, pdef, filename);

            else if (pdef->name()->value() == "inclination")
            GetDefNumber(inclination, pdef, filename);

            else if (pdef->name()->value() == "scale")
            GetDefNumber(scale, pdef, filename);

            else if (pdef->name()->value() == "mtnscale" ||
                    pdef->name()->value() == "mtn_scale")
            GetDefNumber(mtnscale, pdef, filename);

            else if (pdef->name()->value() == "fog_density")
            GetDefNumber(fog_density, pdef, filename);

            else if (pdef->name()->value() == "fog_scale")
            GetDefNumber(fog_scale, pdef, filename);

            else if (pdef->name()->value() == "haze_fade")
            GetDefNumber(haze_fade, pdef, filename);

            else if (pdef->name()->value() == "clouds_alt_high")
            GetDefNumber(clouds_alt_high, pdef, filename);

            else if (pdef->name()->value() == "clouds_alt_low")
            GetDefNumber(clouds_alt_low, pdef, filename);

            else if (pdef->name()->value() == "weather_period")
            GetDefNumber(w_period, pdef, filename);

            else if (pdef->name()->value() == "weather_clear")
            GetDefNumber(w_chances[0], pdef, filename);
            else if (pdef->name()->value() == "weather_high_clouds")
            GetDefNumber(w_chances[1], pdef, filename);
            else if (pdef->name()->value() == "weather_moderate_clouds")
            GetDefNumber(w_chances[2], pdef, filename);
            else if (pdef->name()->value() == "weather_overcast")
            GetDefNumber(w_chances[3], pdef, filename);
            else if (pdef->name()->value() == "weather_fog")
            GetDefNumber(w_chances[4], pdef, filename);
            else if (pdef->name()->value() == "weather_storm")
            GetDefNumber(w_chances[5], pdef, filename);

            else if (pdef->name()->value() == "layer") {
                if (!pdef->term() || !pdef->term()->isStruct()) {
                    Print("WARNING: terrain layer struct missing in '%s'\n", filename);
                }
                else {

                    if (!region)
                    region = new(__FILE__,__LINE__) TerrainRegion(this, rgn_name, size, primary);

                    TermStruct* val = pdef->term()->isStruct();
                    ParseLayer(region, val);
                }
            }
        }
    }

    if (!region)
    region = new(__FILE__,__LINE__) TerrainRegion(this, rgn_name, size, primary);

    region->grid             = grid;
    region->inclination      = inclination;
    region->patch_name       = patch_name;
    region->patch_texture    = patch_texture;
    region->noise_tex0       = noise_tex0;
    region->noise_tex1       = noise_tex1;
    region->apron_name       = apron_name;
    region->apron_texture    = apron_texture;
    region->water_texture    = water_texture;
    region->haze_name        = haze_name;
    region->clouds_high      = clouds_high;
    region->shades_high      = shades_high;
    region->clouds_low       = clouds_low;
    region->shades_low       = shades_low;
    region->scale            = scale;
    region->mtnscale         = mtnscale;
    region->fog_density      = fog_density;
    region->fog_scale        = fog_scale;
    region->haze_fade        = haze_fade;
    region->clouds_alt_high  = clouds_alt_high;
    region->clouds_alt_low   = clouds_alt_low;

    region->env_texture_positive_x = env_texture_positive_x;
    region->env_texture_negative_x = env_texture_negative_x;
    region->env_texture_positive_y = env_texture_positive_y;
    region->env_texture_negative_y = env_texture_negative_y;
    region->env_texture_positive_z = env_texture_positive_z;
    region->env_texture_negative_z = env_texture_negative_z;

    Weather& weather = region->GetWeather();

    weather.SetPeriod(w_period);

    for (int i = 0; i < Weather::NUM_STATES; i++)
    weather.SetChance(i, w_chances[i]);

    region->LoadSkyColors(sky_name);

    primary->regions.append(region);
    all_regions.append(region);
}

// +--------------------------------------------------------------------+

void
StarSystem::ParseLayer(TerrainRegion* rgn, TermStruct* val)
{
    Text   tile_name;
    Text   detail_name;
    double height = 0;


    for (int i = 0; i < val->elements()->size(); i++) {
        TermDef* pdef = val->elements()->at(i)->isDef();
        if (pdef) {
            if (pdef->name()->value().indexOf("tile") == 0)
            GetDefText(tile_name, pdef, filename);

            else if (pdef->name()->value().indexOf("detail") == 0)
            GetDefText(detail_name, pdef, filename);

            else if (pdef->name()->value().contains("height") ||
                    pdef->name()->value().contains("alt"))
            GetDefNumber(height, pdef, filename);
        }
    }

    if (rgn)
    rgn->AddLayer(height, tile_name, detail_name);
}

// +--------------------------------------------------------------------+

void
StarSystem::Create()
{
    if (Game::Server())
    return;

    if (!instantiated) {
        Print("Creating Star System %s\n", (const char*) name);

        DataLoader* loader = DataLoader::GetLoader();
        loader->SetDataPath(datapath);

        // poly star shell:
        if (sky_poly_stars.length()) {
            poly_stars = new(__FILE__,__LINE__) Solid;
            poly_stars->Load(sky_poly_stars, 120);
            poly_stars->SetLuminous(true);
            poly_stars->SetInfinite(true);

            Print("Celestial Sphere '%s' loaded\n", (const char*) sky_poly_stars);
            Print("   radius: %f\n", poly_stars->Radius());
        }

        if (sky_stars) {
            Print("Point Stars: %d\n", sky_stars);
            point_stars = new(__FILE__,__LINE__) Stars(sky_stars);
        }

        loader->SetDataPath(datapath);

        // nebula:
        if (sky_nebula.length()) {
            nebula = new(__FILE__,__LINE__) Solid;
            nebula->Load(sky_nebula, 100);
            nebula->SetLuminous(true);
            nebula->SetInfinite(true);

            Print("Nebular Sky '%s' loaded\n", (const char*) sky_nebula);
            Print("   radius: %f\n", nebula->Radius());
        }

        // atmospheric haze:
        if (sky_haze.length()) {
            loader->SetDataPath(datapath);
            haze = new(__FILE__,__LINE__) TerrainHaze();
            haze->Load(sky_haze, 120);

            haze->SetInfinite(true);

            Print("Atmospheric Haze '%s' loaded\n", (const char*) sky_haze);
            Print("   radius: %f\n", haze->Radius());

            haze->Hide();
        }

        loader->SetDataPath(0);
        
        ListIter<OrbitalBody> star = bodies;
        while (++star) {
            CreateBody(*star);

            ListIter<OrbitalBody> planet = star->Satellites();
            while (++planet) {
                CreateBody(*planet);
                
                ListIter<OrbitalBody> moon = planet->Satellites();
                while (++moon) {
                    CreateBody(*moon);
                }
            }
        }
    }

    instantiated = true;
}

// +--------------------------------------------------------------------+

void
StarSystem::CreateBody(OrbitalBody& body)
{
    DataLoader*  loader = DataLoader::GetLoader();
    loader->SetDataPath(datapath);

    // stars:
    if (body.type == Orbital::STAR) {
        Point starloc = body.loc;
        starloc = starloc.OtherHand();

        PlanetRep* rep = new(__FILE__,__LINE__) PlanetRep(body.tex_name,
        0,
        body.radius, 
        starloc, 
        body.tscale);

        rep->SetLuminous(true);
        rep->MoveTo(loc);
        body.rep       = rep;

        sun_brightness = body.light;
        sun_color      = body.color;

        Light* sun_light = new(__FILE__,__LINE__) Light(1.1f);
        sun_light->SetColor(sun_color);
        sun_light->SetShadow(true);
        sun_light->MoveTo(body.loc);
        sun_light->SetType(Light::LIGHT_DIRECTIONAL);

        sun_lights.append(sun_light);
        body.light_rep = sun_light;

        if (body.back != Color::Black) {
            Light* back_light = new(__FILE__,__LINE__) Light(0.6f);
            back_light->SetColor(body.back);
            back_light->SetShadow(false);
            back_light->MoveTo(body.loc * -1);
            back_light->SetType(Light::LIGHT_DIRECTIONAL);

            back_lights.append(back_light);
            body.back_light = back_light;
        }
    }   

    // planets and moons:
    else {
        Point planetloc = body.loc;
        planetloc = planetloc.OtherHand();

        double rmax = 0;
        double rmin = 0;
        
        if (body.ring_max > 0) {
            rmax = body.ring_max*body.radius;
            rmin = body.ring_min*body.radius;
        }

        Text surface = body.tex_name;
        Text glow    = body.tex_glow;

        if (Game::MaxTexSize() >= 512) {
            if (body.tex_high_res.length())
            surface = body.tex_high_res;

            if (body.tex_glow_high_res.length())
            glow = body.tex_glow_high_res;
        }

        PlanetRep* rep = new(__FILE__,__LINE__) PlanetRep(surface,
        glow,
        body.radius,
        planetloc,
        body.tscale,
        body.tex_ring,
        rmin,
        rmax,
        body.atmosphere,
        body.tex_gloss);

        rep->SetStarSystem(this);

        /***
    if (body.luminous) {
        rep->SetLuminous(1);
    }
    ***/

        if (body.tilt != 0) {
            Matrix m;
            m.Pitch(body.tilt);
            
            rep->SetOrientation(m);
        }

        body.rep = rep;
    }
}

// +--------------------------------------------------------------------+

void
StarSystem::Destroy()
{
    if (instantiated) {
        ListIter<OrbitalBody> star_iter = bodies;
        while (++star_iter) {
            OrbitalBody* star = star_iter.value();

            GRAPHIC_DESTROY(star->rep);
            LIGHT_DESTROY(star->light_rep);
            LIGHT_DESTROY(star->back_light);

            ListIter<OrbitalBody> planet = star->Satellites();
            while (++planet) {
                GRAPHIC_DESTROY(planet->rep);
                
                ListIter<OrbitalBody> moon = planet->Satellites();
                while (++moon) {
                    GRAPHIC_DESTROY(moon->rep);
                }
            }
        }

        GRAPHIC_DESTROY(point_stars);
        GRAPHIC_DESTROY(poly_stars);
        GRAPHIC_DESTROY(nebula);
        GRAPHIC_DESTROY(haze);

        sun_lights.clear();
        back_lights.clear();
    }

    instantiated = false;
    sun_scale    = 1;
}

// +--------------------------------------------------------------------+

void
StarSystem::Activate(Scene& scene)
{
    if (!instantiated)
    Create();

    Starshatter* stars = Starshatter::GetInstance();

    if (!stars)
    return;

    if (point_stars) {
        scene.AddBackground(point_stars);
        point_stars->Hide();
    }

    if (poly_stars) {
        scene.AddBackground(poly_stars);
    }

    if (stars->Nebula() && nebula) {
        scene.AddBackground(nebula);
    }

    if (haze) {
        scene.AddBackground(haze);
        haze->Hide();
    }

    ListIter<OrbitalBody> star_iter = bodies;
    while (++star_iter) {
        OrbitalBody* star = star_iter.value();
        scene.AddGraphic(star->rep);
        scene.AddLight(star->light_rep);
        if (nebula && stars && stars->Nebula()) {
            star->back_light->SetColor(star->back);
        }
        else {
            Color c = Color(60,60,65);
            star->back_light->SetColor(c);
        }
        scene.AddLight(star->back_light);

        if (nebula && stars && stars->Nebula()) {
            scene.SetAmbient(ambient);
        }
        else {
            Color c = ambient;
            int   n = (c.Red() + c.Green() + c.Blue()) / 3;

            c = Color(n,n,n);
            scene.SetAmbient(c);
        }

        ListIter<OrbitalBody> planet = star->Satellites();
        while (++planet) {
            scene.AddGraphic(planet->rep);

            ListIter<OrbitalBody> moon = planet->Satellites();
            while (++moon) {
                scene.AddGraphic(moon->rep);
            }
        }
    }

    ExecFrame();
}

// +--------------------------------------------------------------------+

void
StarSystem::Deactivate()
{
    if (!instantiated)
    return;

    active_region = 0;

    if (point_stars && point_stars->GetScene())
    point_stars->GetScene()->DelBackground(point_stars);

    if (poly_stars && poly_stars->GetScene())
    poly_stars->GetScene()->DelBackground(poly_stars);

    if (nebula && nebula->GetScene())
    nebula->GetScene()->DelBackground(nebula);

    if (haze && haze->GetScene()) {
        haze->GetScene()->DelBackground(haze);
        haze->Hide();
    }

    ListIter<OrbitalBody> star = bodies;
    while (++star) {
        if (star->rep && star->rep->GetScene())
        star->rep->GetScene()->DelGraphic(star->rep);

        if (star->light_rep && star->light_rep->GetScene())
        star->light_rep->GetScene()->DelLight(star->light_rep);

        if (star->back_light && star->back_light->GetScene())
        star->back_light->GetScene()->DelLight(star->back_light);

        ListIter<OrbitalBody> planet = star->Satellites();
        while (++planet) {
            if (planet->rep && planet->rep->GetScene())
            planet->rep->GetScene()->DelGraphic(planet->rep);

            ListIter<OrbitalBody> moon = planet->Satellites();
            while (++moon) {
                if (moon->rep && moon->rep->GetScene())
                moon->rep->GetScene()->DelGraphic(moon->rep);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
StarSystem::SetActiveRegion(OrbitalRegion* rgn)
{
    Scene* scene = 0;

    if (active_region != rgn) {
        active_region = rgn;

        if (active_region) {
            if (active_region->Type() != Orbital::TERRAIN) {
                if (point_stars)  point_stars->Hide();
                if (poly_stars)   poly_stars->Show();
                if (nebula)       nebula->Show();
                if (haze)         haze->Hide();
            }
            else {
                if (point_stars)  point_stars->Show();
                if (poly_stars)   poly_stars->Hide();
                if (nebula)       nebula->Hide();
                if (haze)         haze->Show();
            }

            if (poly_stars) {
                scene = poly_stars->GetScene();
                if (scene)
                scene->SetAmbient(ambient);
            }
            else if (nebula) {
                scene = nebula->GetScene();
                if (scene)
                scene->SetAmbient(ambient);
            }
            
            ListIter<OrbitalBody> star = bodies;
            while (++star) {
                if (star->rep)
                star->rep->Show();
            }

            ExecFrame();
        }
        else {
            if (point_stars)     point_stars->Hide();
            if (poly_stars)      poly_stars->Hide();
            if (nebula)          nebula->Hide();
            if (haze)            haze->Hide();


            ListIter<OrbitalBody> star = bodies;
            while (++star) {
                if (star->rep)
                star->rep->Hide();

                if (star->light_rep) {
                    scene = star->light_rep->GetScene();
                    if (scene)
                    scene->DelLight(star->light_rep);
                }

                if (star->back_light) {
                    scene = star->back_light->GetScene();
                    if (scene)
                    scene->DelLight(star->back_light);
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

static BYTE max3(BYTE a, BYTE b, BYTE c)
{
    if (a > b)
    if (a > c) return a;
    else       return c;
    else
    if (b > c) return b;
    else       return c;
}   

static BYTE min3(BYTE a, BYTE b, BYTE c)
{
    if (a < b)
    if (a < c) return a;
    else       return c;
    else
    if (b < c) return b;
    else       return c;
}   

void
StarSystem::ExecFrame()
{
    CalcStardate();

    ListIter<OrbitalBody> star = bodies;
    while (++star)
    star->Update();

    ListIter<OrbitalRegion> region = regions;
    while (++region)
    region->Update();

    // update the graphic reps, relative to the active region:
    if (instantiated && active_region) {
        Point active_loc = active_region->Location();
        active_loc = active_loc.OtherHand();

        Scene*         scene   = 0;
        TerrainRegion* trgn    = 0;
        bool           terrain = (active_region->Type() == Orbital::TERRAIN);
        Matrix         terrain_orientation;
        Matrix         terrain_transformation;

        if (terrain) {
            trgn = (TerrainRegion*) active_region;
            Color tmp = trgn->SkyColor();
            Game::SetScreenColor(tmp);

            tvpn = (active_region->Location() - active_region->Primary()->Location());
            tvpn.Normalize();
            tvup = Point(0,0,-1);
            tvrt = tvpn.cross(tvup);
            tvrt.Normalize();

            terrain_orientation.Rotate(0, PI/2, 0);
            terrain_transformation = Matrix(tvrt, tvup, tvpn);

            if (point_stars) {
                point_stars->SetOrientation(terrain_transformation);

                Color sky_color = trgn->SkyColor();
                BYTE  sky_red   = (BYTE) sky_color.Red();
                BYTE  sky_green = (BYTE) sky_color.Green();
                BYTE  sky_blue  = (BYTE) sky_color.Blue();

                BYTE Max = max3(sky_red, sky_green, sky_blue);
                BYTE Min = min3(sky_red, sky_green, sky_blue);

                BYTE lum = (BYTE) (240.0 * (Max + Min) / 510.0);

                if (lum > 50) {
                    point_stars->Hide();
                }
                else {
                    Stars* pstars = (Stars*) point_stars;

                    pstars->Illuminate(1.0 - lum / 50.0);
                    pstars->Show();
                }

                scene = point_stars->GetScene();
            }

            if (haze) {
                ((TerrainHaze*)haze)->UseTerrainRegion(trgn);
                scene = haze->GetScene();
            }

            if (scene) {
                scene->SetAmbient(ambient);
            }

        }
        else {
            Game::SetScreenColor(Color::Black);
        }

        double star_alt = 0;

        ListIter<OrbitalBody> star_iter = bodies;
        while (++star_iter) {
            OrbitalBody* star = star_iter.value();

            if (active_region->Inclination() != 0) {
                double distance = (active_region->Location() - star->Location()).length();
                star_alt = sin(active_region->Inclination()) * distance;
            }

            if (terrain) {
                Point sloc = TerrainTransform(star->Location());

                if (star->rep) {
                    star->rep->MoveTo(sloc);

                    PlanetRep* pr = (PlanetRep*) star->rep;
                    pr->SetDaytime(true);
                }

                if (star->light_rep) {
                    star->light_rep->MoveTo(sloc);
                    star->light_rep->SetActive(sloc.y > -100);
                }

                if (star->back_light) {
                    star->back_light->MoveTo(sloc * -1);
                    star->back_light->SetActive(sloc.y > -100);
                }

                if (trgn && star->rep) {
                    if (trgn->IsEclipsed())
                    star->rep->Hide();
                    else
                    star->rep->Show();
                }
            }

            else {
                Point sloc = Point(star->Location() - active_region->Location()).OtherHand();
                sloc.y = star_alt;

                if (star->rep) {
                    star->rep->MoveTo(sloc);

                    PlanetRep* pr = (PlanetRep*) star->rep;
                    pr->SetDaytime(false);
                }

                if (star->light_rep) {
                    star->light_rep->MoveTo(sloc);
                    star->light_rep->SetActive(true);
                }

                if (star->back_light) {
                    star->back_light->MoveTo(sloc * -1);
                    star->back_light->SetActive(true);
                }
            }
            
            ListIter<OrbitalBody> planet_iter = star->Satellites();
            while (++planet_iter) {
                OrbitalBody* planet = planet_iter.value();

                if (planet->rep) {
                    PlanetRep* pr = (PlanetRep*) planet->rep;

                    if (terrain) {
                        pr->MoveTo(TerrainTransform(planet->Location()));
                        pr->SetOrientation(terrain_orientation);

                        if (planet == active_region->Primary()) {
                            pr->Hide();
                        }

                        else {
                            pr->Show();
                            pr->SetDaytime(true);
                        }
                    }
                    else {
                        pr->Show();
                        pr->TranslateBy(active_loc);
                        pr->SetDaytime(false);
                    }
                }
                
                ListIter<OrbitalBody> moon_iter = planet->Satellites();
                while (++moon_iter) {
                    OrbitalBody* moon = moon_iter.value();

                    if (moon->rep) {
                        PlanetRep* pr = (PlanetRep*) moon->rep;

                        if (terrain) {
                            pr->MoveTo(TerrainTransform(moon->Location()));
                            pr->SetOrientation(terrain_orientation);

                            if (moon == active_region->Primary()) {
                                pr->Hide();
                            }
                            
                            else {
                                pr->Show();
                                pr->SetDaytime(true);
                            }
                        }
                        else {
                            pr->Show();
                            pr->TranslateBy(active_loc);
                            pr->SetDaytime(false);
                        }
                    }
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

Orbital*
StarSystem::FindOrbital(const char* name)
{
    if (!name || !name[0])
    return 0;

    ListIter<OrbitalBody> star = bodies;
    while (++star) {
        if (!_stricmp(star->Name(), name))
        return star.value();

        ListIter<OrbitalRegion> star_rgn = star->Regions();
        while (++star_rgn) {
            if (!_stricmp(star_rgn->Name(), name))
            return star_rgn.value();
        }

        ListIter<OrbitalBody> planet = star->Satellites();
        while (++planet) {
            if (!_stricmp(planet->Name(), name))
            return planet.value();

            ListIter<OrbitalRegion> planet_rgn = planet->Regions();
            while (++planet_rgn) {
                if (!_stricmp(planet_rgn->Name(), name))
                return planet_rgn.value();
            }

            ListIter<OrbitalBody> moon = planet->Satellites();
            while (++moon) {
                if (!_stricmp(moon->Name(), name))
                return moon.value();

                ListIter<OrbitalRegion> moon_rgn = moon->Regions();
                while (++moon_rgn) {
                    if (!_stricmp(moon_rgn->Name(), name))
                    return moon_rgn.value();
                }
            }
        }
    }

    ListIter<OrbitalRegion> region = regions;
    while (++region) {
        if (!_stricmp(region->Name(), name))
        return region.value();
    }

    return 0;
}

// +--------------------------------------------------------------------+

OrbitalRegion*
StarSystem::FindRegion(const char* name)
{
    if (!name || !name[0])
    return 0;

    ListIter<OrbitalRegion> region = all_regions;
    while (++region) {
        if (!_stricmp(region->Name(), name))
        return region.value();
    }

    return 0;
}

// +--------------------------------------------------------------------+

bool
StarSystem::HasLinkTo(StarSystem* s) const
{
    ListIter<OrbitalRegion> iter = ((StarSystem*) this)->all_regions;
    while (++iter) {
        OrbitalRegion* rgn = iter.value();

        ListIter<Text> lnk_iter = rgn->Links();
        while (++lnk_iter) {
            Text* t = lnk_iter.value();

            if (s->FindRegion(*t))
            return true;
        }
    }

    return false;
}

// +--------------------------------------------------------------------+

Point
StarSystem::TerrainTransform(const Point& loc)
{
    Point result;

    Point tmp = loc - active_region->Location();

    result.x = tmp * tvrt;
    result.z = tmp * tvup;
    result.y = tmp * tvpn;

    return result;
}

Color
StarSystem::Ambient() const
{
    Color result  = ambient;
    bool  terrain = (active_region && active_region->Type() == Orbital::TERRAIN);

    if (terrain) {
        TerrainRegion* trgn = (TerrainRegion*) active_region;
        result = trgn->Ambient();

        if (trgn->IsEclipsed())
        result = result * 0.3;
    }

    return result;
}

void
StarSystem::SetSunlight(Color color, double brightness)
{
    sun_color = color;
    sun_scale = brightness;

    ListIter<Light> sun_iter = sun_lights;
    while (++sun_iter) {
        Light* sun_light = sun_iter.value();
        sun_light->SetColor(color);
        sun_light->SetIntensity((float) (1.1 * sun_scale));
    }

    ListIter<Light> back_iter = back_lights;
    while (++back_iter) {
        Light* back_light = back_iter.value();
        back_light->SetIntensity((float) (0.5 * sun_scale));
    }
}

void
StarSystem::SetBacklight(Color color, double brightness)
{
    ListIter<Light> back_iter = back_lights;
    while (++back_iter) {
        Light* back_light = back_iter.value();
        back_light->SetColor(color);
        back_light->SetIntensity((float) (0.5 * brightness));
    }
}

void
StarSystem::RestoreTrueSunColor()
{
    ListIter<OrbitalBody> iter = bodies;
    while (++iter) {
        OrbitalBody* star = iter.value();

        if (star) {
            if (star->light_rep) {
                star->light_rep->SetColor(star->LightColor());
                star->light_rep->SetIntensity(1.1f);
            }

            if (star->back_light) {
                star->back_light->SetColor(star->back);
                star->back_light->SetIntensity(0.5f);
            }
        }
    }
}

// +====================================================================+

Color
Star::GetColor() const
{
    return GetColor(seq);
}

int
Star::GetSize() const
{
    return GetSize(seq);
}

Color
Star::GetColor(int s)
{
    switch (s) {
    case O:           return Color(128,128,255); break;
    case B:           return Color(192,192,255); break;
    case A:           return Color(220,220,255); break;
    case F:           return Color(255,255,255); break;
    case G:           return Color(255,255,128); break;
    case K:           return Color(255,192,100); break;
    case M:           return Color(255,100,100); break;

    case RED_GIANT:   return Color(255, 80, 80); break;
    case WHITE_DWARF: return Color(255,255,255); break;
    case BLACK_HOLE:  return Color(  0,  0,  0); break;
    }

    return Color::White;
}

int
Star::GetSize(int s)
{
    switch (s) {
    case O:           return 4; break;
    case B:           return 4; break;
    case A:           return 3; break;
    case F:           return 3; break;
    case G:           return 2; break;
    case K:           return 2; break;
    case M:           return 1; break;

    case RED_GIANT:   return 4; break;
    case WHITE_DWARF: return 1; break;
    case BLACK_HOLE:  return 3; break;
    }

    return 3;
}

// +====================================================================+

Orbital::Orbital(StarSystem* s, const char* n, OrbitalType t, double m, double r, double o, Orbital* p)
: name(n), type(t), subtype(0), radius(r), mass(m), orbit(o),
phase(0), period(0), rotation(0), retro(false),
system(s), primary(p), loc(0, 0, 0), rep(0), velocity(0)
{
    if (system && primary && orbit > 0) {
        velocity = sqrt(GRAV * primary->Mass() / orbit);
        period   = 2 * PI * orbit / velocity;
    }

    Update();
}

// +--------------------------------------------------------------------+

Orbital::~Orbital()
{
    delete rep;
    regions.destroy();
}

// +--------------------------------------------------------------------+

void
Orbital::Update()
{
    if (system && primary && orbit > 0) {
        double grade = (retro) ? -1 : 1;

        // orbits are counter clockwise:
        phase = -2 * PI * grade * StarSystem::Stardate() / period;
        
        loc = primary->Location() + Point((double) (orbit * cos(phase)),
        (double) (orbit * sin(phase)),
        0);
    }

    ListIter<OrbitalRegion> region = regions;
    while (++region)
    region->Update();

    if (rep)
    rep->MoveTo(loc.OtherHand());
}

// +--------------------------------------------------------------------+

Point
Orbital::PredictLocation(double delta_t)
{
    Point predicted_loc = Location();

    if (system && primary && orbit > 0) {
        predicted_loc = primary->PredictLocation(delta_t);

        double grade = (retro) ? -1 : 1;

        // orbits are(?) counter clockwise:
        double predicted_phase = (double) (-2 * PI * grade * (StarSystem::Stardate()+delta_t) / period);
        
        predicted_loc += Point((double) (orbit * cos(predicted_phase)),
        (double) (orbit * sin(predicted_phase)),
        0);
    }

    return predicted_loc;
}

void
Orbital::SetMapIcon(const Bitmap& img)
{
    if (img.Width() >=64 && img.Height() >= 64) {
        map_icon.CopyBitmap(img);
        map_icon.AutoMask();
        map_icon.MakeTexture();
    }
}

// +====================================================================+

OrbitalBody::OrbitalBody(StarSystem* s, const char* n, OrbitalType t, double m, double r, double o, Orbital* p)
: Orbital(s, n, t, m, r, o, p), light(0), light_rep(0), back_light(0),
ring_min(0), ring_max(0), tilt(0), luminous(false)
{
    Update();
}

// +--------------------------------------------------------------------+

OrbitalBody::~OrbitalBody()
{
    satellites.destroy();
}

// +--------------------------------------------------------------------+

void
OrbitalBody::Update()
{
    Orbital::Update();

    theta = 0;

    if (rotation > 0)
    theta = -2 * PI * StarSystem::Stardate() / rotation;

    ListIter<OrbitalBody> body = satellites;
    while (++body)
    body->Update();

    if (rep && theta != 0) {
        Matrix m;
        m.Pitch(tilt);
        m.Roll(tilt/2);
        m.Yaw(theta);
        rep->SetOrientation(m);
    }

    if (light_rep) {
        Point bodyloc = loc;
        bodyloc = bodyloc.OtherHand();
        light_rep->MoveTo(bodyloc);
    }
}

// +====================================================================+

OrbitalRegion::OrbitalRegion(StarSystem* s, const char* n, double m, double r, double o, Orbital* p)
: Orbital(s, n, REGION, m, r, o, p), grid(25.0e3f), inclination(0)
{
}

// +--------------------------------------------------------------------+

OrbitalRegion::~OrbitalRegion()
{
    links.destroy();
}

