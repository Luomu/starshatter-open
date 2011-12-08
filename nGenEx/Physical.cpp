/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Physical.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract Physical Object
*/

#include "MemDebug.h"
#include "Physical.h"
#include "Graphic.h"
#include "Light.h"
#include "Director.h"

// +--------------------------------------------------------------------+

int      Physical::id_key     = 1;
double   Physical::sub_frame  = 1.0 / 60.0;

static const double GRAV = 6.673e-11;

// +--------------------------------------------------------------------+

Physical::Physical()
   : id(id_key++), obj_type(0), rep(0), light(0),
     thrust(0.0f), drag(0.0f), lat_thrust(false),
     trans_x(0.0f), trans_y(0.0f), trans_z(0.0f), straight(false),
     roll(0.0f), pitch(0.0f), yaw(0.0f), dr(0.0f), dp(0.0f), dy(0.0f),
     dr_acc(0.0f), dp_acc(0.0f), dy_acc(0.0f),
     dr_drg(0.0f), dp_drg(0.0f), dy_drg(0.0f),
     flight_path_yaw(0.0f), flight_path_pitch(0.0f), primary_mass(0),
     roll_rate(1.0f), pitch_rate(1.0f), yaw_rate(1.0f), shake(0.0f),
     radius(0.0f), mass(1.0f), integrity(1.0f), life(-1), dir(0),
     g_accel(0.0f), Do(0.0f), CL(0.0f), CD(0.0f), alpha(0.0f), stall(0.0f)
{
   strcpy_s(name, "unknown object");
}

// +--------------------------------------------------------------------+

Physical::Physical(const char* n, int t)
   : id(id_key++), obj_type(t), rep(0), light(0),
     thrust(0.0f), drag(0.0f), lat_thrust(false),
     trans_x(0.0f), trans_y(0.0f), trans_z(0.0f), straight(false),
     roll(0.0f), pitch(0.0f), yaw(0.0f), dr(0.0f), dp(0.0f), dy(0.0f),
     dr_acc(0.0f), dp_acc(0.0f), dy_acc(0.0f),
     dr_drg(0.0f), dp_drg(0.0f), dy_drg(0.0f),
     flight_path_yaw(0.0f), flight_path_pitch(0.0f), primary_mass(0),
     roll_rate(1.0f), pitch_rate(1.0f), yaw_rate(1.0f), shake(0.0f),
     radius(0.0f), mass(1.0f), integrity(1.0f), life(-1), dir(0),
     g_accel(0.0f), Do(0.0f), CL(0.0f), CD(0.0f), alpha(0.0f), stall(0.0f)
{
   strncpy_s(name, n, NAMELEN-1);
   name[NAMELEN-1] = 0;
}

// +--------------------------------------------------------------------+

Physical::~Physical()
{
   // inform graphic rep and light that we are leaving:
   GRAPHIC_DESTROY(rep);
   LIGHT_DESTROY(light);

   // we own the director
   delete dir;
   dir = 0;
}

// +--------------------------------------------------------------------+

inline double random() { return rand()-16384; }

void
Physical::ExecFrame(double s)
{
   Point orig_velocity = Velocity();
   arcade_velocity = Point();

   // if this object is under direction,
   // but doesn't need subframe accuracy,
   // update the control parameters:
   if (dir && !dir->Subframe())
      dir->ExecFrame(s);

   // decrement life before destroying the frame time:
   if (life > 0)
      life -= s;

   // integrate equations
   // using slices no larger
   // than sub_frame:

   double seconds = s;

   while (s > 0.0) {
      if (s > sub_frame)
         seconds = sub_frame;
      else
         seconds = s;

      // if the director needs subframe accuracy, run it now:
      if (dir && dir->Subframe())
         dir->ExecFrame(seconds);

      if (!straight)
         AngularFrame(seconds);

      // LINEAR MOVEMENT ----------------------------
      Point pos = cam.Pos();

      // if the object is thrusting,
      // accelerate along the camera normal:
      if (thrust) {
         Point thrustvec = cam.vpn();
         thrustvec *= ((thrust/mass) * seconds);
         velocity += thrustvec;
      }

      LinearFrame(seconds);

      // move the position by the (time-frame scaled) velocity:
      pos += velocity * seconds;
      cam.MoveTo(pos);

      s -= seconds;
   }

   alpha = 0.0f;

   // now update the graphic rep and light sources:
   if (rep) {
      rep->MoveTo(cam.Pos());
      rep->SetOrientation(cam.Orientation());
   }
   
   if (light) {
      light->MoveTo(cam.Pos());
   }

   if (!straight)
      CalcFlightPath();

   accel = (Velocity() - orig_velocity) * (1/seconds);
   if (!_finite(accel.x) || !_finite(accel.y) || !_finite(accel.z))
      accel = Point();
}

