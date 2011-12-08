/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         System.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Generic Ship Subsystem class 
*/

#include "MemDebug.h"
#include "System.h"
#include "SystemDesign.h"
#include "Component.h"
#include "NetUtil.h"

#include "Game.h"

// +----------------------------------------------------------------------+

System::System(System::CATEGORY t, int s, const char* n, int maxv, 
               double e, double c, double r)
   : type(t), id(0), ship(0), subtype(s), status(NOMINAL), availability(1.0f),
     safety(1.0f), stability(1.0f), crit_level(0.5f), net_avail(-1.0f),
     mount_rel(0.0f, 0.0f, 0.0f), radius(0.0f), safety_overload(0.0f),
     hull_factor(0.5f), energy((float) e), capacity((float) c), sink_rate((float) r),
     power_level(1.0f), power_flags(0), source_index(0), power_on(true),
     explosion_type(0), name(n), abrv(name), design(0), emcon(3)
{
   if (maxv < 100)
      max_value = maxv;
   else
      max_value = (int) (maxv/100.0);

   emcon_power[0] = 100;
   emcon_power[1] = 100;
   emcon_power[2] = 100;
}

// +----------------------------------------------------------------------+

System::System(const System& s)
   : type(s.type), id(s.id), ship(0), subtype(s.subtype), status(s.status),
     availability(s.availability), safety(s.safety), stability(s.stability), 
     crit_level(s.crit_level), net_avail(-1.0f),
     mount_rel(s.mount_rel), radius(s.radius), safety_overload(0.0f),
     hull_factor(s.hull_factor), energy(s.energy), capacity(s.capacity),
     sink_rate(s.sink_rate),  power_level(s.power_level), power_flags(s.power_flags),
     source_index(s.source_index), power_on(s.power_on), max_value(s.max_value),
     explosion_type(s.explosion_type), name(s.name), abrv(s.abrv), design(s.design),
     emcon(s.emcon)
{
   if (design) {
                                 // cast-away const
      ListIter<Component> c = (List<Component>&) s.components;
      while (++c) {
         Component* comp = new(__FILE__,__LINE__) Component(*(c.value()));
         comp->SetSystem(this);
         components.append(comp);
      }
   }

   emcon_power[0] = s.emcon_power[0];
   emcon_power[1] = s.emcon_power[1];
   emcon_power[2] = s.emcon_power[2];
}

// +--------------------------------------------------------------------+

System::~System()
{
   components.destroy();
}

// +--------------------------------------------------------------------+

void
System::SetDesign(SystemDesign* d)
{
   if (design) {
      design = 0;
      components.destroy();
   }

   if (d) {
      design = d;

      ListIter<ComponentDesign> cd = design->components;
      while (++cd) {
         Component* comp = new(__FILE__,__LINE__) Component(cd.value(), this);
         components.append(comp);
      }
   }
}

// +--------------------------------------------------------------------+

void
System::SetPowerLevel(double level)
{
   if (level > 100)
      level = 100;
   else if (level < 0)
      level = 0;

   level /= 100;

   if (power_level != level) {
      // if the system is on emergency override power,
      // do not let the EMCON system use this method
      // to drop it back to normal power:
      if (power_level > 1 && level == 1)
         return;

      power_level = (float) level;

      NetUtil::SendSysStatus(ship, this);
   }
}

void
System::SetOverride(bool over)
{
   bool changed = false;

   if (over && power_level != 1.2f) {
      power_level = 1.2f;
      changed = true;
   }

   else if (!over && power_level > 1) {
      power_level = 1.0f;
      changed = true;
   }

   if (changed)
      NetUtil::SendSysStatus(ship, this);
}

void
System::SetEMCONPower(int index, int power_level)
{
   if (index >= 1 && index <= 3) {
      emcon_power[index-1] = (BYTE) power_level;
   }
}

int
System::GetEMCONPower(int index)
{
   if (index >= 1 && index <= 3) {
      return emcon_power[index-1];
   }

   return 100;
}

void
System::DoEMCON(int index)
{
   int e = GetEMCONPower(index);

   if (power_level * 100 > e || emcon != index) {
      if (e == 0) {
         PowerOff();
      }
      else {
         if (emcon != index)
            PowerOn();

         SetPowerLevel(e);
      }
   }

   emcon = index;
}

