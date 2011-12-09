/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Weapon.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Weapon class
*/

#include "MemDebug.h"
#include "Weapon.h"
#include "Shot.h"
#include "Drone.h"
#include "Contact.h"
#include "Ship.h"
#include "Sim.h"
#include "SimEvent.h"
#include "Random.h"

#include "NetGame.h"
#include "NetUtil.h"

#include "Game.h"
#include "Solid.h"

// +----------------------------------------------------------------------+

Weapon::Weapon(WeaponDesign* d, int nmuz, Vec3* muzzles, double az, double el)
: System(WEAPON, d->type, d->name, d->value,
d->capacity, d->capacity, d->recharge_rate),
design(d), group(d->group), ammo(-1), ripple_count(0),
aim_azimuth((float) az), aim_elevation((float) el),
old_azimuth(0.0f), old_elevation(0.0f), aim_time(0),
enabled(true), refire(0.0f),
mass(d->carry_mass), resist(d->carry_resist),
guided(d->guided), shot_speed(d->speed),
active_barrel(0), locked(false), centered(false), firing(false), blocked(false),
index(0),  target(0), subtarget(0), beams(0), orders(MANUAL),
control(SINGLE_FIRE), sweep(SWEEP_TIGHT), turret(0), turret_base(0)
{
	ZeroMemory(visible_stores, sizeof(visible_stores));

	if (design->primary)
	abrv = Game::GetText("sys.weapon.primary.abrv");
	else   
	abrv = Game::GetText("sys.weapon.secondary.abrv");

	nbarrels = nmuz;

	if (nbarrels > MAX_BARRELS)
	nbarrels = MAX_BARRELS;

	if (nbarrels == 0 && design->nbarrels > 0) {
		nbarrels = design->nbarrels;

		for (int i = 0; i < nbarrels; i++)
		muzzle_pts[i] = rel_pts[i] = design->muzzle_pts[i] * design->scale;

		ammo = design->ammo * nbarrels;
	}
	else if (nbarrels == 1 && design->nstores > 0) {
		nbarrels = design->nstores;

		for (int i = 0; i < nbarrels; i++)
		muzzle_pts[i] = rel_pts[i] = (muzzles[0] + design->attachments[i]);

		ammo = nbarrels;
	}
	else {
		for (int i = 0; i < nbarrels; i++)
		muzzle_pts[i] = rel_pts[i] = muzzles[i];

		ammo = design->ammo * nbarrels;
	}

	if (design->syncro)
	active_barrel = -1;

	emcon_power[0] = 0;
	emcon_power[1] = 0;
	emcon_power[2] = 100;

	aim_az_max  = design->aim_az_max;
	aim_az_min  = design->aim_az_min;
	aim_az_rest = design->aim_az_rest;

	aim_el_max  = design->aim_el_max;
	aim_el_min  = design->aim_el_min;
	aim_el_rest = design->aim_el_rest;
}

// +----------------------------------------------------------------------+

Weapon::Weapon(const Weapon& w)
: System(w), design(w.design), ammo(-1), ripple_count(0),
enabled(true), refire(0.0f),
mass(w.mass), resist(w.resist),
aim_azimuth(w.aim_azimuth), aim_elevation(w.aim_elevation),
old_azimuth(0.0f), old_elevation(0.0f), aim_time(0),
guided(w.guided), shot_speed(w.shot_speed),
active_barrel(0), locked(false), centered(false), firing(false), blocked(false),
target(0), subtarget(0), beams(0), orders(MANUAL), 
control(SINGLE_FIRE), sweep(SWEEP_TIGHT), group(w.group),
aim_az_max(w.aim_az_max), aim_az_min(w.aim_az_min), aim_az_rest(w.aim_az_rest),
aim_el_max(w.aim_el_max), aim_el_min(w.aim_el_min), aim_el_rest(w.aim_el_rest),
turret(0), turret_base(0)
{
	Mount(w);
	ZeroMemory(visible_stores, sizeof(visible_stores));

	nbarrels = w.nbarrels;

	for (int i = 0; i < nbarrels; i++) {
		muzzle_pts[i] = rel_pts[i] = w.muzzle_pts[i];
	}

	ammo = design->ammo * nbarrels;

	if (design->syncro)
	active_barrel = -1;

	if (design->beam) {
		beams = new(__FILE__,__LINE__) Shot* [nbarrels];
		ZeroMemory(beams, sizeof(Shot*) * nbarrels);
	}

	if (aim_az_rest >= 2*PI)
	aim_az_rest = design->aim_az_rest;

	if (aim_el_rest >= 2*PI)
	aim_el_rest = design->aim_el_rest;
}