// +--------------------------------------------------------------------+

void
Physical::AeroFrame(double s)
{
   arcade_velocity = Point();

   // if this object is under direction,
   // but doesn't need subframe accuracy,
   // update the control parameters:
   if (dir && !dir->Subframe())
      dir->ExecFrame(s);

   // decrement life before destroying the frame time:
   if (life > 0)
      life -= s;

   // integrate equations
   // using slices no larger
   // than sub_frame:

   double seconds = s;

   while (s > 0.0) {
      if (s > sub_frame)
         seconds = sub_frame;
      else
         seconds = s;

      // if the director needs subframe accuracy, run it now:
      if (dir && dir->Subframe())
         dir->ExecFrame(seconds);

      AngularFrame(seconds);

      // LINEAR MOVEMENT ----------------------------
      Point pos = cam.Pos();

      // if the object is thrusting,
      // accelerate along the camera normal:
      if (thrust) {
         Point thrustvec = cam.vpn();
         thrustvec *= ((thrust/mass) * seconds);
         velocity += thrustvec;
      }

      // AERODYNAMICS ------------------------------

      if (lat_thrust)
         LinearFrame(seconds);

      // if no thrusters, do constant gravity:
      else if (g_accel > 0)
         velocity += Point(0, -g_accel, 0) * seconds;

      // compute alpha, rho, drag, and lift:

      Point  vfp     = velocity;
      double v       = vfp.Normalize();
      double v_2     = 0;
      double rho     = GetDensity();
      double lift    = 0;

      if (v > 150) {
         v_2 = (v-150) * (v-150);

         Point  vfp1 = vfp - cam.vrt() * (vfp * cam.vrt());
         vfp1.Normalize();

         double cos_alpha = vfp1 * cam.vpn();

         if (cos_alpha >= 1) {
            alpha = 0.0f;
         }
         else {
            alpha = (float) acos(cos_alpha);
         }

         // if flight path is above nose, alpha is negative:
         if (vfp1 * cam.vup() > 0)
            alpha = -alpha;

         if (alpha <= stall) {
            lift = CL * alpha * rho * v_2;
         }
         else {
            lift = CL * (2*stall - alpha) * rho * v_2;
         }

         // add lift to velocity:
         if (_finite(lift))
            velocity += cam.vup() * lift * seconds;
         else
            lift = 0;

         // if drag applies, decellerate:
         double alpha_2 = alpha*alpha;
         double drag_eff = (drag + (CD * alpha_2)) * rho * v_2;

         Point vn = velocity;
         vn.Normalize();

         velocity += vn * -drag_eff * seconds;
      }
      else {
         velocity *= exp(-drag * seconds);
      }

      // move the position by the (time-frame scaled) velocity:
      pos += velocity * seconds;
      cam.MoveTo(pos);

      s -= seconds;
   }

   // now update the graphic rep and light sources:
   if (rep) {
      rep->MoveTo(cam.Pos());
      rep->SetOrientation(cam.Orientation());
   }
   
   if (light) {
      light->MoveTo(cam.Pos());
   }
}

double
Physical::GetDensity() const
{
   double alt = cam.Pos().y;
   double rho = 0.75 * Do * (250e3-alt)/250e3;

   return rho;
}

