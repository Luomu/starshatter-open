/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         WeaponDesign.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Weapon Design parameters class
*/

#include "MemDebug.h"
#include "WeaponDesign.h"
#include "ShipDesign.h"
#include "Weapon.h"

#include "Game.h"
#include "Sprite.h"
#include "Light.h"
#include "Bitmap.h"
#include "Solid.h"
#include "Sound.h"
#include "DataLoader.h"

#include "ParseUtil.h"

// +--------------------------------------------------------------------+

static List<WeaponDesign> catalog;
static List<WeaponDesign> mod_catalog;
static bool               degrees;

#define GET_DEF_BOOL(x) if(defname==(#x))GetDefBool(design->x,pdef,filename)
#define GET_DEF_TEXT(x) if(defname==(#x))GetDefText(design->x,pdef,filename)
#define GET_DEF_NUM(x)  if(defname==(#x))GetDefNumber(design->x,pdef,filename)

// +--------------------------------------------------------------------+

WeaponDesign::WeaponDesign()
{
	type           = 0;
	secret         = 0;
	drone          = 0;
	primary        = 0;
	beam           = 0;
	flak           = 0;
	guided         = 0;
	self_aiming    = 0;
	syncro         = 0;
	value          = 0;
	decoy_type     = 0;
	probe          = 0;
	target_type    = 0;

	visible_stores = 0;
	nstores        = 0;
	nbarrels       = 0;

	recharge_rate  = 0.0f;
	refire_delay   = 0.0f;
	salvo_delay    = 0.0f;
	charge         = 0.0f;
	min_charge     = 0.0f;
	carry_mass     = 0.0f;
	carry_resist   = 0.0f;

	speed          = 0.0f;
	life           = 0.0f;
	mass           = 0.0f;
	drag           = 0.0f;
	thrust         = 0.0f;
	roll_rate      = 0.0f;
	pitch_rate     = 0.0f;
	yaw_rate       = 0.0f;
	roll_drag      = 0.0f;
	pitch_drag     = 0.0f;
	yaw_drag       = 0.0f;

	min_range      = 0.0f;
	max_range      = 0.0f;
	max_track      = 0.0f;

	graphic_type   = 0;
	width          = 0;
	length         = 0;

	scale          = 1.0f;
	explosion_scale = 0.0f;
	light          = 0.0f;
	light_color    = Color::White;
	flash_scale    = 0.0f;
	flare_scale    = 0.0f;

	spread_az      = 0.0f;
	spread_el      = 0.0f;

	beauty_img     = 0;
	turret_model   = 0;
	turret_base_model = 0;
	animation      = 0;
	anim_length    = 0;
	shot_img       = 0;
	shot_model     = 0;
	trail_img      = 0;
	flash_img      = 0;
	flare_img      = 0;
	sound_resource = 0;

	ammo           = -1;
	ripple_count   = 0;
	capacity       = 100.0f;
	damage         = 1.0f;
	damage_type    = 0;
	penetration    = 1.0f;
	firing_cone    = 0.0f;
	aim_az_max     = 1.5f;
	aim_az_min     = -1.5f;
	aim_az_rest    = 0.0f;
	aim_el_max     = 1.5f;
	aim_el_min     = -1.5f;
	aim_el_rest    = 0.0f;
	slew_rate      = (float) (60*DEGREES);
	turret_axis    = 0;
	lethal_radius  = 500.0f;
	integrity      = 100.0f;

	eject          = Vec3(0.0f, -100.0f, 0.0f);

	det_range      = 0.0f;
	det_count      = 0;
	det_spread     = (float) (PI/8);

	trail_length   = 0;
	trail_width    = 0;
	trail_dim      = 0;

	for (int i = 0; i < MAX_STORES; i++) {
		muzzle_pts[i]  = Vec3(0,0,0);
		attachments[i] = Vec3(0,0,0);
	}
}

WeaponDesign::~WeaponDesign()
{
	delete turret_model;
	delete turret_base_model;
	delete shot_model;
	delete sound_resource;
}

// +--------------------------------------------------------------------+

