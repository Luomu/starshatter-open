/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Thruster.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Weapon class
*/

#include "MemDebug.h"
#include "Thruster.h"
#include "Component.h"
#include "Drive.h"
#include "FlightComp.h"
#include "SystemDesign.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Sim.h"
#include "CameraDirector.h"
#include "AudioConfig.h"
#include "Random.h"

#include "Light.h"
#include "Bitmap.h"
#include "Sound.h"
#include "DataLoader.h"
#include "Bolt.h"
#include "Sprite.h"
#include "Game.h"

// +----------------------------------------------------------------------+

static Sound*  thruster_resource    = 0;
static Sound*  thruster_sound       = 0;

extern Bitmap* drive_flare_bitmap[8];
extern Bitmap* drive_trail_bitmap[8];

#define CLAMP(x, a, b) if (x < (a)) x = (a); else if (x > (b)) x = (b);

// +----------------------------------------------------------------------+

ThrusterPort::ThrusterPort(int t, const Point& l, DWORD f, float s)
: type(t), loc(l), flare(0), trail(0), fire(f), burn(0), scale(s)
{ }

ThrusterPort::~ThrusterPort()
{
	GRAPHIC_DESTROY(flare);
	GRAPHIC_DESTROY(trail);
}

// +----------------------------------------------------------------------+

static int     sys_value = 2;

// +----------------------------------------------------------------------+

Thruster::Thruster(int dtype, double max_thrust, float flare_scale)
: System(DRIVE, dtype, "Thruster", sys_value,
max_thrust, max_thrust, max_thrust),
ship(0), thrust(1.0f), scale(flare_scale), 
avail_x(1.0f), avail_y(1.0f), avail_z(1.0f)
{
	name = Game::GetText("sys.thruster");
	abrv = Game::GetText("sys.thruster.abrv");

	power_flags = POWER_WATTS;

	ZeroMemory(burn, sizeof(burn));

	emcon_power[0] = 50;
	emcon_power[1] = 50;
	emcon_power[2] = 100;
}

// +----------------------------------------------------------------------+

Thruster::Thruster(const Thruster& t)
: System(t), ship(0),
thrust(1.0f), scale(t.scale),
avail_x(1.0f), avail_y(1.0f), avail_z(1.0f)
{
	power_flags = POWER_WATTS;
	Mount(t);

	ZeroMemory(burn, sizeof(burn));

	if (subtype != Drive::STEALTH) {
		for (int i = 0; i < t.ports.size(); i++) {
			ThrusterPort* p = t.ports[i];
			CreatePort(p->type, p->loc, p->fire, p->scale);
		}
	}
}

// +--------------------------------------------------------------------+

Thruster::~Thruster()
{
	ports.destroy();

	if (thruster_sound && thruster_sound->IsPlaying()) {
		thruster_sound->Stop();
	}
}

// +--------------------------------------------------------------------+

void
Thruster::Initialize()
{
	static int initialized = 0;
	if (initialized) return;

	DataLoader* loader = DataLoader::GetLoader();

	const int SOUND_FLAGS = Sound::LOCALIZED |
	Sound::LOC_3D    |
	Sound::LOOP      |
	Sound::LOCKED;

	loader->SetDataPath("Sounds/");
	loader->LoadSound("thruster.wav", thruster_resource, SOUND_FLAGS);
	loader->SetDataPath(0);

	if (thruster_resource)
	thruster_resource->SetMaxDistance(15.0e3f);

	initialized = 1;
}

void
Thruster::Close()
{
	delete thruster_resource;
	thruster_resource = 0;

	if (thruster_sound) {
		thruster_sound->Stop();
		thruster_sound->Release();
	}
}

// +--------------------------------------------------------------------+

