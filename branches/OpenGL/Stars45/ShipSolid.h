/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ShipSolid.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	3D Solid (Polygon) Object
*/

#ifndef ShipSolid_h
#define ShipSolid_h

#include "Solid.h"

// +--------------------------------------------------------------------+

class Ship;
class Skin;

// +--------------------------------------------------------------------+

class ShipSolid : public Solid
{
public:
	static const char* TYPENAME() { return "ShipSolid"; }

	ShipSolid(Ship* s);
	virtual ~ShipSolid();

	virtual void   Render(Video* video, DWORD flags);
	virtual void   TranslateBy(const Point& ref);

	const Skin*    GetSkin()               const { return skin; }
	void           SetSkin(const Skin* s)        { skin = s;    }

protected:
	Ship*          ship;
	const Skin*    skin;
	Point          true_eye_point;
	Point          fog_loc;
	bool           in_soup;
};

// +--------------------------------------------------------------------+

#endif ShipSolid_h