void
WeaponDesign::Initialize(const char* filename)
{
	Print("Loading Weapon Designs '%s'\n", filename);
	LoadDesign("Weapons/", filename);
}

// +--------------------------------------------------------------------+

void
WeaponDesign::Close()
{
	catalog.destroy();
	mod_catalog.destroy();
}

void
WeaponDesign::ClearModCatalog()
{
	mod_catalog.destroy();
}

// +--------------------------------------------------------------------+

void
WeaponDesign::LoadDesign(const char* path, const char* filename, bool mod)
{
	// Load Design File:
	DataLoader* loader = DataLoader::GetLoader();
	loader->SetDataPath(path);

	BYTE* block;
	int blocklen = loader->LoadBuffer(filename, block, true);

	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
	Term*  term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse '%s'\n", filename);
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "WEAPON") {
			Print("ERROR: invalid weapon design file '%s'\n", filename);
			return;
		}
	}

	int type = 1;
	degrees  = false;

	do {
		delete term;

		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				Text defname = def->name()->value();
				defname.setSensitive(false);

				if (defname == "primary"   ||
						defname == "missile"   ||
						defname == "drone"     ||
						defname == "beam")       {

					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: weapon structure missing in '%s'\n", filename);
					}
					else {
						TermStruct*   val    = def->term()->isStruct();
						WeaponDesign* design = new(__FILE__,__LINE__) WeaponDesign;

						design->type = type++;
						if (defname == "primary") {
							design->primary = true;
						}

						else if (defname == "beam") {
							design->primary = true;
							design->beam    = true;
							design->guided  = true;

							design->spread_az = 0.15f;
							design->spread_el = 0.15f;

						}

						else if (defname == "drone") {
							design->drone       = true;
							design->penetration = 5.0f;
						}

						else {   // missile
							design->penetration = 5.0f;
						}

						float sound_min_dist = 1.0f;
						float sound_max_dist = 100.0e3f;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								defname = pdef->name()->value();
								defname.setSensitive(false);

								GET_DEF_TEXT(name);
								else GET_DEF_TEXT(group);
								else GET_DEF_TEXT(description);
								else GET_DEF_NUM (guided);
								else GET_DEF_BOOL(self_aiming);
								else GET_DEF_BOOL(flak);
								else GET_DEF_BOOL(syncro);
								else GET_DEF_BOOL(visible_stores);
								else GET_DEF_NUM (value);

								else if (defname==("degrees")) {
									GetDefBool(degrees,pdef,filename);
								}

								else if (defname==("secret")) {
									GetDefBool(design->secret,pdef,filename);
								}

								else if (defname==("aim_az_max")) {
									GetDefNumber(design->aim_az_max,pdef,filename);
									if (degrees) design->aim_az_max *= (float) DEGREES;
									design->aim_az_min = 0.0f - design->aim_az_max;
								}

								else if (defname==("aim_el_max")) {
									GetDefNumber(design->aim_el_max,pdef,filename);
									if (degrees) design->aim_el_max *= (float) DEGREES;
									design->aim_el_min = 0.0f - design->aim_el_max;
								}

								else if (defname==("aim_az_min")) {
									GetDefNumber(design->aim_az_min,pdef,filename);
									if (degrees) design->aim_az_min *= (float) DEGREES;
								}

								else if (defname==("aim_el_min")) {
									GetDefNumber(design->aim_el_min,pdef,filename);
									if (degrees) design->aim_el_min *= (float) DEGREES;
								}

								else if (defname==("aim_az_rest")) {
									GetDefNumber(design->aim_az_rest,pdef,filename);
									if (degrees) design->aim_az_rest *= (float) DEGREES;
								}

								else if (defname==("aim_el_rest")) {
									GetDefNumber(design->aim_el_rest,pdef,filename);
									if (degrees) design->aim_el_rest *= (float) DEGREES;
								}

								else if (defname==("spread_az")) {
									GetDefNumber(design->spread_az,pdef,filename);
									if (degrees) design->spread_az *= (float) DEGREES;
								}

								else if (defname==("spread_el")) {
									GetDefNumber(design->spread_el,pdef,filename);
									if (degrees) design->spread_el *= (float) DEGREES;
								}

								else GET_DEF_NUM (capacity);
								else GET_DEF_NUM (recharge_rate);
								else GET_DEF_NUM (refire_delay);
								else GET_DEF_NUM (salvo_delay);
								else GET_DEF_NUM (ammo);
								else GET_DEF_NUM (ripple_count);
								else GET_DEF_NUM (charge);
								else GET_DEF_NUM (min_charge);
								else GET_DEF_NUM (carry_mass);
								else GET_DEF_NUM (carry_resist);
								else GET_DEF_NUM (damage);
								else GET_DEF_NUM (penetration);
								else GET_DEF_NUM (speed);
								else GET_DEF_NUM (life);
								else GET_DEF_NUM (mass);
								else GET_DEF_NUM (drag);
								else GET_DEF_NUM (thrust);
								else GET_DEF_NUM (roll_rate);
								else GET_DEF_NUM (pitch_rate);
								else GET_DEF_NUM (yaw_rate);
								else GET_DEF_NUM (roll_drag);
								else GET_DEF_NUM (pitch_drag);
								else GET_DEF_NUM (yaw_drag);
								else GET_DEF_NUM (lethal_radius);
								else GET_DEF_NUM (integrity);

								else GET_DEF_NUM (det_range);
								else GET_DEF_NUM (det_count);
								else GET_DEF_NUM (det_spread);
								else GET_DEF_TEXT(det_child);

								else GET_DEF_NUM (slew_rate);

								else GET_DEF_NUM (min_range);
								else GET_DEF_NUM (max_range);
								else GET_DEF_NUM (max_track);
								
								else GET_DEF_NUM (graphic_type);
								else GET_DEF_NUM (width);
								else GET_DEF_NUM (length);
								else GET_DEF_NUM (scale);
								else GET_DEF_NUM (explosion_scale);
								else GET_DEF_NUM (light);
								else GET_DEF_NUM (flash_scale);
								else GET_DEF_NUM (flare_scale);

								else GET_DEF_NUM (trail_length);
								else GET_DEF_NUM (trail_width);
								else GET_DEF_NUM (trail_dim);

								else GET_DEF_TEXT(beauty);
								else GET_DEF_TEXT(bitmap);
								else GET_DEF_TEXT(turret);
								else GET_DEF_TEXT(turret_base);
								else GET_DEF_TEXT(model);
								else GET_DEF_TEXT(trail);
								else GET_DEF_TEXT(flash);
								else GET_DEF_TEXT(flare);
								else GET_DEF_TEXT(sound);
								else GET_DEF_BOOL(probe);
								else GET_DEF_NUM (turret_axis);
								else GET_DEF_NUM (target_type);

								else if (defname == "animation") {
									if (design->anim_length < 16) {
										GetDefText(design->anim_frames[design->anim_length++], pdef, filename);
									}
									else {
										Print("WARNING: too many animation frames for weapon '%s' in '%s'\n",
										(const char*) design->name, filename);
									}
								}

								else if (defname == "light_color")
								GetDefColor(design->light_color, pdef, filename);

								else if (defname == "sound_min_dist")
								GetDefNumber(sound_min_dist,pdef,filename);

								else if (defname == "sound_max_dist")
								GetDefNumber(sound_max_dist,pdef,filename);

								else if (defname == "muzzle") {
									if (design->nbarrels < MAX_STORES) {
										Vec3 a;
										GetDefVec(a, pdef, filename);
										design->muzzle_pts[design->nbarrels++] = a;
									}
									else {
										Print("WARNING: too many muzzles for weapon '%s' in '%s'\n",
										(const char*) design->name, filename);
									}
								}

								else if (defname == "attachment") {
									if (design->nstores < MAX_STORES) {
										Vec3 a;
										GetDefVec(a, pdef, filename);
										design->attachments[design->nstores++] = a;
									}
									else {
										Print("WARNING: too many attachments for weapon '%s' in '%s'\n",
										(const char*) design->name, filename);
									}
								}

								else if (defname == "eject")
								GetDefVec(design->eject,pdef,filename);

								else if (defname == "decoy") {
									char typestr[32];
									GetDefText(typestr, pdef, filename);
									design->decoy_type = ShipDesign::ClassForName(typestr);
								}

								else if (defname == "damage_type") {
									char typestr[32];
									GetDefText(typestr, pdef, filename);

									if (!_stricmp(typestr, "normal"))
									design->damage_type = DMG_NORMAL;

									else if (!_stricmp(typestr, "emp"))
									design->damage_type = DMG_EMP;

									else if (!_stricmp(typestr, "power"))
									design->damage_type = DMG_POWER;

									else
									Print("WARNING: unknown weapon damage type '%s' in '%s'\n",
									typestr, filename);
								}


								else {
									Print("WARNING: parameter '%s' ignored in '%s'\n",
									defname.data(), filename);
								}
							}
							else {
								Print("WARNING: term ignored in '%s'\n", filename);
								val->elements()->at(i)->print();
							}
						}

						if (design->description.length()) {
							design->description = Game::GetText(design->description);
						}

						if (design->anim_length > 0) {
							design->animation = new(__FILE__,__LINE__) Bitmap[design->anim_length];
							for (int i = 0; i < design->anim_length; i++) {
								Bitmap* p = design->animation + i;
								loader->LoadBitmap(design->anim_frames[i], *p, Bitmap::BMP_TRANSLUCENT);
								p->MakeTexture();
							}
						}

						else if (design->bitmap.length()) {
							loader->LoadTexture(design->bitmap, design->shot_img, Bitmap::BMP_TRANSLUCENT);
						}

						if (design->beauty.length()) {
							loader->LoadTexture(design->beauty, design->beauty_img, Bitmap::BMP_TRANSLUCENT);
						}

						if (design->turret.length()) {
							Text        p;
							Text        t = design->turret;
							const char* s = strrchr(t.data(), '/');

							if (s) {
								p = Text(path) + t.substring(0, s-t.data()+1);
								t = t.substring(s-t.data()+1, t.length());
							}
							else {
								s = strrchr(t.data(), '\\');

								if (s) {
									p = Text(path) + t.substring(0, s-t.data()+1);
									t = t.substring(s-t.data()+1, t.length());
								}
							}

							if (p.length())
							loader->SetDataPath(p);

							design->turret_model = new(__FILE__,__LINE__) Model;
							design->turret_model->Load(t, design->scale);

							if (design->turret_base.length()) {
								t = design->turret_base;
								s = strrchr(t.data(), '/');

								if (s) {
									p = Text(path) + t.substring(0, s-t.data()+1);
									t = t.substring(s-t.data()+1, t.length());
								}
								else {
									s = strrchr(t.data(), '\\');

									if (s) {
										p = Text(path) + t.substring(0, s-t.data()+1);
										t = t.substring(s-t.data()+1, t.length());
									}
								}

								if (p.length())
								loader->SetDataPath(p);

								design->turret_base_model = new(__FILE__,__LINE__) Model;
								design->turret_base_model->Load(t, design->scale);
							}

							loader->SetDataPath(path);
						}

						if (design->model.length()) {
							Text        p;
							Text        t = design->model;
							const char* s = strrchr(t.data(), '/');

							if (s) {
								p = Text(path) + t.substring(0, s-t.data()+1);
								t = t.substring(s-t.data()+1, t.length());
							}
							else {
								s = strrchr(t.data(), '\\');

								if (s) {
									p = Text(path) + t.substring(0, s-t.data()+1);
									t = t.substring(s-t.data()+1, t.length());
								}
							}

							if (p.length())
							loader->SetDataPath(p);

							design->shot_model = new(__FILE__,__LINE__) Model;
							design->shot_model->Load(t, design->scale);

							loader->SetDataPath(path);
						}

						if (design->trail.length()) {
							loader->LoadTexture(design->trail, design->trail_img, Bitmap::BMP_TRANSLUCENT);
						}

						if (design->flash.length()) {
							loader->LoadTexture(design->flash, design->flash_img, Bitmap::BMP_TRANSLUCENT);
						}

						if (design->flare.length()) {
							loader->LoadTexture(design->flare, design->flare_img, Bitmap::BMP_TRANSLUCENT);
						}

						if (design->sound.length()) {
							int SOUND_FLAGS = Sound::LOCALIZED | Sound::LOC_3D;

							if (design->beam)
							SOUND_FLAGS = Sound::LOCALIZED | Sound::LOC_3D | Sound::LOCKED;

							if (strstr(path, "Mods") == 0)
							loader->SetDataPath("Sounds/");
							loader->LoadSound(design->sound, design->sound_resource, SOUND_FLAGS);
							loader->SetDataPath(path);

							if (design->sound_resource) {
								design->sound_resource->SetMinDistance(sound_min_dist);
								design->sound_resource->SetMaxDistance(sound_max_dist);
							}
						}
						
						if (design->max_range == 0.0f)
						design->max_range = design->speed * design->life;

						if (design->max_track == 0.0f)
						design->max_track = 3.0f * design->max_range;

						if (design->probe && design->lethal_radius < 1e3)
						design->lethal_radius = 50e3f;

						if (design->beam)
						design->flak = false;

						if (design->self_aiming) {
							if (fabs(design->aim_az_max) > design->firing_cone)
							design->firing_cone = (float) fabs(design->aim_az_max);

							if (fabs(design->aim_az_min) > design->firing_cone)
							design->firing_cone = (float) fabs(design->aim_az_min);

							if (fabs(design->aim_el_max) > design->firing_cone)
							design->firing_cone = (float) fabs(design->aim_el_max);

							if (fabs(design->aim_el_min) > design->firing_cone)
							design->firing_cone = (float) fabs(design->aim_el_min);
						}

						if (mod)
						mod_catalog.append(design);
						else
						catalog.append(design);
					}
				}
				
				else
				Print("WARNING: unknown definition '%s' in '%s'\n",
				def->name()->value().data(), filename);
			}
			else {
				Print("WARNING: term ignored in '%s'\n", filename);
				term->print();
			}
		}
	}
	while (term);

	loader->ReleaseBuffer(block);
	loader->SetDataPath(0);
}