// +--------------------------------------------------------------------+

void
Physical::ArcadeFrame(double s)
{
   // if this object is under direction,
   // but doesn't need subframe accuracy,
   // update the control parameters:
   if (dir && !dir->Subframe())
      dir->ExecFrame(s);

   // decrement life before destroying the frame time:
   if (life > 0)
      life -= s;

   // integrate equations
   // using slices no larger
   // than sub_frame:

   double seconds = s;

   while (s > 0.0) {
      if (s > sub_frame)
         seconds = sub_frame;
      else
         seconds = s;

      // if the director needs subframe accuracy, run it now:
      if (dir && dir->Subframe())
         dir->ExecFrame(seconds);

      if (!straight)
         AngularFrame(seconds);

      Point pos = cam.Pos();

      // ARCADE FLIGHT MODEL:
      // arcade_velocity vector is always in line with heading

      double speed = arcade_velocity.Normalize();
      double bleed = arcade_velocity * cam.vpn();

      speed *= pow(bleed, 30);
      arcade_velocity = cam.vpn() * speed;

      if (thrust) {
         Point thrustvec = cam.vpn();
         thrustvec *= ((thrust/mass) * seconds);
         arcade_velocity += thrustvec;
      }

      if (drag)
         arcade_velocity *= exp(-drag * seconds);

      LinearFrame(seconds);

      // move the position by the (time-frame scaled) velocity:
      pos += arcade_velocity * seconds + 
             velocity        * seconds;

      cam.MoveTo(pos);

      s -= seconds;
   }

   alpha = 0.0f;

   // now update the graphic rep and light sources:
   if (rep) {
      rep->MoveTo(cam.Pos());
      rep->SetOrientation(cam.Orientation());
   }
   
   if (light) {
      light->MoveTo(cam.Pos());
   }
}

// +--------------------------------------------------------------------+

void
Physical::AngularFrame(double seconds)
{
   if (!straight) {
      dr += (float) (dr_acc * seconds);
      dy += (float) (dy_acc * seconds);
      dp += (float) (dp_acc * seconds);
      
      dr *= (float) exp(-dr_drg * seconds);
      dy *= (float) exp(-dy_drg * seconds);
      dp *= (float) exp(-dp_drg * seconds);

      roll  = (float) (dr * seconds);
      pitch = (float) (dp * seconds);
      yaw   = (float) (dy * seconds);

      if (shake > 0.01) {
         vibration = Point(random(), random(), random());
         vibration.Normalize();
         vibration *= (float) (shake * seconds);

         shake *= (float) exp(-1.5 * seconds);
      }
      else {
         vibration.x = vibration.y = vibration.z = 0.0f;
         shake = 0.0f;
      }

      cam.Aim(roll, pitch, yaw);
   }
}

// +--------------------------------------------------------------------+

void
Physical::LinearFrame(double seconds)
{
   // deal with lateral thrusters:

   if (trans_x) { // side-to-side
      Point transvec = cam.vrt();
      transvec *= ((trans_x/mass) * seconds);

      velocity += transvec;
   }

   if (trans_y) { // fore-and-aft
      Point transvec = cam.vpn();
      transvec *= ((trans_y/mass) * seconds);

      velocity += transvec;
   }

   if (trans_z) { // up-and-down
      Point transvec = cam.vup();
      transvec *= ((trans_z/mass) * seconds);

      velocity += transvec;
   }

   // if gravity applies, attract:
   if (primary_mass > 0) {
      Point  g = primary_loc - cam.Pos();
      double r = g.Normalize();

      g *= GRAV * primary_mass / (r*r);

      velocity += g * seconds;
   }

   // constant gravity:
   else if (g_accel > 0)
      velocity += Point(0, -g_accel, 0) * seconds;

   // if drag applies, decellerate:
   if (drag)
      velocity *= exp(-drag * seconds);
}

// +--------------------------------------------------------------------+

