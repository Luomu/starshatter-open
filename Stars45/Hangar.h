/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Hangar.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Everything needed to store and maintain space craft

    See Also: FlightDeck
*/

#ifndef Hangar_h
#define Hangar_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Ship;
class ShipDesign;
class FlightDeck;
class FlightDeckSlot;
class Element;
class HangarSlot;
class InboundSlot;
class CombatGroup;

// +--------------------------------------------------------------------+

class Hangar : public SimObserver
{
public:
   Hangar();
   Hangar(const Hangar& rhs);
   virtual ~Hangar();

   enum HANGAR_STATE {  UNAVAIL  = -2, 
                        MAINT    = -1, 
                        STORAGE  =  0, 
                        PREP,
                        ALERT,
                        QUEUED, 
                        LOCKED, 
                        LAUNCH,
                        ACTIVE,
                        APPROACH,
                        RECOVERY
                     };

   enum CONSTANTS    { MAX_SQUADRONS=10 };

   virtual void      ExecFrame(double seconds);
   void              SetShip(Ship* s)     { ship  = s; }

   virtual bool      CreateSquadron(Text squadron, CombatGroup* g,
                                    const ShipDesign* design, int count, int iff=-1,
                                    int* def_load=0, int maint_count=0, int dead_count=0);

   // used only by net client to expedite creation of already active ships:
   virtual bool      GotoActiveFlight(int squadron, int slot, Element* elem, int* loadout);

   virtual bool      GotoAlert(int squadron, int slot, FlightDeck* d, Element* elem=0, int* loadout=0, bool pkg=false, bool expedite=false);
   virtual bool      Ready(int squadron, int slot, FlightDeck* d);
   virtual bool      Launch(int squadron, int slot);
   virtual bool      StandDown(int squadron, int slot);
   virtual bool      CanStow(Ship* s);
   virtual bool      Stow(Ship* s);
   virtual bool      FindSlot(Ship* s, int& squadron, int& slot, int state=UNAVAIL);
   virtual bool      FindSquadronAndSlot(Ship* s, int& squadron, int& slot);
   virtual bool      FindAvailSlot(const ShipDesign* s, int& squadron, int& slot);
   virtual bool      FinishPrep(HangarSlot* slot);
   virtual void      SetAllIFF(int iff);

   virtual bool         Update(SimObject* obj);
   virtual const char*  GetObserverName() const;

   // accessors:
   int               NumSquadrons()          const { return nsquadrons; }
   Text              SquadronName(int n)     const;
   int               SquadronSize(int n)     const;
   int               SquadronIFF(int n)      const;
   const ShipDesign* SquadronDesign(int n)   const;

   int               NumShipsReady(int squadron)            const;
   int               NumShipsMaint(int squadron)            const;
   int               NumShipsDead(int squadron)             const;
   int               NumSlotsEmpty()                        const;

   int               GetActiveElements(List<Element>& active_list);

   const HangarSlot* GetSlot(int squadron, int index)       const;
   Ship*             GetShip(const HangarSlot* s)           const;
   const ShipDesign* GetDesign(const HangarSlot* s)         const;
   FlightDeck*       GetFlightDeck(const HangarSlot* s)     const;
   int               GetFlightDeckSlot(const HangarSlot* s) const;
   int               GetState(const HangarSlot* s)          const;
   double            TimeRemaining(const HangarSlot* s)     const;
   Element*          GetPackageElement(const HangarSlot* s) const;
   const int*        GetLoadout(const HangarSlot* s)        const;
   Text              StatusName(const HangarSlot* s)        const;

   int               PreflightQueue(FlightDeck* d)          const;
   DWORD             GetLastPatrolLaunch()                  const;
   void              SetLastPatrolLaunch(DWORD t);

protected:
   Ship*             ship;
   int               nsquadrons;
   int               nslots[MAX_SQUADRONS];
   Text              names[MAX_SQUADRONS];
   HangarSlot*       squadrons[MAX_SQUADRONS];
   DWORD             last_patrol_launch;
};

// +--------------------------------------------------------------------+


#endif Hangar_h