// +--------------------------------------------------------------------+

Weapon::~Weapon()
{
	if (beams) {
		for (int i = 0; i < nbarrels; i++) {
			if (beams[i]) {
				Ignore(beams[i]);
				delete beams[i];
				beams[i] = 0;
			}
		}

		delete [] beams;
	}

	GRAPHIC_DESTROY(turret);
	GRAPHIC_DESTROY(turret_base);

	for (int i = 0; i < MAX_BARRELS; i++)
	GRAPHIC_DESTROY(visible_stores[i]);
}

// +--------------------------------------------------------------------+

bool
Weapon::IsPrimary() const
{
	return design->primary;
}

bool
Weapon::IsDrone()   const
{
	return design->drone;
}

bool
Weapon::IsDecoy()   const
{
	return design->decoy_type != 0;
}

bool
Weapon::IsProbe()   const
{
	return design->probe != 0;
}

bool
Weapon::IsMissile() const
{
	return !design->primary;
}

bool
Weapon::IsBeam()    const
{
	return design->beam;
}

// +--------------------------------------------------------------------+

Shot*
Weapon::GetBeam(int i)
{
	if (beams && i >= 0 && i < nbarrels)
	return beams[i];

	return 0;
}

// +--------------------------------------------------------------------+

void
Weapon::SetOwner(Ship* s)
{
	ship = s;

	if (design->turret_model) {
		Solid* t = new(__FILE__,__LINE__) Solid;
		t->UseModel(design->turret_model);
		turret = t;
	}

	if (design->turret_base_model) {
		Solid* t = new(__FILE__,__LINE__) Solid;
		t->UseModel(design->turret_base_model);
		turret_base = t;
	}

	if (!design->primary          && 
			design->visible_stores   &&
			ammo == nbarrels         &&
			design->shot_model != 0)
	{
		for (int i = 0; i < nbarrels; i++) {
			Solid* s = new(__FILE__,__LINE__) Solid;
			s->UseModel(design->shot_model);

			visible_stores[i] = s;
		}
	}
}

Solid*
Weapon::GetTurret()
{
	return turret;
}

Solid*
Weapon::GetTurretBase()
{
	return turret_base;
}

Solid*
Weapon::GetVisibleStore(int i)
{
	if (i >= 0 && i < MAX_BARRELS)
	return visible_stores[i];

	return 0;
}

void
Weapon::SetAmmo(int a)
{
	if (a >= 0) {
		if (active_barrel >= 0 && design->visible_stores) {
			while (a < ammo) {
				if (active_barrel >= nbarrels)
				active_barrel = 0;

				if (visible_stores[active_barrel]) {
					GRAPHIC_DESTROY(visible_stores[active_barrel]);
					active_barrel++;
					ammo--;
				}
			}
		}

		ammo = a;
	}
}

// +--------------------------------------------------------------------+

