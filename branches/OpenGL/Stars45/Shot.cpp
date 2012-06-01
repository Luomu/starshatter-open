/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright (C) 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Shot.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Laser and Missile class
*/

#include "MemDebug.h"
#include "Shot.h"
#include "Weapon.h"
#include "DriveSprite.h"
#include "SeekerAI.h"
#include "Sim.h"
#include "Ship.h"
#include "Trail.h"
#include "Random.h"
#include "AudioConfig.h"
#include "TerrainRegion.h"
#include "Terrain.h"

#include "Game.h"
#include "Bolt.h"
#include "Sprite.h"
#include "Solid.h"
#include "Light.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Sound.h"

// +--------------------------------------------------------------------+

Shot::Shot(const Point& pos, const Camera& shot_cam, WeaponDesign* dsn, const Ship* ship)
: first_frame(true), owner(ship), flash(0), flare(0), trail(0), sound(0), eta(0),
charge(1.0f), design(dsn), offset(1.0e5f), altitude_agl(-1.0e6f), hit_target(false)
{
	obj_type    = SimObject::SIM_SHOT;
	type        = design->type;
	primary     = design->primary;
	beam        = design->beam;
	base_damage = design->damage;
	armed       = false;

	radius      = 10.0f;

	if (primary || design->decoy_type || !design->guided) {
		straight = true;
		armed    = true;
	}

	cam.Clone(shot_cam);

	life     = design->life;
	velocity = cam.vpn() * (double) design->speed;

	MoveTo(pos);

	if (beam)
	origin = pos + (shot_cam.vpn() * -design->length);

	switch (design->graphic_type) {
	case Graphic::BOLT: {
			Bolt* s = new(__FILE__,__LINE__) Bolt(design->length, design->width, design->shot_img, 1);
			s->SetDirection(cam.vpn());
			rep = s;
		}
		break;

	case Graphic::SPRITE: {
			Sprite* s = 0;
			
			if (design->animation)
			s = new(__FILE__,__LINE__) DriveSprite(design->animation, design->anim_length);
			else
			s = new(__FILE__,__LINE__) DriveSprite(design->shot_img);

			s->Scale((double) design->scale);
			rep = s;
		}
		break;

	case Graphic::SOLID: {
			Solid* s = new(__FILE__,__LINE__) Solid;
			s->UseModel(design->shot_model);
			rep = s;

			radius = rep->Radius();
		}
		break;
	}

	if (rep)
	rep->MoveTo(pos);

	light = 0;

	if (design->light > 0) {
		light = new(__FILE__,__LINE__) Light(design->light);
		light->SetColor(design->light_color);
	}

	mass   = design->mass;
	drag   = design->drag;
	thrust = 0.0f;

	dr_drg = design->roll_drag;
	dp_drg = design->pitch_drag;
	dy_drg = design->yaw_drag;

	SetAngularRates((float) design->roll_rate, (float) design->pitch_rate, (float) design->yaw_rate);

	if (design->flash_img != 0) {
		flash = new(__FILE__,__LINE__) Sprite(design->flash_img);
		flash->Scale((double) design->flash_scale);
		flash->MoveTo(pos - cam.vpn() * design->length);
		flash->SetLuminous(true);
	}

	if (design->flare_img != 0) {
		flare = new(__FILE__,__LINE__) DriveSprite(design->flare_img);
		flare->Scale((double) design->flare_scale);
		flare->MoveTo(pos);
	}

	if (owner) {
		iff_code = (BYTE) owner->GetIFF();
		Observe((SimObject*) owner);
	}

	sprintf_s(name, "Shot(%s)", design->name.data());
}

// +--------------------------------------------------------------------+

Shot::~Shot()
{
	GRAPHIC_DESTROY(flash);
	GRAPHIC_DESTROY(flare);
	GRAPHIC_DESTROY(trail);

	if (sound) {
		sound->Stop();
		sound->Release();
	}
}

// +--------------------------------------------------------------------+

const char*
Shot::DesignName() const
{
	return design->name;
}

// +--------------------------------------------------------------------+

void
Shot::SetCharge(float c)
{
	charge = c;

	// trim beam life to amount of energy available:
	if (beam)
	life = design->life * charge / design->charge;
}

void
Shot::SetFuse(double seconds)
{
	if (seconds > 0 && !beam)
	life = seconds;
}

// +--------------------------------------------------------------------+

