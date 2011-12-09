/*  Project STARSHATTER
	John DiCamillo
	Copyright © 1997-2002. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         FlightDeck.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Everything needed to launch and recover space craft

	See Also: Hangar
*/

#ifndef FlightDeck_h
#define FlightDeck_h

#include "Types.h"
#include "Geometry.h"
#include "System.h"
#include "SimObject.h"
#include "Text.h"

// +----------------------------------------------------------------------+

class Hoop;
class Light;
class Ship;
class ShipDesign;
class FlightDeck;
class FlightDeckSlot;
class InboundSlot;

// +======================================================================+

class InboundSlot : public SimObserver
{
public:
	static const char* TYPENAME() { return "InboundSlot"; }

	InboundSlot() : ship(0), deck(0), squadron(0), slot(0), cleared(0), final(0), approach(0) { }
	InboundSlot(Ship* s, FlightDeck* d, int squad, int index);

	int operator <  (const InboundSlot& that) const;
	int operator <= (const InboundSlot& that) const;
	int operator == (const InboundSlot& that) const;

	// SimObserver:
	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

	Ship*             GetShip()      { return ship;     }
	FlightDeck*       GetDeck()      { return deck;     }
	int               Squadron()     { return squadron; }
	int               Index()        { return slot;     }
	int               Cleared()      { return cleared;  }
	int               Final()        { return final;    }
	int               Approach()     { return approach; }
	Point             Offset()       { return offset;   }
	double            Distance();

	void              SetApproach(int a) { approach = a; }
	void              SetOffset(const Point& p) { offset = p; }
	void              SetFinal(int f) { final = f; }
	void              Clear(bool clear=true);

private:
	Ship*             ship;
	FlightDeck*       deck;
	int               squadron;
	int               slot;
	int               cleared;
	int               final;
	int               approach;
	Point             offset;
};

// +----------------------------------------------------------------------+

class FlightDeck : public System, public SimObserver
{
public:
	static const char* TYPENAME() { return "FlightDeck"; }

	FlightDeck();
	FlightDeck(const FlightDeck& rhs);
	virtual ~FlightDeck();

	enum FLIGHT_DECK_MODE   { FLIGHT_DECK_LAUNCH, FLIGHT_DECK_RECOVERY      };
	enum FLIGHT_SLOT_STATE  { CLEAR, READY, QUEUED, LOCKED, LAUNCH, DOCKING };
	enum CONSTANTS          { NUM_APPROACH_PTS = 8 };

	static void    Initialize();
	static void    Close();

	virtual void   ExecFrame(double seconds);
	void           SetCarrier(Ship* s)     { ship = carrier = s; }
	void           SetIndex(int n)         { index = n;   }

	virtual int    SpaceLeft(int type) const;

	virtual bool   Spot(Ship* s, int& index);
	virtual bool   Clear(int index);
	virtual bool   Launch(int index);
	virtual bool   LaunchShip(Ship* s);
	virtual bool   Recover(Ship* s);
	virtual bool   Dock(Ship* s);
	virtual int    Inbound(InboundSlot*& s);
	virtual void   GrantClearance();

	virtual void   AddSlot(const Point& loc, DWORD filter=0xf);

	virtual bool   IsLaunchDeck()       const { return subtype == FLIGHT_DECK_LAUNCH;   }
	virtual void   SetLaunchDeck()            {        subtype =  FLIGHT_DECK_LAUNCH;   }
	virtual bool   IsRecoveryDeck()     const { return subtype == FLIGHT_DECK_RECOVERY; }
	virtual void   SetRecoveryDeck()          {        subtype =  FLIGHT_DECK_RECOVERY; }

	Point          BoundingBox()        const { return box;                             }
	Point          ApproachPoint(int i) const { return approach_point[i];               }
	Point          RunwayPoint(int i)   const { return runway_point[i];                 }
	Point          StartPoint()         const { return start_point;                     }
	Point          EndPoint()           const { return end_point;                       }
	Point          CamLoc()             const { return cam_loc;                         }
	double         Azimuth()            const { return azimuth;                         }

	virtual void   SetBoundingBox(Point dimensions) { box = dimensions; }
	virtual void   SetApproachPoint(int i, Point loc);
	virtual void   SetRunwayPoint(int i, Point loc);
	virtual void   SetStartPoint(Point loc);
	virtual void   SetEndPoint(Point loc);
	virtual void   SetCamLoc(Point loc);
	virtual void   SetCycleTime(double time);
	virtual void   SetAzimuth(double az)      { azimuth = az; }
	virtual void   SetLight(double l);

	virtual void   Orient(const Physical* rep);

	// SimObserver:
	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

	// accessors:
	int            NumSlots()                 const { return num_slots; }
	double         TimeRemaining(int index)   const;
	int            State(int index)           const;
	int            Sequence(int index)        const;
	const Ship*    GetCarrier()               const { return carrier; }
	int            GetIndex()                 const { return index;   }
	Ship*          GetShip(int index)         const;
	int            NumHoops()                 const { return num_hoops; }
	Hoop*          GetHoops()                 const { return hoops;     }
	Light*         GetLight()                       { return light;     }

	List<InboundSlot>& GetRecoveryQueue()           { return recovery_queue; }
	void           PrintQueue();

	bool           OverThreshold(Ship* s)        const;
	bool           ContainsPoint(const Point& p) const;

protected:
	Ship*             carrier;
	int               index;
	int               num_slots;
	FlightDeckSlot*   slots;

	Point             box;
	Point             start_rel;
	Point             end_rel;
	Point             cam_rel;
	Point             approach_rel[NUM_APPROACH_PTS];
	Point             runway_rel[2];

	Point             start_point;
	Point             end_point;
	Point             cam_loc;
	Point             approach_point[NUM_APPROACH_PTS];
	Point             runway_point[2];

	double            azimuth;
	double            cycle_time;

	int               num_approach_pts;
	int               num_catsounds;
	int               num_hoops;
	Hoop*             hoops;
	Light*            light;
	List<InboundSlot> recovery_queue;
};

// +----------------------------------------------------------------------+

#endif FlightDeck_h

