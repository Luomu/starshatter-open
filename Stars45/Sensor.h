/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Sensor.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Integrated (Passive and Active) Sensor Package class
*/

#ifndef Sensor_h
#define Sensor_h

#include "Types.h"
#include "SimObject.h"
#include "System.h"
#include "Geometry.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Shot;
class Contact;

// +--------------------------------------------------------------------+

class Sensor : public System, public SimObserver
{
public:
   enum Mode {
      PAS, STD, ACM, GM,   // fighter modes
      PST, CST             // starship modes
   };

   Sensor();
   Sensor(const Sensor& rhs);
   virtual ~Sensor();

   virtual void      ExecFrame(double seconds);
   virtual SimObject* LockTarget(int  type=SimObject::SIM_SHIP,
                                 bool closest=false,
                                 bool hostile=false);
   virtual SimObject* LockTarget(SimObject* candidate);
   virtual bool      IsTracking(SimObject* tgt);
   virtual void      DoEMCON(int emcon);

   virtual void      ClearAllContacts();

   virtual Mode      GetMode()         const { return mode; }
   virtual void      SetMode(Mode m);
   virtual double    GetBeamLimit()    const;
   virtual double    GetBeamRange()    const;
   virtual void      IncreaseRange();
   virtual void      DecreaseRange();
   virtual void      AddRange(double r);

   Contact*          FindContact(Ship* s);
   Contact*          FindContact(Shot* s);

   // borrow this sensor for missile seeker
   SimObject*        AcquirePassiveTargetForMissile();
   SimObject*        AcquireActiveTargetForMissile();

   // SimObserver:
   virtual bool         Update(SimObject* obj);
   virtual const char*  GetObserverName() const;

protected:
   void              ProcessContact(Ship* contact, double az1, double az2);
   void              ProcessContact(Shot* contact, double az1, double az2);

   Mode              mode;
   int               nsettings;
   int               range_index;
   float             range_settings[8];
   SimObject*        target;

   List<Contact>     contacts;
};

#endif Sensor_h