void
Weapon::ExecFrame(double seconds)
{
	System::ExecFrame(seconds);

	if (refire > 0)
	refire -= (float) seconds;

	locked   = false;
	centered = false;

	if (!ship)
	return;

	if (orders == POINT_DEFENSE && enabled)
	SelectTarget();

	if (beams && !target) {
		for (int i = 0; i < nbarrels; i++) {
			if (beams[i]) {
				// aim beam straight:
				Aim();
				SetBeamPoints(false);
				return;
			}
		}
	}

	if (design->self_aiming) {
		Track(target, subtarget);
	}
	else if (turret) {
		ZeroAim();
	}

	if (ship->CheckFire())
	return;

	// aim beam at target:
	bool aim_beams = false;

	if (beams) {
		for (int i = 0; i < nbarrels; i++) {
			if (beams[i]) {
				aim_beams = true;
				SetBeamPoints(true);
				break;
			}
		}
	}

	if (!aim_beams) {
		if (ripple_count > 0) {
			if (Fire())
			ripple_count--;
		}

		else if (locked && !blocked) {
			if (!ship->IsHostileTo(target))
			return;

			if (orders == AUTO && centered) {
				if (energy >= design->charge &&
						(ammo < 0 || target && target->Integrity() >= 1) && 
						objective.length() < design->max_range)
				Fire();
			}

			else if (orders == POINT_DEFENSE) {
				if (energy >= design->min_charge &&
						(ammo < 0 || target && target->Integrity() >= 1) && 
						objective.length() < design->max_range)
				Fire();
			}
		}
	}
}

// +----------------------------------------------------------------------+

void
Weapon::Distribute(double delivered_energy, double seconds)
{
	if (UsesWatts()) {
		if (seconds < 0.01)
		seconds = 0.01;

		// convert Joules to Watts:
		energy = (float) (delivered_energy/seconds);
	}

	else if (!Game::Paused()) {
		energy += (float) (delivered_energy * 1.25);

		if (energy > capacity)
		energy = capacity;

		else if (energy < 0)
		energy = 0.0f;
	}
}


// +--------------------------------------------------------------------+

bool
Weapon::Update(SimObject* obj)
{
	if (obj == target) {
		target = 0;
	}

	else if (beams) {
		for (int i = 0; i < nbarrels; i++)
		if (obj == beams[i])
		beams[i] = 0;
	}

	return SimObserver::Update(obj);   
}

const char*
Weapon::GetObserverName() const
{
	static char name[256];
	sprintf_s(name, "Weapon %s", design->name);
	return name;
}

// +--------------------------------------------------------------------+

void
Weapon::SetFiringOrders(int o)
{
	if (o >= MANUAL && o <= POINT_DEFENSE)
	orders = o;
}

void
Weapon::SetControlMode(int m)
{
	if (m >= SINGLE_FIRE && m <= SALVO_FIRE)
	control = m;
}

void
Weapon::SetSweep(int s)
{
	if (s >= SWEEP_NONE && s <= SWEEP_WIDE)
	sweep = s;
}

// +--------------------------------------------------------------------+

bool
Weapon::CanTarget(DWORD classification) const
{
	return (design->target_type & classification) ? true : false;
}

// +--------------------------------------------------------------------+

void
Weapon::SetTarget(SimObject* targ, System* sub)
{
	// check self targeting:
	if (targ == (SimObject*) ship)
	return;

	// check target class filter:
	if (targ) {
		switch (targ->Type()) {
		case SimObject::SIM_SHIP: {
				Ship* tgt_ship = (Ship*) targ;

				if ((tgt_ship->Class() & design->target_type) == 0)
				return;
			}
			break;

		case SimObject::SIM_SHOT:
			return;

		case SimObject::SIM_DRONE: {
				if ((design->target_type & Ship::DRONE) == 0)
				return;
			}
			break;

		default: 
			return;
		}
	}

	// if ok, target this object:
	if (target != targ) {
		target = targ;
		
		if (target)
		Observe(target);
	}

	subtarget = sub;
}

// +--------------------------------------------------------------------+