void
Shot::SeekTarget(SimObject* target, System* sub)
{
	if (dir && !primary) {
		SeekerAI*  seeker = (SeekerAI*) dir;
		SimObject* old_target = seeker->GetTarget();

		if (old_target->Type()==SimObject::SIM_SHIP) {
			Ship* tgt_ship = (Ship*) old_target;
			tgt_ship->DropThreat(this);
		}
	}

	delete dir;
	dir = 0;

	if (target) {
		SeekerAI* seeker = new(__FILE__,__LINE__) SeekerAI(this);
		seeker->SetTarget(target, sub);
		seeker->SetPursuit(design->guided);
		seeker->SetDelay(1);

		dir = seeker;

		if (!primary && target->Type()==SimObject::SIM_SHIP) {
			Ship* tgt_ship = (Ship*) target;
			tgt_ship->AddThreat(this);
		}
	}
}

bool
Shot::IsTracking(Ship* tgt) const
{
	return tgt && (GetTarget() == tgt);
}

SimObject*
Shot::GetTarget() const
{
	if (dir) {
		SeekerAI* seeker = (SeekerAI*) dir;

		if (seeker->GetDelay() <= 0)
		return seeker->GetTarget();
	}

	return 0;
}

bool
Shot::IsFlak() const
{
	return design && design->flak;
}

// +--------------------------------------------------------------------+

bool
Shot::IsHostileTo(const SimObject* o) const
{
	if (o) {
		if (o->Type() == SIM_SHIP) {
			Ship* s = (Ship*) o;

			if (s->IsRogue())
			return true;

			if (s->GetIFF() > 0 && s->GetIFF() != GetIFF())
			return true;
		}

		else if (o->Type() == SIM_SHOT || o->Type() == SIM_DRONE) {
			Shot* s = (Shot*) o;

			if (s->GetIFF() > 0 && s->GetIFF() != GetIFF())
			return true;
		}
	}

	return false;
}

// +--------------------------------------------------------------------+

void
Shot::ExecFrame(double seconds)
{
	altitude_agl = -1.0e6f;

	// add random flickering effect:
	double flicker = 0.75 + (double) rand() / 8e4;
	if (flicker > 1) flicker = 1;

	if (flare) {
		flare->SetShade(flicker);
	}
	else if (beam) {
		Bolt* blob = (Bolt*) rep;
		blob->SetShade(flicker);
		offset -= (float) (seconds * 10);
	}

	if (Game::Paused())
	return;

	if (beam) {
		if (!first_frame) {
			if (life > 0) {
				life -= seconds;

				if (life < 0)
				life = 0;
			}
		}
	}
	else {
		origin = Location();

		if (!first_frame)
		Physical::ExecFrame(seconds);
		else
		Physical::ExecFrame(0);

		double len = design->length;
		if (len < 50) len = 50;

		if (!trail && life > 0 && design->life - life > 0.2) {
			if (design->trail.length()) {
				trail = new(__FILE__,__LINE__) Trail(design->trail_img, design->trail_length);

				if (design->trail_width > 0)
				trail->SetWidth(design->trail_width);

				if (design->trail_dim > 0)
				trail->SetDim(design->trail_dim);

				trail->AddPoint(Location() + Heading() * -100);

				Scene* scene = 0;

				if (rep)
				scene = rep->GetScene();

				if (scene)
				scene->AddGraphic(trail);
			}
		}

		if (trail)
		trail->AddPoint(Location());

		if (!armed) {
			SeekerAI* seeker = (SeekerAI*) dir;

			if (seeker && seeker->GetDelay() <= 0)
			armed = true;
		}

		// handle submunitions:
		else if (design->det_range > 0 && design->det_count > 0) {
			if (dir && !primary) {
				SeekerAI*  seeker = (SeekerAI*) dir;
				SimObject* target = seeker->GetTarget();

				if (target) {
					double range = Point(Location() - target->Location()).length();

					if (range < design->det_range) {
						life = 0;

						Sim*           sim          = Sim::GetSim();
						WeaponDesign*  child_design = WeaponDesign::Find(design->det_child);

						if (sim && child_design) {
							double spread = design->det_spread;

							Camera aim_cam;
							aim_cam.Clone(Cam());
							aim_cam.LookAt(target->Location());

							for (int i = 0; i < design->det_count; i++) {
								Shot* child = sim->CreateShot(Location(), aim_cam, child_design,
								owner, owner->GetRegion());

								child->SetCharge(child_design->charge);

								if (child_design->guided)
								child->SeekTarget(target, seeker->GetSubTarget());

								if (child_design->beam)
								child->SetBeamPoints(Location(), target->Location());

								if (i) aim_cam.LookAt(target->Location());
								aim_cam.Pitch(Random(-spread, spread));
								aim_cam.Yaw(Random(-spread, spread));
							}
						}
					}
				}
			}
		}

		if (flash && !first_frame)
		GRAPHIC_DESTROY(flash);

		if (thrust < design->thrust)
		thrust += (float) (seconds * 5.0e3);
		else
		thrust = design->thrust;
	}

	first_frame = 0;   

	if (flare)
	flare->MoveTo(Location());
}