// +--------------------------------------------------------------------+

void
System::ExecFrame(double seconds)
{
   if (seconds < 0.01)
      seconds = 0.01;

   STATUS s = DESTROYED;

   if (availability > 0.99)
      s = NOMINAL;
   else if (availability > crit_level)
      s = DEGRADED;
   else
      s = CRITICAL;

   bool repair = false;

   if (components.size() > 0) {
      ListIter<Component> comp = components;
      while (++comp) {
         if (comp->Status() > Component::NOMINAL) {
            repair = true;
            break;
         }
      }
   }

   if (repair) {
      Repair();
   }

   else {
      if (status != s) {
         status = s;
         NetUtil::SendSysStatus(ship, this);
      }

      // collateral damage due to unsafe operation:
      if (power_on && power_level > safety) {
         safety_overload += (float) seconds;

         // inflict some damage now:
         if (safety_overload > 60) {
            safety_overload -= (float) (rand() / (1000 * (power_level-safety)));
            ApplyDamage(15);

            NetUtil::SendSysStatus(ship, this);
         }
      }

      else if (safety_overload > 0) {
         safety_overload -= (float) seconds;
      }
   }
}

void
System::Repair()
{
   if (status != MAINT) {
      status = MAINT;
      safety_overload = 0.0f;

      NetUtil::SendSysStatus(ship, this);
   }
}

// +--------------------------------------------------------------------+

void
System::ExecMaintFrame(double seconds)
{
   if (components.size() > 0) {
      ListIter<Component> comp = components;
      while (++comp) {
         if (comp->Status() > Component::NOMINAL) {
            comp->ExecMaintFrame(seconds);
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
System::ApplyDamage(double damage)
{
   if (!power_on)
      damage /= 10;

   if (components.size() > 0) {
      int index = rand() % components.size();

      if (damage > 50) {
         damage/=2;
         components[index]->ApplyDamage(damage);

         index = rand() % components.size();
      }

      components[index]->ApplyDamage(damage);

      if (safety < 0.5)
         SetPowerLevel(50);

      else if (safety < 1.0)
         SetPowerLevel(safety * 100);
   }
   else {
      availability -= (float) (damage/100.0f);
      if (availability < 0.01) availability = 0.0f;
   }
}

void
System::CalcStatus()
{
   if (components.size() > 0) {
      availability = 1.0f;
      safety       = 1.0f;
      stability    = 1.0f;

      ListIter<Component> comp = components;
      while (++comp) {
         if (comp->DamageEfficiency())
            availability *= comp->Availability() / 100.0f;

         if (comp->DamageSafety())
            safety       *= comp->Availability() / 100.0f;

         if (comp->DamageStability())
            stability    *= comp->Availability() / 100.0f;

         if (comp->IsJerried()) {
            safety       *= 0.95f;
            stability    *= 0.95f;
         }
      }

      if (net_avail >= 0 && availability < net_avail)
         availability = net_avail;
   }
}

// +--------------------------------------------------------------------+

void
System::Mount(Point loc, float rad, float hull)
{
   mount_rel   = loc;
   radius      = rad;
   hull_factor = hull;
}

void
System::Mount(const System& system)
{
   mount_rel   = system.mount_rel;
   radius      = system.radius;
   hull_factor = system.hull_factor;
}

// +--------------------------------------------------------------------+

void
System::Orient(const Physical* rep)
{
   const Matrix& orientation = rep->Cam().Orientation();
   Point         loc         = rep->Location();

   mount_loc = (mount_rel * orientation) + loc;
}

// +----------------------------------------------------------------------+

double
System::GetRequest(double seconds) const
{
   if (!power_on || capacity == energy)
      return 0;

   else
      return power_level * sink_rate * seconds;
}

// +----------------------------------------------------------------------+

void
System::Distribute(double delivered_energy, double seconds)
{
   if (UsesWatts()) {
      if (seconds < 0.01)
         seconds = 0.01;

      // convert Joules to Watts:
      energy = (float) (delivered_energy/seconds);
   }

   else if (!Game::Paused()) {
      energy += (float) delivered_energy;

      if (energy > capacity)
         energy = capacity;

      else if (energy < 0)
         energy = 0.0f;
   }
}

// +----------------------------------------------------------------------+

void
System::DrainPower(double to_level)
{
   energy = 0.0f;
}