void
Weapon::SelectTarget()
{
	bool        select_locked = false;
	SimObject*  targ = 0;
	double      dist = 1e9;
	double      az   = 0;
	double      el   = 0;

	if (ammo && enabled && (availability > crit_level)) {
		ZeroAim();

		ListIter<Contact> contact = ship->ContactList();

		// lock onto any threatening shots first (if we can):
		if (design->target_type & Ship::DRONE) {
			while (++contact) {
				Shot* c_shot = contact->GetShot();

				if (c_shot && contact->Threat(ship)) {

					// distance from self to target:
					double distance = Point(c_shot->Location() - muzzle_pts[0]).length();

					if (distance > design->min_range && 
							distance < design->max_range && 
							distance < dist) {
						// check aim basket:
						select_locked = CanLockPoint(c_shot->Location(), az, el);

						if (select_locked) {
							targ = c_shot;
							dist = distance;
						}
					}
				}
			}
		}

		// lock onto a threatening ship only if it is (much) closer:
		dist *= 0.2;
		contact.reset();
		while (++contact) {
			Ship* c_ship  = contact->GetShip();

			if (!c_ship) continue;

			// can we lock onto this target?
			if ((c_ship->IsRogue() || c_ship->GetIFF() > 0 && c_ship->GetIFF() != ship->GetIFF()) &&
					(c_ship->Class() & design->target_type) &&
					c_ship->Weapons().size() > 0) {
				// distance from self to target:
				double distance = Point(c_ship->Location() - muzzle_pts[0]).length();

				if (distance < design->max_range && distance < dist) {
					// check aim basket:
					select_locked = CanLockPoint(c_ship->Location(), az, el);

					if (select_locked) {
						targ = c_ship;
						dist = distance;
					}
				}
			}
		}
	}

	if (!ammo || !enabled) {
		SetTarget(0,0);
		locked = false;
	}

	else {
		SetTarget(targ, 0);
	}
}

// +--------------------------------------------------------------------+

int
Weapon::Track(SimObject* targ, System* sub)
{
	if (ammo && enabled && (availability > crit_level)) {
		firing = 0;
		Aim();
	}
	else if (turret) {
		ZeroAim();
	}

	return locked;
}

// +--------------------------------------------------------------------+

Shot*
Weapon::Fire()
{
	if (Game::Paused())
	return 0;

	if (ship && ship->CheckFire())
	return 0;

	if (ship->IsStarship() && target && !centered)
	return 0;

	if (beams && active_barrel >= 0 && active_barrel < nbarrels && beams[active_barrel])
	return 0;

	Shot* shot = 0;

	if (ammo && enabled &&
			(refire <= 0) && (energy > design->min_charge) &&
			(availability > crit_level)) {

		refire = design->refire_delay;

		NetGame* net_game   = NetGame::GetInstance();
		bool     net_client = net_game ? net_game->IsClient() : false;

		// all barrels
		if (active_barrel < 0) {
			if (net_client || IsPrimary())
			NetUtil::SendWepTrigger(this, nbarrels);

			if (!net_game || IsPrimary()) {
				for (int i = 0; i < nbarrels; i++)
				shot = FireBarrel(i);
			}

			else if (net_game && net_game->IsServer() && IsMissile()) {
				for (int i = 0; i < nbarrels; i++) {
					shot = FireBarrel(i);
					NetUtil::SendWepRelease(this, shot);
				}
			}
		}

		// single barrel
		else {
			if (net_client || IsPrimary())
			NetUtil::SendWepTrigger(this, nbarrels);

			if (!net_game || IsPrimary()) {
				shot = FireBarrel(active_barrel++);
			}

			else if (net_game && net_game->IsServer() && IsMissile()) {
				shot = FireBarrel(active_barrel++);
				NetUtil::SendWepRelease(this, shot);
			}

			if (active_barrel >= nbarrels) {
				active_barrel = 0;
				refire += design->salvo_delay;
			}
		}   

		if (design->ripple_count > 0 && ripple_count <= 0)
		ripple_count = design->ripple_count-1;

		if (status != NOMINAL)
		refire *= 2;
	}

	return shot;
}

// +--------------------------------------------------------------------+

Shot*
Weapon::NetFirePrimary(SimObject* tgt, System* sub, int count)
{
	Shot* shot = 0;

	if (!IsPrimary() || Game::Paused())
	return shot;

	if (active_barrel < 0 || active_barrel >= nbarrels)
	active_barrel = 0;

	target    = tgt;
	subtarget = sub;
	aim_time  = 0;

	for (int i = 0; i < count; i++) {
		shot = FireBarrel(active_barrel++);

		if (active_barrel >= nbarrels) {
			active_barrel = 0;
			refire += design->salvo_delay;
		}
	}

	if (target)
	Observe(target);

	return shot;
}

