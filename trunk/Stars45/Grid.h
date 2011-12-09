/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Grid.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Tactical Grid
*/

#ifndef Grid_h
#define Grid_h

#include "Types.h"
#include "Graphic.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Window;
class Projector;
class PolyRender;

// +--------------------------------------------------------------------+

class Grid : public Graphic
{
public:
	Grid(int size, int step);
	virtual ~Grid();

	virtual void      Render(Video* video, DWORD flags);
	virtual int       CollidesWith(Graphic& o);

protected:
	virtual void      DrawLine(Video* video, Point& p1, Point& p2, Color c);

	int               size;
	int               step;
	int               drawn;
};

// +--------------------------------------------------------------------+

#endif Grid_h

