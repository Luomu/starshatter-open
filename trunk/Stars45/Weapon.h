/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Weapon.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Weapon (gun or missile launcher) class
*/

#ifndef Weapon_h
#define Weapon_h

#include "Types.h"
#include "SimObject.h"
#include "System.h"
#include "WeaponDesign.h"
#include "Geometry.h"
#include "text.h"

// +--------------------------------------------------------------------+

class Weapon;
class Ship;
class Shot;

class Solid;

// +--------------------------------------------------------------------+

class Weapon : public System, public SimObserver
{
public:
	static const char* TYPENAME() { return "Weapon"; }

	enum Constants { MAX_BARRELS=8 };
	enum Orders    { MANUAL, AUTO, POINT_DEFENSE };
	enum Control   { SINGLE_FIRE, RIPPLE_FIRE, SALVO_FIRE };
	enum Sweep     { SWEEP_NONE, SWEEP_TIGHT, SWEEP_WIDE };

	Weapon(WeaponDesign* d, int nmuz, Vec3* muzzles, double az=0, double el=0);
	Weapon(const Weapon& rhs);
	virtual ~Weapon();

	int operator==(const Weapon& w) const { return this == &w; }

	int               Track(SimObject* targ, System* sub);
	Shot*             Fire();
	Shot*             NetFirePrimary(SimObject* targ, System* sub, int count);
	Shot*             NetFireSecondary(SimObject* targ, System* sub, DWORD objid);
	void              SetTarget(SimObject* t, System* sub);
	void              SelectTarget();
	bool              CanTarget(DWORD classification) const;
	SimObject*         GetTarget() const { return target;     }
	System*           GetSubTarget() const { return subtarget; }
	void              SetFiringOrders(int o);
	int               GetFiringOrders()          const    { return orders; }
	void              SetControlMode(int m);
	int               GetControlMode()           const    { return control; }
	void              SetSweep(int s);
	int               GetSweep()                 const    { return sweep;   }

	void              Enable()    { enabled = true; }
	void              Disable()   { enabled = false; }

	const WeaponDesign* Design()  const { return design;        }
	const char*       Group()     const { return group;         }
	int               Enabled()   const { return enabled;       }
	int               Ammo()      const { return ammo;          }
	int               Guided()    const { return guided;        }
	int               Locked()    const { return locked;        }
	float             Velocity()  const { return shot_speed;    }
	float             Mass()      const { return mass*ammo;     }
	float             Resistance()const { return resist*ammo;   }
	Shot*             GetBeam(int i);

	// needed to set proper ammo level when joining multiplayer in progress:
	void              SetAmmo(int a);

	bool              IsPrimary() const;
	bool              IsDrone()   const;
	bool              IsDecoy()   const;
	bool              IsProbe()   const;
	bool              IsMissile() const;
	bool              IsBeam()    const;

	virtual void      ExecFrame(double factor);
	virtual void      Orient(const Physical* rep);
	virtual void      Distribute(double delivered_energy, double seconds);

	const Ship*       Owner() const              { return ship;  }
	void              SetOwner(Ship* ship);
	int               GetIndex()  const          { return index; }
	void              SetIndex(int n)            { index = n;    }

	Point             GetAimVector() const       { return aim_cam.vpn();       }
	void              SetAzimuth(double a)       { aim_azimuth = (float) a;    }
	double            GetAzimuth()   const       { return aim_azimuth;         }
	void              SetElevation(double e)     { aim_elevation = (float) e;  }
	double            GetElevation() const       { return aim_elevation;       }

	void              SetRestAzimuth(double a)   { aim_az_rest = (float) a;    }
	double            GetRestAzimuth()   const   { return aim_az_rest;         }
	void              SetRestElevation(double e) { aim_el_rest = (float) e;    }
	double            GetRestElevation() const   { return aim_el_rest;         }

	void              SetAzimuthMax(double a)    { aim_az_max = (float) a;     }
	double            GetAzimuthMax() const      { return aim_az_max;          }
	void              SetAzimuthMin(double a)    { aim_az_min = (float) a;     }
	double            GetAzimuthMin() const      { return aim_az_min;          }

	void              SetElevationMax(double e)  { aim_el_max = (float) e;     }
	double            GetElevationMax() const    { return aim_el_max;          }
	void              SetElevationMin(double e)  { aim_el_min = (float) e;     }
	double            GetElevationMin() const    { return aim_el_min;          }

	void              SetGroup(const char* n)    { group = n;                  }

	bool              IsBlockedFriendly() const  { return blocked;             }
	void              SetBlockedFriendly(bool b) { blocked = b;                }

	Solid*            GetTurret();
	Solid*            GetTurretBase();
	Solid*            GetVisibleStore(int i);

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

protected:
	Shot*             FireBarrel(int n);
	Shot*             CreateShot(const Point& loc, const Camera& cam, WeaponDesign* dsn, const Ship* owner);

	void              SetBeamPoints(bool aim=false);
	void              Aim();
	void              AimTurret(double az, double el);
	void              ZeroAim();
	void              FindObjective();
	Point             Transform(const Point& pt);
	bool              CanLockPoint(const Point& test, double& az, double& el, Point* obj=0);

	// data members:
	WeaponDesign*     design;
	Text              group;
	Point             muzzle_pts[MAX_BARRELS];
	Point             rel_pts[MAX_BARRELS];
	Solid*            turret;
	Solid*            turret_base;
	Solid*            visible_stores[MAX_BARRELS];
	int               nbarrels;
	int               active_barrel;

	float             refire;
	int               ammo;
	int               ripple_count;

	// carrying costs per shot:
	float             mass;
	float             resist;

	// for targeting computer:
	int               guided;
	bool              enabled;
	bool              locked;
	bool              centered;
	bool              firing;
	bool              blocked;
	float             shot_speed;

	int               index;

	int               orders;
	int               control;
	int               sweep;

	SimObject*        target;
	System*           subtarget;

	Point             objective;
	Point             obj_w;
	Camera            aim_cam;
	float             aim_azimuth;
	float             aim_elevation;
	float             old_azimuth;
	float             old_elevation;

	// auto-aiming arc
	float             aim_az_max;       // maximum deflection in azimuth
	float             aim_az_min;       // minimum deflection in azimuth
	float             aim_az_rest;      // azimuth of turret at rest
	float             aim_el_max;       // maximum deflection in elevation
	float             aim_el_min;       // minimum deflection in elevation
	float             aim_el_rest;      // elevation of turret at rest

	DWORD             aim_time;

	Shot**            beams;
};

#endif Weapon_h