Shot*
Weapon::NetFireSecondary(SimObject* tgt, System* sub, DWORD objid)
{
	Shot* shot = 0;

	if (IsPrimary() || Game::Paused())
	return shot;

	if (active_barrel < 0 || active_barrel >= nbarrels)
	active_barrel = 0;

	target    = tgt;
	subtarget = sub;
	aim_time  = 0;

	shot = FireBarrel(active_barrel++);

	if (active_barrel >= nbarrels) {
		active_barrel = 0;
		refire += design->salvo_delay;
	}

	if (shot)
	shot->SetObjID(objid);

	if (target)
	Observe(target);

	return shot;
}

// +--------------------------------------------------------------------+

Shot*
Weapon::FireBarrel(int n)
{
	const Point&   base_vel = ship->Velocity();
	Shot*          shot     = 0;
	SimRegion*     region   = ship->GetRegion();

	if (!region || n < 0 || n >= nbarrels || Game::Paused())
	return 0;

	firing = 1;
	Aim();

	Camera rail_cam;
	rail_cam.Clone(aim_cam);

	Point shotpos = muzzle_pts[n];
	if (design->length > 0)
	shotpos = shotpos + aim_cam.vpn() * design->length;

	// guns may be slewed towards target:
	if (design->primary) {
		shot = CreateShot(shotpos, aim_cam, design, ship);

		if (shot) {
			shot->SetVelocity(shot->Velocity() + base_vel);
		}
	}

	// missiles always launch in rail direction:
	else {
		// unless they are on a mobile launcher
		if (turret && design->self_aiming) {
			shot = CreateShot(shotpos, aim_cam, design, ship);
			shot->SetVelocity(base_vel);
		}
		else {
			shot = CreateShot(shotpos, rail_cam, design, ship);
			if (shot /* && !turret */) {
				Matrix orient = ship->Cam().Orientation();
				if (aim_azimuth != 0)   orient.Yaw(aim_azimuth);
				if (aim_elevation != 0) orient.Pitch(aim_elevation);

				Point eject = design->eject * orient;
				shot->SetVelocity(base_vel + eject);
			}
		}

		if (shot && visible_stores[n]) {
			GRAPHIC_DESTROY(visible_stores[n]);
		}
	}

	if (shot) {
		if (ammo > 0)
		ammo--;

		if (guided && target)
		shot->SeekTarget(target, subtarget);

		float shot_load;
		if (energy > design->charge)
		shot_load = design->charge;
		else
		shot_load = energy;   

		energy -= shot_load;
		shot->SetCharge(shot_load * availability);

		if (target && design->flak && !design->guided) {
			double speed = shot->Velocity().length();
			double range = (target->Location() - shot->Location()).length();

			if (range > design->min_range && range < design->max_range) {
				shot->SetFuse(range / speed);
			}
		}

		region->InsertObject(shot);

		if (beams) {
			beams[n] = shot;
			Observe(beams[n]);

			// aim beam at target:
			SetBeamPoints(true);
		}

		if (ship) {
			ShipStats* stats = ShipStats::Find(ship->Name());

			if (design->primary)
			stats->AddGunShot();
			else if (design->decoy_type == 0 && design->damage > 0)
			stats->AddMissileShot();
		}
	}

	return shot;
}

Shot*
Weapon::CreateShot(const Point& loc, const Camera& cam, WeaponDesign* dsn, const Ship* own)
{
	if (dsn->drone)
	return new(__FILE__,__LINE__) Drone(loc, cam, dsn, own);

	else
	return new(__FILE__,__LINE__) Shot(loc, cam, dsn, own);
}

// +--------------------------------------------------------------------+

