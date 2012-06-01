/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Sim.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Simulation Universe and Region classes
*/

#ifndef Sim_h
#define Sim_h

#include "Types.h"
#include "Universe.h"
#include "Scene.h"
#include "Physical.h"
#include "Geometry.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Sim;
class SimRegion;
class SimObject;
class SimObserver;
class SimHyper;
class SimSplash;

class StarSystem;
class Orbital;
class OrbitalRegion;
class Asteroid;

class NetGame;

class CameraDirector;
class Contact;
class Ship;
class ShipDesign;
class System;
class Element;
class Shot;
class Drone;
class Explosion;
class Debris;
class WeaponDesign;
class MotionController;
class Dust;
class Grid;
class Mission;
class MissionElement;
class MissionEvent;
class Hangar;
class FlightDeck;

class Terrain;
class TerrainPatch;

class Model;

// +--------------------------------------------------------------------+

class Sim : public Universe
{
	friend class SimRegion;

public:
	enum { REAL_SPACE, AIR_SPACE };

	Sim(MotionController* ctrl);
	virtual ~Sim();

	static  Sim*         GetSim() { return sim; }

	virtual void         ExecFrame(double seconds);

	void                 LoadMission(Mission* msn, bool preload_textures=false);
	void                 ExecMission();
	void                 CommitMission();
	void                 UnloadMission();

	void                 NextView();
	void                 ShowGrid(int show = true);
	bool                 GridShown() const;

	const char*          FindAvailCallsign(int IFF);
	Element*             CreateElement(const char* callsign, int IFF, int type=0/*PATROL*/);
	void                 DestroyElement(Element* elem);
	Ship*                CreateShip(const char* name,
	const char* reg_num,
	ShipDesign* design,
	const char* rgn_name,
	const Point& loc,
	int         IFF=0,
	int         cmd_ai=0,
	const int*  loadout=0);
	Ship*                FindShip(const char* name, const char* rgn_name=0);
	Shot*                CreateShot(const Point& pos, const Camera& shot_cam, WeaponDesign* d, const Ship* ship=0, SimRegion* rgn=0);
	Explosion*           CreateExplosion(const Point& pos, const Point& vel, int type, float exp_scale, float part_scale, SimRegion* rgn=0, SimObject* source=0, System* sys=0);
	Debris*              CreateDebris(const Point& pos, const Point& vel, Model* model, double mass, SimRegion* rgn=0);
	Asteroid*            CreateAsteroid(const Point& pos, int type, double mass, SimRegion* rgn=0);
	void                 CreateSplashDamage(Ship* ship);
	void                 CreateSplashDamage(Shot* shot);
	void                 DestroyShip(Ship* ship);
	void                 NetDockShip(Ship* ship, Ship* carrier, FlightDeck* deck);

	virtual Ship*        FindShipByObjID(DWORD objid);
	virtual Shot*        FindShotByObjID(DWORD objid);

	Mission*             GetMission()      { return mission; }
	List<MissionEvent>&  GetEvents()       { return events;  }
	List<SimRegion>&     GetRegions()      { return regions; }
	SimRegion*           FindRegion(const char* name);
	SimRegion*           FindRegion(OrbitalRegion* rgn);
	SimRegion*           FindNearestSpaceRegion(SimObject* object);
	SimRegion*           FindNearestSpaceRegion(Orbital* body);
	SimRegion*           FindNearestTerrainRegion(SimObject* object);
	SimRegion*           FindNearestRegion(SimObject* object, int type);
	bool                 ActivateRegion(SimRegion* rgn);

	void                 RequestHyperJump(Ship*        obj,
	SimRegion*   rgn,
	const Point& loc,
	int          type=0,
	Ship*        fc_src=0,
	Ship*        fc_dst=0);

	SimRegion*           GetActiveRegion() { return active_region; }
	StarSystem*          GetStarSystem()   { return star_system;   }
	List<StarSystem>&    GetSystemList();
	Scene*               GetScene()        { return &scene;        }
	Ship*                GetPlayerShip();
	Element*             GetPlayerElement();
	Orbital*             FindOrbitalBody(const char* name);

	void                 SetSelection(Ship* s);
	bool                 IsSelected(Ship* s);
	ListIter<Ship>       GetSelection();
	void                 ClearSelection();
	void                 AddSelection(Ship* s);

	void                 SetTestMode(bool t=true);

	bool                 IsTestMode()   const { return test_mode;        }
	bool                 IsNetGame()    const { return netgame != 0;     }
	bool                 IsActive()     const;
	bool                 IsComplete()   const;

	MotionController*    GetControls()  const { return ctrl; }

	Element*             FindElement(const char* name);
	List<Element>&       GetElements()        { return elements; }

	int                  GetAssignedElements(Element* elem, List<Element>& assigned);

	void                 SkipCutscene();
	void                 ResolveTimeSkip(double seconds);
	void                 ResolveHyperList();
	void                 ResolveSplashList();

	void                 ExecEvents(double seconds);
	void                 ProcessEventTrigger(int type, int event_id=0, const char* ship=0, int param=0);
	double               MissionClock() const;
	DWORD                StartTime()    const { return start_time; }

