/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         WeaponDesign.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Weapon (gun or missile launcher) Design parameters class
*/

#ifndef WeaponDesign_h
#define WeaponDesign_h

#include "Types.h"
#include "Geometry.h"
#include "Color.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Bitmap;
class Model;
class Sound;

// +--------------------------------------------------------------------+

class WeaponDesign
{
public:
	static const char* TYPENAME() { return "WeaponDesign"; }

	enum CONSTANTS { 
		DMG_NORMAL=0,
		DMG_EMP   =1,
		DMG_POWER =2,
		MAX_STORES=8
	};

	WeaponDesign();
	~WeaponDesign();
	int operator == (const WeaponDesign& rhs) const { return (type == rhs.type) ||
		(name == rhs.name); }

	static void Initialize(const char* filename);
	static void Close();

	static WeaponDesign*    Get(int type);
	static WeaponDesign*    Find(const char* name);
	static WeaponDesign*    FindModDesign(const char* name);
	static void             ClearModCatalog();
	static int              GetDesignList(List<Text>& designs);

	// identification:
	int               type;             // unique id
	Text              name;
	Text              group;
	Text              description;      // background info for tactical reference
	bool              secret;           // don't display in the tactical reference

	bool              drone;            // visible to sensors?
	bool              primary;          // laser or missile?
	bool              beam;             // if laser, beam or bolt?
	bool              self_aiming;      // turret or fixed?
	bool              syncro;           // fire all barrels?
	bool              flak;             // splash damage
	int               guided;           // straight, pure pursuit, lead pursuit
	int               value;            // AI importance of system
	int               decoy_type;       // Ship Classifcation of decoy signature
	bool              probe;            // is sensor probe?
	DWORD             target_type;      // bitmask of acceptable target classes

	// for turrets:
	Vec3              muzzle_pts[MAX_STORES];    // default turret muzzle points
	int               nbarrels;                  // number of barrels on the turret

	// for missile hard points:
	bool              visible_stores;            // are external stores visible?
	Vec3              attachments[MAX_STORES];   // attachment points on the rail
	int               nstores;                   // number of stores on this hard point
	Vec3              eject;                     // eject velocity from rail in 3D

	// auto-aiming arc
	float             firing_cone;      // maximum deflection in any orientation
	float             aim_az_max;       // maximum deflection in azimuth
	float             aim_az_min;       // minimum deflection in azimuth
	float             aim_az_rest;      // azimuth of turret at rest
	float             aim_el_max;       // maximum deflection in elevation
	float             aim_el_min;       // minimum deflection in elevation
	float             aim_el_rest;      // elevation of turret at rest
	float             slew_rate;        // max rate of turret slew in rad/sec
	int               turret_axis;      // 0=az  1=el  2=not supported

	// functional parameters:
	float             capacity;         // full charge (joules)
	float             recharge_rate;    // watts
	float             refire_delay;     // seconds - mechanical limit
	float             salvo_delay;      // seconds - ai refire time
	int               ammo;
	int               ripple_count;     // number of rounds per salvo

	// carrying costs per shot:
	float             charge;           // energy cost of full charge
	float             min_charge;       // minimum energy needed to fire
	float             carry_mass;
	float             carry_resist;

	// shot parameters:
	int               damage_type;      // 0: normal, 1: EMP, 2: power drain
	float             damage;           // if beam, damage per second;
	// else,    damage per shot.
	float             penetration;      // ability to pierce shields, 1 is default
	float             speed;
	float             life;
	float             mass;
	float             drag;
	float             thrust;
	float             roll_rate;
	float             pitch_rate;
	float             yaw_rate;
	float             roll_drag;
	float             pitch_drag;
	float             yaw_drag;
	float             integrity;        // hit points for drones = 100
	float             lethal_radius;    // detonation range for missiles

	float             det_range;        // detonation range for cluster weapons
	Text              det_child;        // type of submunition
	int               det_count;        // number of submunitions
	float             det_spread;       // spread of submunition deployment

	// HUD parameters:
	float             min_range;
	float             max_range;
	float             max_track;

	// shot representation:
	int               graphic_type;     // sprite or blob?
	float             width;            // blob width
	float             length;           // blob length
	float             scale;            // sprite scale
	float             explosion_scale;  // scale factor for damage to this drone
	float             light;            // light emitted by shot
	Color             light_color;      // color of light emitted by shot
	float             flash_scale;      // size of muzzle flash sprite
	float             flare_scale;      // size of drive flare sprite

	float             spread_az;        // spread range in radians
	float             spread_el;        // spread range in radians

	Text              anim_frames[16];
	int               anim_length;
	Text              beauty;
	Text              bitmap;
	Text              model;
	Text              turret;
	Text              turret_base;
	Text              trail;
	Text              flash;
	Text              flare;
	Text              sound;

	Bitmap*           beauty_img;
	Bitmap*           animation;
	Bitmap*           shot_img;
	Bitmap*           trail_img;
	Bitmap*           flash_img;
	Bitmap*           flare_img;
	Model*            shot_model;
	Model*            turret_model;
	Model*            turret_base_model;
	Sound*            sound_resource;

	int               trail_length;
	float             trail_width;
	int               trail_dim;

private:
	static void LoadDesign(const char* path, const char* filename, bool mod=false);
};

#endif WeaponDesign_h