// +--------------------------------------------------------------------+

void
Shot::Disarm()
{
	if (armed && !primary) {
		armed = false;
		delete dir;
		dir = 0;
	}
}

void
Shot::Destroy()
{
	life = 0;
}

// +--------------------------------------------------------------------+

void
Shot::SetBeamPoints(const Point& from, const Point& to)
{
	if (beam) {
		MoveTo(to);
		origin = from;

		if (sound) {
			sound->SetLocation(from);
		}

		if (rep) {
			Bolt* s = (Bolt*) rep;
			s->SetEndPoints(from, to);

			double len = Point(to - from).length() / 500;
			s->SetTextureOffset(offset, offset + len);
		}
	}

	if (flash) {
		flash->MoveTo(origin);
	}
}

// +--------------------------------------------------------------------+

double
Shot::AltitudeMSL() const
{
	return Location().y;
}

double
Shot::AltitudeAGL() const
{
	if (altitude_agl < -1000) {
		Shot*    pThis   = (Shot*) this; // cast-away const
		Point    loc     = Location();
		Terrain* terrain = region->GetTerrain();

		if (terrain)
		pThis->altitude_agl = (float) (loc.y - terrain->Height(loc.x, loc.z));

		else
		pThis->altitude_agl = (float) loc.y;

		if (!_finite(altitude_agl)) {
			pThis->altitude_agl = 0.0f;
		}
	}

	return altitude_agl;
}

// +--------------------------------------------------------------------+

void
Shot::Initialize()
{
}

// +--------------------------------------------------------------------+

void
Shot::Close()
{
}

// +--------------------------------------------------------------------+

double
Shot::Damage() const
{
	double damage = 0;

	// beam damage based on length:
	if (beam) {
		double fade = 1;

		if (design) {
			// linear fade with distance:
			double len  = Point(origin - Location()).length();

			if (len > design->min_range)
			fade = (design->length - len) / (design->length - design->min_range);
		}

		damage = base_damage * charge * fade * Game::FrameTime();
	}

	// energy wep damage based on time:
	else if (primary) {
		damage = base_damage * charge * life;
	}

	// missile damage is constant:
	else {
		damage = base_damage * charge;
	}

	return damage;
}

double
Shot::Length() const
{
	if (design)
	return design->length;

	return 500;
}   

// +--------------------------------------------------------------------+

void
Shot::Activate(Scene& scene)
{
	SimObject::Activate(scene);

	if (trail)
	scene.AddGraphic(trail);

	if (flash)
	scene.AddGraphic(flash);

	if (flare)
	scene.AddGraphic(flare);

	if (first_frame) {
		if (design->sound_resource) {
			sound = design->sound_resource->Duplicate();

			if (sound) {
				long max_vol = AudioConfig::EfxVolume();
				long volume  = -1000;

				if (volume > max_vol)
				volume = max_vol;

				if (beam) {
					sound->SetLocation(origin);
					sound->SetVolume(volume);
					sound->Play();
				}
				else {
					sound->SetLocation(Location());
					sound->SetVolume(volume);
					sound->Play();
					sound = 0;  // fire and forget:
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Shot::Deactivate(Scene& scene)
{
	SimObject::Deactivate(scene);

	if (trail)
	scene.DelGraphic(trail);

	if (flash)
	scene.DelGraphic(flash);

	if (flare)
	scene.DelGraphic(flare);
}

// +--------------------------------------------------------------------+

int
Shot::GetIFF() const
{
	return iff_code;
}

// +--------------------------------------------------------------------+

Color
Shot::MarkerColor() const
{
	return Ship::IFFColor(GetIFF());
}

// +--------------------------------------------------------------------+

const char*
Shot::GetObserverName() const
{
	return name;
}

// +--------------------------------------------------------------------+

bool
Shot::Update(SimObject* obj)
{
	if (obj == (SimObject*) owner)
	owner = 0;

	return SimObserver::Update(obj);
}
