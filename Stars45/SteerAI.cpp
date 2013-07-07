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
    FILE:         SteerAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Steering (low-level) Artificial Intelligence class
*/

#include "MemDebug.h"
#include "SteerAI.h"
#include "SeekerAI.h"
#include "FighterAI.h"
#include "StarshipAI.h"
#include "GroundAI.h"
#include "System.h"

#include "Game.h"
#include "Physical.h"

// +----------------------------------------------------------------------+

Steer  Steer::operator+(const Steer& s) const
{
    return Steer(yaw+s.yaw, pitch+s.pitch, roll+s.roll, (brake>s.brake)?brake:s.brake);
}

Steer  Steer::operator-(const Steer& s) const
{
    return Steer(yaw-s.yaw, pitch-s.pitch, roll-s.roll, (brake<s.brake)?brake:s.brake);
}

Steer  Steer::operator*(double f)       const
{
    return Steer(yaw*f, pitch*f, roll*f, brake);
}

Steer  Steer::operator/(double f)       const
{
    return Steer(yaw/f, pitch/f, roll/f, brake);
}


Steer& Steer::operator+=(const Steer& s)
{
    yaw   += s.yaw;
    pitch += s.pitch;
    roll  += s.roll;

    if (s.brake > brake)
    brake = s.brake;

    if (s.stop)
    stop = 1;
    
    return *this;   
}

Steer& Steer::operator-=(const Steer& s)
{
    yaw   -= s.yaw;
    pitch -= s.pitch;
    roll  -= s.roll;

    if (s.brake < brake)
    brake = s.brake;

    if (s.stop)
    stop = 1;
    
    return *this;   
}


double
Steer::Magnitude() const
{
    return sqrt(yaw*yaw + pitch*pitch);
}

// +--------------------------------------------------------------------+

Director*
SteerAI::Create(SimObject* self, int type)
{
    switch (type) {
    case SEEKER:   return new(__FILE__,__LINE__) SeekerAI(self);
        break;

    case STARSHIP: return new(__FILE__,__LINE__) StarshipAI(self);
        break;

    case GROUND:   return new(__FILE__,__LINE__) GroundAI(self);
        break;

    default:
    case FIGHTER:  return new(__FILE__,__LINE__) FighterAI(self);
        break;
    }
}

// +----------------------------------------------------------------------+

SteerAI::SteerAI(SimObject* ship)
: self(ship),
target(0), subtarget(0), other(0), distance(0.0), evade_time(0),
objective(0.0f, 0.0f, 0.0f)
{
    seek_gain = 20;
    seek_damp = 0.5;

    for (int i = 0; i < 3; i++)
    az[i] = el[i] = 0;
}


// +--------------------------------------------------------------------+

SteerAI::~SteerAI()
{ }

// +--------------------------------------------------------------------+

void
SteerAI::SetTarget(SimObject* targ, System* sub)
{
    if (target != targ) {
        target = targ;
        
        if (target)
        Observe(target);
    }

    subtarget = sub;
}

void
SteerAI::DropTarget(double dtime)
{
    SetTarget(0);
}

// +--------------------------------------------------------------------+

bool
SteerAI::Update(SimObject* obj)
{
    if (obj == target) {
        target = 0;
        subtarget = 0;
    }

    if (obj == other) {
        other = 0;
    }

    return SimObserver::Update(obj);
}

const char*
SteerAI::GetObserverName() const
{
    static char name[64];
    sprintf_s(name, "SteerAI(%s)", self->Name());
    return name;
}

// +--------------------------------------------------------------------+

Point
SteerAI::ClosingVelocity()
{
    if (self) {
        if (target)
        return self->Velocity() - target->Velocity();
        else
        return self->Velocity();
    }

    return Point(1, 0, 0);
}

void
SteerAI::FindObjective()
{
    if (!self || !target) return;

    Point  cv   = ClosingVelocity();
    double cvl  = cv.length();
    double time = 0;

    if (cvl > 5) {
        // distance from self to target:
        distance = Point(target->Location() - self->Location()).length();

        // time to reach target:
        time     = distance / cvl;

        // where the target will be when we reach it:
        Point run_vec = target->Velocity();
        obj_w   = target->Location() + (run_vec * time);
    }

    else {
        obj_w   = target->Location();
    }

    // subsystem offset:
    if (subtarget) {
        Point offset = target->Location() - subtarget->MountLocation();
        obj_w -= offset;
    }

    distance = Point(obj_w - self->Location()).length();

    if (cvl > 5)
    time     = distance / cvl;

    // where we will be when the target gets there:
    Point self_dest = self->Location() + cv * time;
    Point err = obj_w - self_dest;

    obj_w += err;

    // transform into camera coords:
    objective = Transform(obj_w);
    objective.Normalize();

    distance = Point(obj_w - self->Location()).length();
}

