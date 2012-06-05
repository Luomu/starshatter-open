/*  Project nGenEx
	Destroyer Studios LLC
	Copyright (C) 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Graphic.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Abstract 3D Graphic Object
*/

#ifndef Graphic_h
#define Graphic_h

#include "Geometry.h"
#include "Color.h"
#include "List.h"

// +--------------------------------------------------------------------+

#define GRAPHIC_DESTROY(x) if (x) { x->Destroy(); x = 0; } //-V571

// +--------------------------------------------------------------------+

class Projector;
class Light;
class Scene;
class Video;

// +--------------------------------------------------------------------+

class Graphic
{
public:
	static const char* TYPENAME() { return "Graphic"; }

	enum TYPE { OTHER, SOLID, SPRITE, BOLT, QUAD };

	enum RENDER_FLAGS {
		RENDER_SOLID         = 0x0001,
		RENDER_ALPHA         = 0x0002,
		RENDER_ADDITIVE      = 0x0004,
		RENDER_FIRST_LIGHT   = 0x1000,
		RENDER_ADD_LIGHT     = 0x2000
	};

	Graphic();
	virtual ~Graphic();

	int operator == (const Graphic& g) const { return id == g.id; }
	int operator <  (const Graphic& g) const;
	int operator <= (const Graphic& g) const;

	// operations
	virtual void      Render(Video* video, DWORD flags)   {  }
	virtual void      Update()                            {  }
	virtual void      SetOrientation(const Matrix& o)     {  }

	virtual int       CollidesWith(Graphic& o);

	// accessors / mutators
	int               Identity()     const { return id;      }
	const char*       Name()         const { return name;    }
	bool              IsVisible()    const { return visible; }
	void              SetVisible(bool v)   { visible = v;    }
	float             Radius()       const { return radius;  }

	Point             Location()     const { return loc;     }
	virtual void      MoveTo(const Point& p) { loc = p;      }
	virtual void      TranslateBy(const Point& ref) { loc = loc - ref; }

	virtual float     Depth()        const { return depth;   }
	virtual void      SetDepth(float d)    { depth = d;      }
	static int        Nearer(Graphic* a, Graphic* b);
	static int        Farther(Graphic* a, Graphic* b);

	virtual int       IsInfinite()   const { return infinite;      }
	virtual void      SetInfinite(bool b);
	virtual int       IsForeground() const { return foreground;    }
	virtual void      SetForeground(bool f){ foreground = f;       }
	virtual int       IsBackground() const { return background;    }
	virtual void      SetBackground(bool b){ background = b;       }

	virtual int       Hidden()       const { return hidden;  }
	virtual int       Life()         const { return life;    }
	virtual void      Destroy();
	virtual void      Hide()               { hidden = true;  }
	virtual void      Show()               { hidden = false; }
	virtual bool      Luminous()     const { return luminous;}
	virtual void      SetLuminous(bool l)  { luminous = l;   }
	virtual bool      Translucent()  const { return trans;   }
	virtual bool      CastsShadow()  const { return shadow;  }
	virtual void      SetShadow(bool s)    { shadow = s;     }

	virtual bool      IsSolid()      const { return false;   }
	virtual bool      IsSprite()     const { return false;   }
	virtual bool      IsBolt()       const { return false;   }
	virtual bool      IsQuad()       const { return false;   }

	virtual void      ProjectScreenRect(Projector* p);
	const Rect&       ScreenRect()   const { return screen_rect; }
	virtual Scene*    GetScene()     const { return scene;   }
	virtual void      SetScene(Scene*s)    { scene = s;      }

	virtual int       CheckRayIntersection(Point pt, Point vpn, double len, Point& ipt,
	bool treat_translucent_polys_as_solid=true);

	virtual bool      CheckVisibility(Projector& projector);

protected:
	static int        id_key;

	int               id;
	Point             loc;
	float             depth;
	float             radius;
	int               life;

	bool              visible;
	bool              infinite;
	bool              foreground;
	bool              background;
	bool              hidden;
	bool              trans;
	bool              shadow;
	bool              luminous;

	Rect              screen_rect;
	Scene*            scene;
	char              name[32];
};

// +--------------------------------------------------------------------+

#endif Graphic_h