// +--------------------------------------------------------------------+

WeaponDesign*
WeaponDesign::Get(int type)
{
	WeaponDesign  test;
	test.type = type;

	WeaponDesign* result = catalog.find(&test);

	if (!result)
	result = mod_catalog.find(&test);

	return result;
}

// +--------------------------------------------------------------------+

WeaponDesign*
WeaponDesign::Find(const char* name)
{
	for (int i = 0; i < catalog.size(); i++) {
		WeaponDesign* d = catalog.at(i);

		if (d->name == name) {
			return d;
		}
	}

	for (int i = 0; i < mod_catalog.size(); i++) {
		WeaponDesign* d = mod_catalog.at(i);

		if (d->name == name) {
			return d;
		}
	}

	Print("WeaponDesign: no catalog entry for design '%s', checking mods...\n", name);
	return WeaponDesign::FindModDesign(name);
}

WeaponDesign*
WeaponDesign::FindModDesign(const char* name)
{
	Text fname = name;
	fname += ".def";

	LoadDesign("Mods/Weapons/", fname, true);

	for (int i = 0; i < mod_catalog.size(); i++) {
		WeaponDesign* d = mod_catalog.at(i);

		if (d->name == name) {
			Print("WeaponDesign: found mod weapon '%s'\n", d->name);
			return d;
		}
	}

	Print("WeaponDesign: no mod found for design '%s'\n", name);
	return 0;
}

// +--------------------------------------------------------------------+

int
WeaponDesign::GetDesignList(List<Text>& designs)
{
	designs.clear();

	for (int i = 0; i < catalog.size(); i++) {
		WeaponDesign* design = catalog[i];
		designs.append(&design->name);
	}

	for (int i = 0; i < mod_catalog.size(); i++) {
		WeaponDesign* design = mod_catalog[i];
		designs.append(&design->name);
	}

	return designs.size();
}