	// Create a list of mission elements based on the current
	// state of the simulation.  Used for multiplayer join in progress.
	ListIter<MissionElement> GetMissionElements();

protected:
	void                 CreateRegions();
	void                 CreateElements();
	void                 CopyEvents();
	void                 BuildLinks();

	// Convert a single live element into a mission element
	// that can be serialized over the net.
	MissionElement*      CreateMissionElement(Element* elem);
	Hangar*              FindSquadron(const char* name, int& index);

	static Sim*          sim;
	SimRegion*           active_region;
	StarSystem*          star_system;
	Scene                scene;
	Dust*                dust;
	CameraDirector*      cam_dir;

	List<SimRegion>      regions;
	List<SimRegion>      rgn_queue;
	List<SimHyper>       jumplist;
	List<SimSplash>      splashlist;
	List<Element>        elements;
	List<Element>        finished;
	List<MissionEvent>   events;
	List<MissionElement> mission_elements;

	MotionController*    ctrl;

	bool                 test_mode;
	bool                 grid_shown;
	Mission*             mission;

	NetGame*             netgame;
	DWORD                start_time;
};

// +--------------------------------------------------------------------+

class SimRegion
{
	friend class Sim;

public:
	static const char* TYPENAME() { return "SimRegion"; }

	enum { REAL_SPACE, AIR_SPACE };

	SimRegion(Sim* sim, const char* name, int type);
	SimRegion(Sim* sim, OrbitalRegion* rgn);
	virtual ~SimRegion();

	int operator == (const SimRegion& r) const { return (sim==r.sim) && (name==r.name); }
	int operator <  (const SimRegion& r) const;
	int operator <= (const SimRegion& r) const;

	virtual void         Activate();
	virtual void         Deactivate();
	virtual void         ExecFrame(double seconds);
	void                 ShowGrid(int show = true);
	void                 NextView();
	Ship*                FindShip(const char* name);
	Ship*                GetPlayerShip() { return player_ship; }
	void                 SetPlayerShip(Ship* ship);
	OrbitalRegion*       GetOrbitalRegion() { return orbital_region; }
	Terrain*             GetTerrain()       { return terrain;        }
	bool                 IsActive()   const { return active;         }
	bool                 IsAirSpace() const { return type == AIR_SPACE;  }
	bool                 IsOrbital()  const { return type == REAL_SPACE; }
	bool                 CanTimeSkip()const;

	virtual Ship*        FindShipByObjID(DWORD objid);
	virtual Shot*        FindShotByObjID(DWORD objid);

	virtual void         InsertObject(Ship*      ship);
	virtual void         InsertObject(Shot*      shot);
	virtual void         InsertObject(Explosion* explosion);
	virtual void         InsertObject(Debris*    debris);
	virtual void         InsertObject(Asteroid*  asteroid);

	const char*          Name() const { return name;            }
	int                  Type() const { return type;            }
	int                  NumShips()   { return ships.size(); }
	List<Ship>&          Ships()      { return ships;           }
	List<Ship>&          Carriers()   { return carriers;        }
	List<Shot>&          Shots()      { return shots;           }
	List<Drone>&         Drones()     { return drones;          }
	List<Debris>&        Rocks()      { return debris;          }
	List<Asteroid>&      Roids()      { return asteroids;       }
	List<Explosion>&     Explosions() { return explosions;      }
	List<SimRegion>&     Links()      { return links;           }
	StarSystem*          System()     { return star_system;     }

	Point                Location() const  { return location;   }

	void                 SetSelection(Ship* s);
	bool                 IsSelected(Ship* s);
	ListIter<Ship>       GetSelection();
	void                 ClearSelection();
	void                 AddSelection(Ship* s);

	List<Contact>&       TrackList(int iff);

	void                 ResolveTimeSkip(double seconds);

protected:
	void                 CommitMission();
	void                 TranslateObject(SimObject* object);

	void                 AttachPlayerShip(int index);
	void                 DestroyShips();
	void                 DestroyShip(Ship* ship);
	void                 NetDockShip(Ship* ship, Ship* carrier, FlightDeck* deck);

	void                 UpdateSky(double seconds, const Point& ref);
	void                 UpdateShips(double seconds);
	void                 UpdateShots(double seconds);
	void                 UpdateExplosions(double seconds);
	void                 UpdateTracks(double seconds);

	void                 DamageShips();
	void                 CollideShips();
	void                 CrashShips();
	void                 DockShips();

	Sim*                 sim;
	Text                 name;
	int                  type;
	StarSystem*          star_system;
	OrbitalRegion*       orbital_region;
	Point                location;
	Grid*                grid;
	Terrain*             terrain;
	bool                 active;

	Ship*                player_ship;
	int                  current_view;
	List<Ship>           ships;
	List<Ship>           carriers;
	List<Ship>           selection;
	List<Ship>           dead_ships;
	List<Shot>           shots;
	List<Drone>          drones;
	List<Explosion>      explosions;
	List<Debris>         debris;
	List<Asteroid>       asteroids;
	List<Contact>        track_database[5];
	List<SimRegion>      links;

	DWORD                sim_time;
	int                  ai_index;
};

#endif Sim_h

