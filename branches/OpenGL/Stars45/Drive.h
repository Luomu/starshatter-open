/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Drive.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Conventional Drive (system) class
*/

#ifndef Drive_h
#define Drive_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Bolt;
class DriveSprite;
class Light;
class Sound;
class Ship;

// +--------------------------------------------------------------------+

struct DrivePort {
	static const char* TYPENAME() { return "DrivePort"; }

	DrivePort(const Point& l, float s);
	~DrivePort();

	Point          loc;
	float          scale;

	DriveSprite*   flare;
	Bolt*          trail;
};

// +--------------------------------------------------------------------+

class Drive : public System
{
public:
	enum SUBTYPE   { PLASMA, FUSION, GREEN, RED, BLUE, YELLOW, STEALTH };
	enum Constants { MAX_ENGINES=16 };

	Drive(SUBTYPE s, float max_thrust, float max_aug, bool show_trail=true);
	Drive(const Drive& rhs);
	virtual ~Drive();

	static void       Initialize();
	static void       Close();
	static void       StartFrame();

	float             Thrust(double seconds);
	float             MaxThrust()          const { return thrust;        }
	float             MaxAugmenter()       const { return augmenter;     }
	int               NumEngines()         const;
	DriveSprite*      GetFlare(int port)   const;
	Bolt*             GetTrail(int port)   const;
	bool              IsAugmenterOn()      const;

	virtual void      AddPort(const Point& loc, float flare_scale=0);
	virtual void      CreatePort(const Point& loc, float flare_scale);

	virtual void      Orient(const Physical* rep);

	void              SetThrottle(double t, bool aug=false);
	virtual double    GetRequest(double seconds) const;

protected:
	float             thrust;
	float             augmenter;
	float             scale;
	float             throttle;
	float             augmenter_throttle;
	float             intensity;

	List<DrivePort>   ports;

	Sound*            sound;
	Sound*            burner_sound;
	bool              show_trail;
};

#endif Drive_h

