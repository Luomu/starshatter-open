/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Contact.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Integrated (Passive and Active) Sensor Package class implementation
*/

#include "MemDebug.h"
#include "Contact.h"
#include "Drone.h"
#include "Sensor.h"
#include "Ship.h"
#include "Sim.h"
#include "WeaponDesign.h"

#include "Game.h"

// +----------------------------------------------------------------------+

const int      DEFAULT_TRACK_UPDATE = 500; // milliseconds
const int      DEFAULT_TRACK_LENGTH =  20; // 10 seconds
const double   DEFAULT_TRACK_AGE    =  10; // 10 seconds
const double   SENSOR_THRESHOLD     = 0.25;

// +----------------------------------------------------------------------+

Contact::Contact()
   : ship(0), shot(0), d_pas(0.0f), d_act(0.0f),
     track(0), ntrack(0), time(0), track_time(0), probe(false)
{
   acquire_time = Game::GameTime();
}

Contact::Contact(Ship* s, float p, float a)
   : ship(s), shot(0), d_pas(p), d_act(a),
     track(0), ntrack(0), time(0), track_time(0), probe(false)
{
   acquire_time = Game::GameTime();
   Observe(ship);
}

Contact::Contact(Shot* s, float p, float a)
   : ship(0), shot(s), d_pas(p), d_act(a),
     track(0), ntrack(0), time(0), track_time(0), probe(false)
{
   acquire_time = Game::GameTime();
   Observe(shot);
}

// +----------------------------------------------------------------------+

Contact::~Contact()
{
   delete [] track;
}

// +----------------------------------------------------------------------+

int 
Contact::operator == (const Contact& c) const
{
   if (ship)
      return ship == c.ship;

   else if (shot)
      return shot == c.shot;

   return 0;
}

// +----------------------------------------------------------------------+

bool
Contact::Update(SimObject* obj)
{
   if (obj == ship || obj == shot) {
      ship  = 0;
      shot  = 0;
      d_act = 0;
      d_pas = 0;

      ClearTrack();
   }

   return SimObserver::Update(obj);   
}

const char*
Contact::GetObserverName() const
{
   static char name[128];

   if (ship)
      sprintf(name, "Contact Ship='%s'", ship->Name());
   else if (shot)
      sprintf(name, "Contact Shot='%s' %s", shot->Name(), shot->DesignName());
   else
      sprintf(name, "Contact (unknown)");

   return name;
}

// +----------------------------------------------------------------------+

double
Contact::Age() const
{
   double age = 0;

   if (!ship && !shot)
      return age;

   double seconds = (Game::GameTime() - time) / 1000.0;

   age = 1.0 - seconds/DEFAULT_TRACK_AGE;

   if (age < 0)
      age = 0;

   return age;
}

int
Contact::GetIFF(const Ship* observer) const
{
   int i = 0;

   if (ship) {
      i = ship->GetIFF();

      // if the contact is on our side or has locked us up,
      // we know whose side he's on.

      // Otherwise:
      if (i != observer->GetIFF() && !Threat(observer)) {
         if (d_pas < 2*SENSOR_THRESHOLD && d_act < SENSOR_THRESHOLD && !Visible(observer))
            i = -1000;   // indeterminate iff reading
      }
   }

   else if (shot && shot->Owner()) {
      i = shot->Owner()->GetIFF();
   }

   return i;
}

bool
Contact::ActLock() const
{
   return d_act >= SENSOR_THRESHOLD;
}

bool
Contact::PasLock() const
{
   return d_pas >= SENSOR_THRESHOLD;
}

// +----------------------------------------------------------------------+

void
Contact::GetBearing(const Ship* observer, double& az, double& el, double& rng) const
{
   // translate:
   Point targ_pt = loc - observer->Location();

   // rotate:
   const Camera* cam = &observer->Cam();
   double        tx  = targ_pt * cam->vrt();
   double        ty  = targ_pt * cam->vup();
   double        tz  = targ_pt * cam->vpn();

   // convert to spherical coords:
   rng = targ_pt.length();
   az  = asin(fabs(tx) / rng);
   el  = asin(fabs(ty) / rng);

   if (tx < 0) az = -az;
   if (ty < 0) el = -el;

   // correct az/el for back hemisphere:
   if (tz < 0) {
      if (az < 0) az = -PI - az;
      else        az =  PI - az;
   }
}

