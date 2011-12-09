/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Shadow.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Dynamic Stencil Shadow Volumes
*/

#ifndef Shadow_h
#define Shadow_h

#include "Geometry.h"
#include "Color.h"

// +--------------------------------------------------------------------+

#define Shadow_DESTROY(x) if (x) { x->Destroy(); x = 0; }

// +--------------------------------------------------------------------+

class Light;
class Scene;
class Solid;
class Video;

// +--------------------------------------------------------------------+

class Shadow
{
public:
	static const char* TYPENAME() { return "Shadow"; }

	Shadow(Solid* solid);
	virtual ~Shadow();

	int operator == (const Shadow& s) const { return this == &s; }

	// operations
	void     Render(Video* video);
	void     Update(Light* light);
	void     AddEdge(WORD v1, WORD v2);
	void     Reset();

	bool     IsEnabled()          const { return enabled; }
	void     SetEnabled(bool e)         { enabled = e;    }

	static void SetVisibleShadowVolumes(bool vis);
	static bool GetVisibleShadowVolumes();

protected:
	Solid*   solid;
	Vec3*    verts;
	int      nverts;
	int      max_verts;
	bool     enabled;

	WORD*    edges;
	DWORD    num_edges;
};

// +--------------------------------------------------------------------+

#endif Shadow_h

