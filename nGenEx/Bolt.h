/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Bolt.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	3D Bolt (Polygon) Object
*/

#ifndef Bolt_h
#define Bolt_h

#include "Graphic.h"
#include "Polygon.h"

// +--------------------------------------------------------------------+

class Bolt : public Graphic
{
public:
	static const char* TYPENAME() { return "Bolt"; }

	Bolt(double len=16, double wid=1, Bitmap* tex=0, int share=0);
	virtual ~Bolt();

	// operations
	virtual void   Render(Video* video, DWORD flags);
	virtual void   Update();

	// accessors / mutators
	virtual void   SetOrientation(const Matrix& o);
	void           SetDirection(const Point& v);
	void           SetEndPoints(const Point& from, const Point& to);
	void           SetTextureOffset(double from, double to);

	virtual void   TranslateBy(const Point& ref);

	double         Shade()     const    { return shade;   }
	void           SetShade(double s)   { shade = s;      }
	virtual bool   IsBolt()       const { return true;    }

protected:
	double         length;
	double         width;
	double         shade;

	Poly           poly;
	Material       mtl;
	VertexSet      vset;
	Bitmap*        texture;
	int            shared;

	Point          vpn;
	Point          origin;
};

// +--------------------------------------------------------------------+

#endif Bolt_h