void
Physical::CalcFlightPath()
{
   flight_path_yaw   = 0.0f;
   flight_path_pitch = 0.0f;

   // transform flight path into camera frame:
   Point flight_path = velocity;
   if (flight_path.Normalize() < 1)
      return;

   Point tmp = flight_path;
   flight_path.x = tmp * cam.vrt();
   flight_path.y = tmp * cam.vup();
   flight_path.z = tmp * cam.vpn();

   if (flight_path.z < 0.1)
      return;

   // first, compute azimuth:
   flight_path_yaw = (float) atan(flight_path.x / flight_path.z);
   if (flight_path.z < 0)     flight_path_yaw -= (float) PI;
   if (flight_path_yaw < -PI) flight_path_yaw += (float) (2*PI);

   // then, rotate path into azimuth frame to compute elevation:
   Camera yaw_cam;
   yaw_cam.Clone(cam);
   yaw_cam.Yaw(flight_path_yaw);

   flight_path.x = tmp * yaw_cam.vrt();
   flight_path.y = tmp * yaw_cam.vup();
   flight_path.z = tmp * yaw_cam.vpn();

   flight_path_pitch = (float) atan(flight_path.y / flight_path.z);
}

// +--------------------------------------------------------------------+

void
Physical::MoveTo(const Point& new_loc)
{
   cam.MoveTo(new_loc);
}

void
Physical::TranslateBy(const Point& ref)
{
   Point new_loc = cam.Pos() - ref;
   cam.MoveTo(new_loc);
}

void
Physical::ApplyForce(const Point& force)
{
   velocity += force/mass;
}

void
Physical::ApplyTorque(const Point& torque)
{
   dr += (float) (torque.x/mass);
   dp += (float) (torque.y/mass);
   dy += (float) (torque.z/mass);
}

void
Physical::SetThrust(double t)
{
   thrust = (float) t;
}

void
Physical::SetTransX(double t)
{
   trans_x = (float) t;
}

void
Physical::SetTransY(double t)
{
   trans_y = (float) t;
}

void
Physical::SetTransZ(double t)
{
   trans_z = (float) t;
}

// +--------------------------------------------------------------------+

void
Physical::SetHeading(double r, double p, double y)
{
   roll  = (float) r;
   pitch = (float) p;
   yaw   = (float) y;

   cam.Aim(roll, pitch, yaw);
}

void
Physical::LookAt(const Point& dst)
{
   cam.LookAt(dst);
}

void
Physical::CloneCam(const Camera& c)
{
   cam.Clone(c);
}

void
Physical::SetAbsoluteOrientation(double r, double p, double y)
{
   roll  = (float) r;
   pitch = (float) p;
   yaw   = (float) y;

   Camera work(Location().x, Location().y, Location().z);
   work.Aim(r,p,y);
   cam.Clone(work);
}

void
Physical::ApplyRoll(double r)
{
   if (r > 1)       r =  1;
   else if (r < -1) r = -1;

   dr_acc = (float) r * roll_rate;
}

void
Physical::ApplyPitch(double p)
{
   if (p > 1)       p =  1;
   else if (p < -1) p = -1;

   dp_acc = (float) p * pitch_rate;
}

void
Physical::ApplyYaw(double y)
{
   if (y > 1)       y =  1;
   else if (y < -1) y = -1;

   dy_acc = (float) y * yaw_rate;
}

void
Physical::SetAngularRates(double  r, double  p, double  y)
{
   roll_rate  = (float) r;
   pitch_rate = (float) p;
   yaw_rate   = (float) y;
}

void
Physical::GetAngularRates(double& r, double& p, double& y)
{
   r = roll_rate;
   p = pitch_rate;
   y = yaw_rate;
}

void
Physical::SetAngularDrag(double  r, double  p, double  y)
{
   dr_drg = (float) r;
   dp_drg = (float) p;
   dy_drg = (float) y;
}

void
Physical::GetAngularDrag(double& r, double& p, double& y)
{
   r = dr_drg;
   p = dp_drg;
   y = dy_drg;
}