double
Contact::Range(const Ship* observer, double limit) const
{
   double r = Point(loc - observer->Location()).length();

   // if passive only, return approximate range:
   if (!ActLock()) {
      const int chunk = 25000;

      if (!PasLock()) {
         r = (int) limit;
      }

      else if (r <= chunk) {
         r = chunk;
      }

      else {
         int r1 = (int) (r + chunk/2) / chunk;
         r = r1 * chunk;
      }
   }

   return r;
}

// +----------------------------------------------------------------------+

bool
Contact::InFront(const Ship* observer) const
{
   // translate:
   Point targ_pt = loc - observer->Location();

   // rotate:
   const Camera* cam = &observer->Cam();
   double tz = targ_pt * cam->vpn();

   if (tz > 1.0)
      return true;

   return false;
}

bool
Contact::Threat(const Ship* observer) const
{
   bool threat = false;

   if (observer && observer->Life() != 0) {
      if (ship && ship->Life() != 0) {
         threat = (ship->GetIFF() &&
                   ship->GetIFF() != observer->GetIFF() &&
                   ship->GetEMCON() > 2 &&
                   ship->IsTracking((Ship*) observer) &&
                   ship->Weapons().size() > 0);

         if (threat && observer->GetIFF() == 0)
            threat = ship->GetIFF() > 1;
      }

      else if (shot) {
         threat = shot->IsTracking((Ship*) observer);

         if (!threat && shot->Design()->probe && shot->GetIFF() != observer->GetIFF()) {
            Point  probe_pt = shot->Location() - observer->Location();
            double prng     = probe_pt.length();

            threat = (prng < shot->Design()->lethal_radius);
         }
      }
   }

   return threat;
}

bool
Contact::Visible(const Ship* observer) const
{
   // translate:
   Point targ_pt = loc - observer->Location();
   double radius = 0;
   
   if (ship)
      radius = ship->Radius();

   else if (shot)
      radius = shot->Radius();

   // rotate:
   const Camera* cam = &observer->Cam();
   double        rng = targ_pt.length();

   return radius/rng > 0.002;
}

// +----------------------------------------------------------------------+

void
Contact::Reset()
{
   if (Game::Paused()) return;

   float step_down = (float) (Game::FrameTime() / 10);

   if (d_pas > 0)
      d_pas -= step_down;

   if (d_act > 0)
      d_act -= step_down;
}

void
Contact::Merge(Contact* c)
{
   if (c->GetShip() == ship && c->GetShot() == shot) {
      if (c->d_pas > d_pas)
         d_pas = c->d_pas;

      if (c->d_act > d_act)
         d_act = c->d_act;
   }
}

void
Contact::ClearTrack()
{
   delete [] track;
   track = 0;
   ntrack = 0;
}

void
Contact::UpdateTrack()
{
   time = Game::GameTime();

   if (shot || (ship && ship->IsGroundUnit()))
      return;

   if (!track) {
      track = new(__FILE__,__LINE__) Point[DEFAULT_TRACK_LENGTH];
      track[0] = loc;
      ntrack   = 1;
      track_time = time;
   }

   else if (time - track_time > DEFAULT_TRACK_UPDATE) {
      if (loc != track[0]) {
         for (int i = DEFAULT_TRACK_LENGTH-2; i >= 0; i--)
            track[i+1] = track[i];

         track[0] = loc;
         if (ntrack < DEFAULT_TRACK_LENGTH) ntrack++;
      }

      track_time = time;
   }
}

// +----------------------------------------------------------------------+

Point
Contact::TrackPoint(int i) const
{
   if (track && i < ntrack)
      return track[i];

   return Point();
}
