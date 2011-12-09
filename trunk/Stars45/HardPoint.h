/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         HardPoint.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Hard Point (gun or missile launcher) class
*/

#ifndef HardPoint_h
#define HardPoint_h

#include "Types.h"
#include "Geometry.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Weapon;
class WeaponDesign;

// +--------------------------------------------------------------------+

class HardPoint
{
public:
	static const char* TYPENAME() { return "HardPoint"; }

	enum CONSTANTS { MAX_DESIGNS=8 };

	HardPoint(Vec3 muzzle, double az=0, double el=0);
	HardPoint(const HardPoint& rhs);
	virtual ~HardPoint();

	int operator==(const HardPoint& w) const { return this == &w; }

	virtual void      AddDesign(WeaponDesign* dsn);
	virtual Weapon*   CreateWeapon(int type_index=0);
	virtual double    GetCarryMass(int type_index=0);
	WeaponDesign*     GetWeaponDesign(int n) { return designs[n]; }

	virtual void      Mount(Point loc, float rad, float hull=0.5f);
	Point             MountLocation()               const { return mount_rel;   }
	double            Radius()                      const { return radius;      }
	double            HullProtection()              const { return hull_factor; }

	virtual const char* GetName()                   const { return name;        }
	virtual void        SetName(const char* s)            { name = s;           }
	virtual const char* GetAbbreviation()           const { return abrv;        }
	virtual void        SetAbbreviation(const char* s)    { abrv = s;           }
	virtual const char* GetDesign()                 const { return sys_dsn;     }
	virtual void        SetDesign(const char* s)          { sys_dsn = s;        }

	virtual double    GetAzimuth()                  const { return aim_azimuth;      }
	virtual void      SetAzimuth(double a)                { aim_azimuth = (float) a; }
	virtual double    GetElevation()                const { return aim_elevation; }
	virtual void      SetElevation(double e)              { aim_elevation = (float) e; }

protected:
	// Displayable name:
	Text              name;
	Text              abrv;
	Text              sys_dsn;

	WeaponDesign*     designs[MAX_DESIGNS];
	Vec3              muzzle;
	float             aim_azimuth;
	float             aim_elevation;

	// Mounting:
	Point             mount_rel;  // object space
	float             radius;
	float             hull_factor;
};

#endif HardPoint_h