void
Physical::GetAngularThrust(double& r, double& p, double& y)
{
   r = 0;
   p = 0;
   y = 0;

   if      (dr_acc > 0.05 * roll_rate)    r =  1;
   else if (dr_acc < -0.05 * roll_rate)   r = -1;
   else if (dr > 0.01 * roll_rate)        r = -1;
   else if (dr < -0.01 * roll_rate)       r =  1;

   if      (dy_acc > 0.05 * yaw_rate)     y =  1;
   else if (dy_acc < -0.05 * yaw_rate)    y = -1;
   else if (dy > 0.01 * yaw_rate)         y = -1;
   else if (dy < -0.01 * yaw_rate)        y =  1;

   if      (dp_acc > 0.05 * pitch_rate)   p =  1;
   else if (dp_acc < -0.05 * pitch_rate)  p = -1;
   else if (dp > 0.01 * pitch_rate)       p = -1;
   else if (dp < -0.01 * pitch_rate)      p =  1;
}


void
Physical::SetPrimary(const Point& l, double m)
{
   primary_loc  = l;
   primary_mass = m;
}

void
Physical::SetGravity(double g)
{
   if (g >= 0)
      g_accel = (float) g;
}

void
Physical::SetBaseDensity(double d)
{
   if (d >= 0)
      Do = (float) d;
}

// +--------------------------------------------------------------------+

void
Physical::InflictDamage(double damage, int /*type*/)
{
   integrity -= (float) damage;

   if (integrity < 1.0f)
      integrity = 0.0f;
}

// +--------------------------------------------------------------------+

int
Physical::CollidesWith(Physical& o)
{
   // representation collision test (will do bounding spheres first):
   if (rep && o.rep)
      return rep->CollidesWith(*o.rep);

   Point delta_loc = Location() - o.Location();

   // bounding spheres test:
   if (delta_loc.length() > radius + o.radius)
      return 0;

   // assume collision:
   return 1;
}


// +--------------------------------------------------------------------+

void
Physical::ElasticCollision(Physical& a, Physical& b)
{
   double mass_sum   = a.mass + b.mass;
   double mass_delta = a.mass - b.mass;

   Point vel_a = (Point(b.velocity) * (2 * b.mass) + Point(a.velocity) * mass_delta) * (1/mass_sum);
   Point vel_b = (Point(a.velocity) * (2 * a.mass) - Point(b.velocity) * mass_delta) * (1/mass_sum);

   a.velocity = vel_a;
   b.velocity = vel_b;
}

// +--------------------------------------------------------------------+

void
Physical::InelasticCollision(Physical& a, Physical& b)
{
   double mass_sum   = a.mass + b.mass;

   Point vel_a = (Point(a.velocity) * a.mass + Point(b.velocity) * b.mass) * (1/mass_sum);

   a.velocity = vel_a;
   b.velocity = vel_a;
}

// +--------------------------------------------------------------------+

void
Physical::SemiElasticCollision(Physical& a, Physical& b)
{
   double mass_sum   = a.mass + b.mass;
   double mass_delta = a.mass - b.mass;

   Point avel  = a.Velocity();
   Point bvel  = b.Velocity();
   Point dv    = avel - bvel;

   // low delta-v: stick
   if (dv.length() < 20) {
      if (a.mass > b.mass) {
         b.velocity = a.velocity;
      }

      else {
         a.velocity = b.velocity;
      }
   }

   // high delta-v: bounce
   else {
      Point Ve_a  = (bvel * (2 * b.mass) + avel * mass_delta) * (1/mass_sum) * 0.65;
      Point Ve_b  = (avel * (2 * a.mass) - bvel * mass_delta) * (1/mass_sum) * 0.65;
      Point Vi_ab = (avel * a.mass + bvel * b.mass)           * (1/mass_sum) * 0.35;

      a.arcade_velocity = Point();
      b.arcade_velocity = Point();

      a.velocity = Ve_a + Vi_ab;
      b.velocity = Ve_b + Vi_ab;
   }
}