Point
SteerAI::Transform(const Point& pt)
{
    Point obj_t = pt - self->Location();
    Point result;

    if (self->FlightPathYawAngle() != 0 || self->FlightPathPitchAngle() != 0) {
        double az = self->FlightPathYawAngle();
        double el = self->FlightPathPitchAngle();

        const double MAX_ANGLE = 15*DEGREES;
        const double MIN_ANGLE =  3*DEGREES;

        if (az > MAX_ANGLE)
        az = MAX_ANGLE;
        else if (az < -MAX_ANGLE)
        az = -MAX_ANGLE;
        else if (az > MIN_ANGLE)
        az = MIN_ANGLE + (az-MIN_ANGLE)/2;
        else if (az < -MIN_ANGLE)
        az = -MIN_ANGLE + (az+MIN_ANGLE)/2;

        if (el > MAX_ANGLE)
        el = MAX_ANGLE;
        else if (el < -MAX_ANGLE)
        el = -MAX_ANGLE;
        else if (el > MIN_ANGLE)
        el = MIN_ANGLE + (el-MIN_ANGLE)/2;
        else if (el < -MIN_ANGLE)
        el = -MIN_ANGLE + (el+MIN_ANGLE)/2;

        Camera cam;
        cam.Clone(self->Cam());
        cam.Yaw(az);
        cam.Pitch(-el);

        result = Point(obj_t * cam.vrt(),
        obj_t * cam.vup(),
        obj_t * cam.vpn());
    }
    else {
        Camera& cam = (Camera&) self->Cam();   // cast away const

        result = Point(obj_t * cam.vrt(),
        obj_t * cam.vup(),
        obj_t * cam.vpn());
    }

    return result;
}

Point
SteerAI::AimTransform(const Point& pt)
{
    Camera& cam  = (Camera&) self->Cam();   // cast away const
    Point obj_t  = pt - self->Location();

    Point result = Point(obj_t * cam.vrt(),
    obj_t * cam.vup(),
    obj_t * cam.vpn());

    return result;
}

// +--------------------------------------------------------------------+

void
SteerAI::Navigator()
{
    accumulator.Clear();
    magnitude = 0;
}

int
SteerAI::Accumulate(const Steer& steer)
{
    int overflow = 0;

    double mag = steer.Magnitude();

    if (magnitude + mag > 1) {
        overflow = 1;
        double scale = (1 - magnitude) / mag;
        
        accumulator += steer * scale;
        magnitude   =  1;
        
        if (seeking) {
            az[0] *= scale;
            el[0] *= scale;
            seeking = 0;
        }
    }
    else {
        accumulator += steer;
        magnitude   += mag;
    }

    return overflow;
}

// +--------------------------------------------------------------------+

Steer
SteerAI::Seek(const Point& point)
{
    Steer s;

    // advance memory pipeline:
    az[2] = az[1]; az[1] = az[0];
    el[2] = el[1]; el[1] = el[0];

    // approach
    if (point.z > 0.0f) {
        az[0] = atan2(fabs(point.x), point.z) * seek_gain;
        el[0] = atan2(fabs(point.y), point.z) * seek_gain;

        if (point.x < 0) az[0] = -az[0];
        if (point.y > 0) el[0] = -el[0];

        s.yaw   = az[0] - seek_damp * (az[1] + az[2] * 0.5);
        s.pitch = el[0] - seek_damp * (el[1] + el[2] * 0.5);
    }

    // reverse
    else {
        if (point.x > 0) s.yaw = 1.0f;
        else             s.yaw = -1.0f;

        s.pitch = -point.y * 0.5f;
    }

    seeking = 1;

    return s;
}

// +--------------------------------------------------------------------+

Steer
SteerAI::Flee(const Point& pt)
{
    Steer s;

    Point point = pt;
    point.Normalize();

    // approach
    if (point.z > 0.0f) {
        if (point.x > 0) s.yaw = -1.0f;
        else             s.yaw =  1.0f;
    }

    // flee
    else {
        s.yaw   = -point.x;
        s.pitch =  point.y;
    }

    return s;
}

// +--------------------------------------------------------------------+

Steer
SteerAI::Avoid(const Point& point, float radius)
{
    Steer s;

    if (point.z > 0) {
        double ax = radius - fabs(point.x);
        double ay = radius - fabs(point.y);

        // go around?
        if (ax < ay) {
            s.yaw = atan2(ax, point.z) * seek_gain;
            if (point.x > 0) s.yaw = -s.yaw;
        }

        // go over/under:
        else {
            s.pitch = atan2(ay, point.z) * seek_gain;
            if (point.y < 0) s.pitch = -s.pitch;
        }
    }

    return s;
}

// +--------------------------------------------------------------------+

Steer
SteerAI::Evade(const Point& point, const Point& vel)
{
    Steer evade;

    if (Game::GameTime() - evade_time > 1250) {
        evade_time = Game::GameTime();

        int   direction = (rand()>>9) & 0x07;

        switch (direction) {
        default:
        case 0:  evade.yaw =  0; evade.pitch = -0.5; break;
        case 1:  evade.yaw =  0; evade.pitch = -1.0; break;
        case 2:  evade.yaw =  1; evade.pitch = -0.3; break;
        case 3:  evade.yaw =  1; evade.pitch = -0.6; break;
        case 4:  evade.yaw =  1; evade.pitch = -1.0; break;
        case 5:  evade.yaw = -1; evade.pitch = -0.3; break;
        case 6:  evade.yaw = -1; evade.pitch = -0.6; break;
        case 7:  evade.yaw = -1; evade.pitch = -1.0; break;
        }
    }

    return evade;
}