void
Weapon::Orient(const Physical* rep)
{
	System::Orient(rep);

	const Matrix& orientation = rep->Cam().Orientation();
	Point         loc         = rep->Location();

	// align graphics with camera:
	if (turret) {
		if (!design->self_aiming)
		ZeroAim();

		const Matrix& aim = aim_cam.Orientation();

		turret->MoveTo(mount_loc);
		turret->SetOrientation(aim);

		if (turret_base) {
			Matrix base = orientation;

			if (design->turret_axis == 1) {
				base.Pitch(aim_elevation);
				base.Pitch(old_elevation);
			}
			else {
				base.Yaw(aim_azimuth);
				base.Yaw(old_azimuth);
			}

			turret_base->MoveTo(mount_loc);
			turret_base->SetOrientation(base);
		}

		for (int i = 0; i < nbarrels; i++) {
			muzzle_pts[i] = (rel_pts[i] * aim) + mount_loc;

			if (visible_stores[i]) {
				visible_stores[i]->SetOrientation(aim);
				visible_stores[i]->MoveTo(muzzle_pts[i]);
			}
		}
	}
	else {
		for (int i = 0; i < nbarrels; i++) {
			muzzle_pts[i] = (rel_pts[i] * orientation) + loc;

			if (visible_stores[i]) {
				visible_stores[i]->SetOrientation(orientation);
				visible_stores[i]->MoveTo(muzzle_pts[i]);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Weapon::SetBeamPoints(bool aim)
{
	for (int i = 0; i < nbarrels; i++) {
		if (beams && beams[i]) {
			Point  from = muzzle_pts[i];
			Point  to;
			double len  = design->length;

			if (len < 1 || len > 1e7)
			len = design->max_range;

			if (len < 1)
			len = 3e5;

			else if (len > 1e7)
			len = 1e7;

			// always use the aim cam, to enforce slew_rate
			to = from + aim_cam.vpn() * len;

			beams[i]->SetBeamPoints(from, to);
		}
	}
}

// +--------------------------------------------------------------------+

void
Weapon::Aim()
{
	locked   = false;
	centered = false;

	FindObjective();

	if (target) {
		double az = 0;
		double el = 0;

		locked = CanLockPoint(obj_w, az, el, &objective);

		double spread_az = design->spread_az;
		double spread_el = design->spread_el;

		// beam sweep target:
		if (design->beam) {
			double factor   = 0;
			double az_phase = 0;
			double el_phase = 0;

			if (target->Type() == SimObject::SIM_SHIP) {
				Ship* s = (Ship*) target;

				if (s->IsStarship()) {
					switch (sweep) {
					default:
					case SWEEP_NONE:  factor = 0; break;
					case SWEEP_TIGHT: factor = 1; break;
					case SWEEP_WIDE:  factor = 2; break;
					}
				}
			}

			if (factor > 0) {
				factor  *= atan2(target->Radius(), (double) objective.z);

				for (int i = 0; i < nbarrels; i++) {
					if (beams && beams[i]) {
						az_phase = sin(beams[i]->Life() * 0.4 * PI);
						el_phase = sin(beams[i]->Life() * 1.0 * PI);
						break;
					}
				}

				az += factor * spread_az * az_phase;
				el += factor * spread_el * el_phase * 0.25;
			}
		}

		else if (!design->beam) {
			if (spread_az > 0)
			az += Random(-spread_az, spread_az);

			if (spread_el > 0)
			el += Random(-spread_el, spread_el);
		}

		AimTurret(az, -el);

		// check range for guided weapons:
		if (locked && guided) {
			double range = objective.length();

			if (range > design->max_track)
			locked = false;

			else if (range > design->max_range) {
				if (firing) {
					if (RandomChance(1,4))   // 1 in 4 chance of locking anyway
					locked = false;
				}
				else {
					locked = false;
				}
			}
		}

		if (locked) {
			Point tloc = target->Location();
			tloc = Transform(tloc);

			if (tloc.z > 1) {
				az = atan2(fabs(tloc.x), tloc.z);
				el = atan2(fabs(tloc.y), tloc.z);

				double firing_cone = 10*DEGREES;

				if (orders == MANUAL)
				firing_cone = 30*DEGREES;

				if (az < firing_cone && el < firing_cone)
				centered = true;
			}
		}
	}
	else {
		AimTurret(aim_az_rest, -aim_el_rest);
	}
}

void
Weapon::FindObjective()
{
	ZeroAim();

	if (!target || !design->self_aiming) {
		objective = Point();
		return;
	}

	obj_w = target->Location();

	if (subtarget) {
		obj_w = subtarget->MountLocation();
	}
	else if (target->Type() == SimObject::SIM_SHIP) {
		Ship* tgt_ship = (Ship*) target;

		if (tgt_ship->IsGroundUnit())
		obj_w += Point(0,150,0);
	}

	if (!design->beam && shot_speed > 0) {
		// distance from self to target:
		double distance = Point(obj_w - muzzle_pts[0]).length();

		// TRUE shot speed is relative to ship speed:
		Point  eff_shot_vel   = ship->Velocity() + aim_cam.vpn() * shot_speed - target->Velocity();
		double eff_shot_speed = eff_shot_vel.length();

		// time to reach target:
		double time = distance / eff_shot_speed;

		// where the target will be when the shot reaches it:
		obj_w += (target->Velocity() - ship->Velocity()) * time +
		target->Acceleration() * 0.25 * time * time;
	}

	// transform into camera coords:
	objective = Transform(obj_w);
}

Point
Weapon::Transform(const Point& pt)
{
	Point result;

	Point obj_t = pt - aim_cam.Pos();
	result.x = obj_t * aim_cam.vrt();
	result.y = obj_t * aim_cam.vup();
	result.z = obj_t * aim_cam.vpn();

	return result;
}

bool
Weapon::CanLockPoint(const Point& test, double& az, double& el, Point* obj)
{
	Point pt     = Transform(test);
	bool  locked = true;

	// first compute az:
	if (fabs(pt.z) < 0.1) pt.z = 0.1;
	az = atan(pt.x / pt.z);
	if (pt.z < 0) az -= PI;
	if (az < -PI) az += 2*PI;

	// then, rotate target into az-coords to compute el:
	Camera tmp;
	tmp.Clone(aim_cam);
	aim_cam.Yaw(az);
	pt = Transform(test);
	aim_cam.Clone(tmp);

	if (fabs(pt.z) < 0.1) pt.z = 0.1;
	el = atan(pt.y / pt.z);

	if (obj) *obj = pt;

	// is the target in the basket?
	// clamp if necessary:

	if (az > aim_az_max) {
		az     = aim_az_max;
		locked = false;
	}
	else if (az < aim_az_min) {
		az     = aim_az_min;
		locked = false;
	}

	if (el > aim_el_max) {
		el     = aim_el_max;
		locked = false;
	}
	else if (el < aim_el_min) {
		el     = aim_el_min;
		locked = false;
	}

	if (IsDrone() && guided) {
		double firing_cone = 10*DEGREES;

		if (orders == MANUAL)
		firing_cone = 20*DEGREES;

		if (az < firing_cone && el < firing_cone)
		locked = true;
	}

	return locked;
}

// +--------------------------------------------------------------------+

void
Weapon::AimTurret(double az, double el)
{
	double seconds = (Game::GameTime() - aim_time) / 1000.0;

	// don't let the weapon turn faster than turret slew rate:
	double max_turn = design->slew_rate * seconds;

	if (fabs(az-old_azimuth) > max_turn) {
		if (az > old_azimuth)
		az = old_azimuth + max_turn;
		else
		az = old_azimuth - max_turn;
	}

	if (fabs(el-old_elevation) > PI/2 * seconds) {
		if (el > old_elevation)
		el = old_elevation + max_turn;
		else
		el = old_elevation - max_turn;
	}

	aim_cam.Yaw(az);
	aim_cam.Pitch(el);

	old_azimuth   = (float) az;
	old_elevation = (float) el;

	aim_time = Game::GameTime();
}

void
Weapon::ZeroAim()
{
	aim_cam.Clone(ship->Cam());
	if (aim_azimuth   != 0) aim_cam.Yaw(aim_azimuth);
	if (aim_elevation != 0) aim_cam.Pitch(aim_elevation);

	aim_cam.MoveTo(muzzle_pts[0]);
}
