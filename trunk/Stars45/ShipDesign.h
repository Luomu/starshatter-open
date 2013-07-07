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
    FILE:         ShipDesign.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship Design parameters class
*/

#ifndef ShipDesign_h
#define ShipDesign_h

#include "Types.h"
#include "Bitmap.h"
#include "Geometry.h"
#include "term.h"
#include "List.h"

// +----------------------------------------------------------------------+

class ShipDesign;
class Model;
class Skin;
class PowerSource;
class Weapon;
class HardPoint;
class Computer;
class Drive;
class QuantumDrive;
class Farcaster;
class Thruster;
class Sensor;
class NavLight;
class NavSystem;
class Shield;
class FlightDeck;
class LandingGear;
class System;
class Sound;

// +====================================================================+

class ShipLoad
{
public:
    static const char* TYPENAME() { return "ShipLoad"; }

    ShipLoad();

    char     name[64];
    int      load[16];
    double   mass;
};

class ShipSquadron
{
public:
    static const char* TYPENAME() { return "ShipSquadron"; }

    ShipSquadron();

    char        name[64];
    ShipDesign* design;
    int         count;
    int         avail;
};

class ShipExplosion
{
public:
    static const char* TYPENAME() { return "ShipExplosion"; }

    ShipExplosion() { ZeroMemory(this, sizeof(ShipExplosion)); }

    int               type;
    float             time;
    Vec3              loc;
    bool              final;
};

class ShipDebris
{
public:
    static const char* TYPENAME() { return "ShipDebris"; }

    ShipDebris() { ZeroMemory(this, sizeof(ShipDebris)); }

    Model*            model;
    int               count;
    int               life;
    Vec3              loc;
    float             mass;
    float             speed;
    float             drag;
    int               fire_type;
    Vec3              fire_loc[5];
};

// +====================================================================+
// Used to share common information about ships of a single type.
// ShipDesign objects are loaded from a text file and stored in a
// static list (catalog) member for use by the Ship.

class ShipDesign
{
public:
    static const char* TYPENAME() { return "ShipDesign"; }

    enum CONSTANTS {
        MAX_DEBRIS     = 10,
        MAX_EXPLOSIONS = 10
    };

    ShipDesign();
    ShipDesign(const char* name, const char* path, const char* filename, bool secret=false);
    ~ShipDesign();

    // public interface:
    static void          Initialize();
    static void          Close();
    static bool          CheckName(const char* name);
    static ShipDesign*   Get(const char* design_name, const char* design_path=0);
    static ShipDesign*   FindModDesign(const char* design_name, const char* design_path=0);
    static void          ClearModCatalog();
    static int           GetDesignList(int type, List<Text>& designs); // never destroy the design list!

    static int           ClassForName(const char* name);
    static const char*   ClassName(int type);

    static int           LoadCatalog(const char* path, const char* file, bool mod=false);
    static void          LoadSkins(const char* path, const char* archive=0);
    static void          PreloadCatalog(int index=-1);
    static int           StandardCatalogSize();

    int operator == (const ShipDesign& s) const { return !strncmp(name, s.name, 31); }

    // Parser:
    void  ParseShip(TermDef* def);

    void  ParsePower(TermStruct* val);
    void  ParseDrive(TermStruct* val);
    void  ParseQuantumDrive(TermStruct* val);
    void  ParseFarcaster(TermStruct* val);
    void  ParseThruster(TermStruct* val);
    void  ParseNavlight(TermStruct* val);
    void  ParseFlightDeck(TermStruct* val);
    void  ParseLandingGear(TermStruct* val);
    void  ParseWeapon(TermStruct* val);
    void  ParseHardPoint(TermStruct* val);
    void  ParseSensor(TermStruct* val);
    void  ParseNavsys(TermStruct* val);
    void  ParseComputer(TermStruct* val);
    void  ParseShield(TermStruct* val);
    void  ParseDeathSpiral(TermStruct* val);
    void  ParseExplosion(TermStruct* val, int index);
    void  ParseDebris(TermStruct* val, int index);
    void  ParseLoadout(TermStruct* val);
    void  ParseMap(TermStruct* val);
    void  ParseSquadron(TermStruct* val);
    Skin* ParseSkin(TermStruct* val);
    void  ParseSkinMtl(TermStruct* val, Skin* skin);

    // general information:
    const char*       DisplayName() const;

    char              filename[64];
    char              path_name[64];
    char              name[64];
    char              display_name[64];
    char              abrv[16];
    int               type;
    float             scale;
    int               auto_roll;
    bool              valid;
    bool              secret;        // don't display in editor
    Text              description;   // background info for tactical reference

    // LOD representation:
    int               lod_levels;
    List<Model>       models[4];
    List<Point>       offsets[4];
    float             feature_size[4];
    List<Point>       spin_rates;

    // player selectable skins:
    List<Skin>        skins;
    const Skin*       FindSkin(const char* skin_name) const;

    // virtual cockpit:
    Model*            cockpit_model;
    float             cockpit_scale;

    // performance:
    float             vlimit;
    float             agility;
    float             air_factor;
    float             roll_rate;
    float             pitch_rate;
    float             yaw_rate;
    float             trans_x;
    float             trans_y;
    float             trans_z;
    float             turn_bank;
    Vec3              chase_vec;
    Vec3              bridge_vec;
    Vec3              beauty_cam;

    float             prep_time;

    // physical data:
    float             drag, roll_drag, pitch_drag, yaw_drag;
    float             arcade_drag;
    float             mass, integrity, radius;

    // aero data:
    float             CL, CD, stall;

    // weapons:
    int               primary;
    int               secondary;

    // drives:
    int               main_drive;

    // visibility:
    float             pcs;           // passive sensor cross section
    float             acs;           // active sensor cross section
    float             detet;         // maximum detection range
    float             e_factor[3];   // pcs scaling by emcon setting

    // ai settings:
    float             avoid_time;
    float             avoid_fighter;
    float             avoid_strike;
    float             avoid_target;
    float             commit_range;

    // death spriral sequence:
    float             death_spiral_time;
    float             explosion_scale;
    ShipExplosion     explosion[MAX_EXPLOSIONS];
    ShipDebris        debris[MAX_DEBRIS];

    List<PowerSource> reactors;
    List<Weapon>      weapons;
    List<HardPoint>   hard_points;
    List<Drive>       drives;
    List<Computer>    computers;
    List<FlightDeck>  flight_decks;
    List<NavLight>    navlights;
    QuantumDrive*     quantum_drive;
    Farcaster*        farcaster;
    Thruster*         thruster;
    Sensor*           sensor;
    NavSystem*        navsys;
    Shield*           shield;
    Model*            shield_model;
    Weapon*           decoy;
    Weapon*           probe;
    LandingGear*      gear;

    float             splash_radius;
    float             scuttle;
    float             repair_speed;
    int               repair_teams;
    bool              repair_auto;
    bool              repair_screen;
    bool              wep_screen;

    Text              bolt_hit_sound;
    Text              beam_hit_sound;

    Sound*            bolt_hit_sound_resource;
    Sound*            beam_hit_sound_resource;

    List<ShipLoad>    loadouts;
    List<Bitmap>      map_sprites;
    List<ShipSquadron>      squadrons;

    Bitmap            beauty;
    Bitmap            hud_icon;
};

// +--------------------------------------------------------------------+

#endif ShipDesign_h