void
Thruster::Orient(const Physical* rep)
{
	System::Orient(rep);

	bool hide_all = false;
	if (!ship || (ship->IsAirborne() && ship->Class() != Ship::LCA)) {
		hide_all = true;
	}

	if (ship->Rep() && ship->Rep()->Hidden()) {
		hide_all = true;
	}

	if (thrust <= 0) {
		hide_all = true;
	}

	const Matrix& orientation = rep->Cam().Orientation();
	Point         ship_loc    = rep->Location();

	for (int i = 0; i < ports.size(); i++) {
		ThrusterPort* p = ports[i];

		Point projector = (p->loc * orientation) + ship_loc;
		
		if (p->flare)
		p->flare->MoveTo(projector);

		if (p->trail) {
			double intensity = p->burn;

			if (intensity > 0.5 && !hide_all) {
				Bolt* t   = (Bolt*) p->trail;
				double  len = -50 * p->scale * intensity;

				t->Show();

				switch (p->type) {
				case LEFT:     t->SetEndPoints(projector, projector + rep->Cam().vrt() * len); break;
				case RIGHT:    t->SetEndPoints(projector, projector - rep->Cam().vrt() * len); break;
				case AFT:      t->SetEndPoints(projector, projector + rep->Cam().vpn() * len); break;
				case FORE:     t->SetEndPoints(projector, projector - rep->Cam().vpn() * len); break;
				case BOTTOM:   t->SetEndPoints(projector, projector + rep->Cam().vup() * len); break;
				case TOP:      t->SetEndPoints(projector, projector - rep->Cam().vup() * len); break;
				default:       t->Hide(); break;
				}
			}
			else {
				p->trail->Hide();
				if (p->flare)
				p->flare->Hide();
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Thruster::ExecFrame(double seconds)
{
	System::ExecFrame(seconds);

	if (ship) {
		double rr = 0, pr = 0, yr = 0;
		double rd = 0, pd = 0, yd = 0;

		double agility   = 1;
		double stability = 1;

		FlightComp* flcs = ship->GetFLCS();

		if (flcs) {
			if (!flcs->IsPowerOn() || flcs->Status() < DEGRADED) {
				agility   = 0.3;
				stability = 0.0;
			}
		}

		// check for thruster damage here:
		if (components.size() >= 3) {
			int stat = components[0]->Status();
			if (stat == Component::NOMINAL)
			avail_x = 1.0f;
			else if (stat == Component::DEGRADED)
			avail_x = 0.5f;
			else
			avail_x = 0.0f;

			stat = components[1]->Status();
			if (stat == Component::NOMINAL)
			avail_z = 1.0f;
			else if (stat == Component::DEGRADED)
			avail_z = 0.5f;
			else
			avail_z = 0.0f;

			stat = components[2]->Status();
			if (stat == Component::NOMINAL)
			avail_y = 1.0f;
			else if (stat == Component::DEGRADED)
			avail_y = 0.5f;
			else
			avail_y = 0.0f;
		}

		// thrust limited by power distribution:
		thrust = energy/capacity;
		energy = 0.0f;

		if (thrust < 0)
		thrust = 0.0f;

		agility   *= thrust;
		stability *= thrust;

		rr = roll_rate  * agility   * avail_y;
		pr = pitch_rate * agility   * avail_y;
		yr = yaw_rate   * agility   * avail_x;

		rd = roll_drag  * stability * avail_y;
		pd = pitch_drag * stability * avail_y;
		yd = yaw_drag   * stability * avail_x;

		ship->SetAngularRates(rr,pr,yr);
		ship->SetAngularDrag (rd,pd,yd);
	}
}

// +--------------------------------------------------------------------+

void
Thruster::SetShip(Ship* s)
{
	const double ROLL_SPEED  = PI * 0.0400;
	const double PITCH_SPEED = PI * 0.0250;
	const double YAW_SPEED   = PI * 0.0250;

	ship = s;

	if (ship) {
		ShipDesign* design = (ShipDesign*) ship->Design();

		trans_x     = design->trans_x;
		trans_y     = design->trans_y;
		trans_z     = design->trans_z;

		roll_drag   = design->roll_drag;
		pitch_drag  = design->pitch_drag;
		yaw_drag    = design->yaw_drag;
		
		roll_rate   = (float) (design->roll_rate  * PI / 180);
		pitch_rate  = (float) (design->pitch_rate * PI / 180);
		yaw_rate    = (float) (design->yaw_rate   * PI / 180);
		
		double agility = design->agility;

		if (roll_rate == 0)  roll_rate  = (float) (agility * ROLL_SPEED);
		if (pitch_rate == 0) pitch_rate = (float) (agility * PITCH_SPEED);
		if (yaw_rate == 0)   yaw_rate   = (float) (agility * YAW_SPEED);
	}
}

// +--------------------------------------------------------------------+

double
Thruster::TransXLimit()
{
	return trans_x * avail_x;
}

double
Thruster::TransYLimit()
{
	return trans_y * avail_y;
}

double
Thruster::TransZLimit()
{
	return trans_z * avail_z;
}

// +--------------------------------------------------------------------+

void
Thruster::ExecTrans(double x, double y, double z)
{
	if (!ship || (ship->IsAirborne() && ship->Class() != Ship::LCA)) {
		if (thruster_sound && thruster_sound->IsPlaying())
		thruster_sound->Stop();

		for (int i = 0; i < ports.size(); i++) {
			ThrusterPort* p = ports[i];
			if (p->flare) p->flare->Hide();
			if (p->trail) p->trail->Hide();
		}

		return;
	}

	bool   sound_on   = false;
	bool   show_flare = true;

	if (ship->Rep() && ship->Rep()->Hidden())
	show_flare = false;

	if (ship->Class() == Ship::LCA && 
			ship->IsAirborne() &&
			ship->Velocity().length() < 250 &&
			ship->AltitudeAGL() > ship->Radius()/2) {

		sound_on = true;
		IncBurn(BOTTOM, TOP);
	}

	else if (!ship->IsAirborne()) {
		double tx_limit = ship->Design()->trans_x;
		double ty_limit = ship->Design()->trans_y;
		double tz_limit = ship->Design()->trans_z;

		if (x < -0.15 * tx_limit)     IncBurn(RIGHT,  LEFT);
		else if (x > 0.15 * tx_limit) IncBurn(LEFT,   RIGHT);
		else                          DecBurn(LEFT,   RIGHT);

		if (y < -0.15 * ty_limit)     IncBurn(FORE,   AFT);
		else if (y > 0.15 * ty_limit) IncBurn(AFT,    FORE);
		else                          DecBurn(FORE,   AFT);

		if (z < -0.15 * tz_limit)     IncBurn(TOP,    BOTTOM);
		else if (z > 0.15 * tz_limit) IncBurn(BOTTOM, TOP);
		else                          DecBurn(TOP,    BOTTOM);

		double r, p, y;
		ship->GetAngularThrust(r, p, y);

		// Roll seems to have the opposite sign from
		// the pitch and yaw thrust factors.  Not sure why.

		if (r > 0)        IncBurn(ROLL_L,  ROLL_R);
		else if (r < 0)   IncBurn(ROLL_R,  ROLL_L);
		else              DecBurn(ROLL_R,  ROLL_L);

		if (y < 0)        IncBurn(YAW_L,   YAW_R);
		else if (y > 0)   IncBurn(YAW_R,   YAW_L);
		else              DecBurn(YAW_R,   YAW_L);

		if (p < 0)        IncBurn(PITCH_D, PITCH_U);
		else if (p > 0)   IncBurn(PITCH_U, PITCH_D);
		else              DecBurn(PITCH_U, PITCH_D);
	}

	else {
		for (int i = 0; i < 12; i++) {
			burn[i] -= 0.1f;
			if (burn[i] < 0)
			burn[i] = 0.0f;
		}
	}

	for (int i = 0; i < ports.size(); i++) {
		ThrusterPort* p = ports[i];

		if (p->fire) {
			p->burn = 0;

			int flag = 1;

			for (int n = 0; n < 12; n++) {
				if ((p->fire & flag) != 0 && burn[n] > p->burn)
				p->burn = burn[n];

				flag <<= 1;
			}
		}

		else {
			p->burn = burn[p->type];
		}

		if (p->burn > 0 && thrust > 0) {
			sound_on = true;

			if (show_flare) {
				Sprite* flare_rep = (Sprite*) p->flare;
				if (flare_rep) {
					flare_rep->Show();
					flare_rep->SetShade(1);
				}

				if (p->trail) {
					Bolt* t = (Bolt*) p->trail;
					t->Show();
					t->SetShade(1);
				}
			}
		}
		else {
			if (p->flare)  p->flare->Hide();
			if (p->trail)  p->trail->Hide();
		}
	}

	// thruster sound:
	if (ship && ship == Sim::GetSim()->GetPlayerShip() && ports.size() > 0) {
		CameraDirector* cam_dir = CameraDirector::GetInstance();

		// no sound when paused!
		if (!Game::Paused() && cam_dir && cam_dir->GetCamera()) {
			if (!thruster_sound) {
				if (thruster_resource)
				thruster_sound = thruster_resource->Duplicate();
			}

			if (thruster_sound) {
				if (sound_on) {
					Point cam_loc = cam_dir->GetCamera()->Pos();
					double dist = (ship->Location() - cam_loc).length();

					long max_vol = AudioConfig::EfxVolume();
					long volume  = -2000;

					if (volume > max_vol)
					volume = max_vol;

					if (dist < thruster_sound->GetMaxDistance()) {
						thruster_sound->SetLocation(ship->Location());
						thruster_sound->SetVolume(volume);
						thruster_sound->Play();
					}
					else if (thruster_sound->IsPlaying()) {
						thruster_sound->Stop();
					}
				}
				else if (thruster_sound->IsPlaying()) {
					thruster_sound->Stop();
				}
			}
		}
	}

	ship->SetTransX(x * thrust);
	ship->SetTransY(y * thrust);
	ship->SetTransZ(z * thrust);
}

// +--------------------------------------------------------------------+

void
Thruster::AddPort(int type, const Point& loc, DWORD fire, float flare_scale)
{
	if (flare_scale == 0) flare_scale = scale;
	ThrusterPort* port = new(__FILE__,__LINE__) ThrusterPort(type, loc, fire, flare_scale);
	ports.append(port);
}

void
Thruster::CreatePort(int type, const Point& loc, DWORD fire, float flare_scale)
{
	Bitmap* flare_bmp = drive_flare_bitmap[subtype];
	Bitmap* trail_bmp = drive_trail_bitmap[subtype];

	if (subtype != Drive::STEALTH) {
		Sprite* flare_rep = new(__FILE__,__LINE__) Sprite(flare_bmp);
		flare_rep->Scale(flare_scale * 0.667f);
		flare_rep->SetShade(0);

		Bolt* trail_rep = new(__FILE__,__LINE__) Bolt(flare_scale * 30, flare_scale * 8, trail_bmp, true);

		ThrusterPort* port = new(__FILE__,__LINE__) ThrusterPort(type, loc, fire, flare_scale);
		port->flare = flare_rep;
		port->trail = trail_rep;
		ports.append(port);
	}
}

// +--------------------------------------------------------------------+

int
Thruster::NumThrusters() const
{
	return ports.size();
}

Graphic*
Thruster::Flare(int engine) const
{
	if (engine >= 0 && engine < ports.size())
	return ports[engine]->flare;

	return 0;
}

Graphic*
Thruster::Trail(int engine) const
{
	if (engine >= 0 && engine < ports.size())
	return ports[engine]->trail;

	return 0;
}

// +--------------------------------------------------------------------+

void
Thruster::IncBurn(int inc, int dec)
{
	burn[inc] += 0.1f;
	if (burn[inc] > 1)
	burn[inc] = 1.0f;

	burn[dec] -= 0.1f;
	if (burn[dec] < 0)
	burn[dec] = 0.0f;
}

void
Thruster::DecBurn(int a, int b)
{
	burn[a] -= 0.1f;
	if (burn[a] < 0)
	burn[a] = 0.0f;

	burn[b] -= 0.1f;
	if (burn[b] < 0)
	burn[b] = 0.0f;
}

// +----------------------------------------------------------------------+

double
Thruster::GetRequest(double seconds) const
{
	if (!power_on)
	return 0;

	for (int i = 0; i < 12; i++)
	if (burn[i] != 0)
	return power_level * sink_rate * seconds;

	return 0;
}

