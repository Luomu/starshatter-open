/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MFD.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Class for all Multi Function Displays
*/

#ifndef MFD_h
#define MFD_h

#include "Types.h"
#include "Geometry.h"
#include "Color.h"
#include "HUDView.h"

// +--------------------------------------------------------------------+

class Contact;
class Ship;
class Sprite;
class Window;

// +--------------------------------------------------------------------+

class MFD
{
public:
	enum Modes { MFD_MODE_OFF,  MFD_MODE_GAME, MFD_MODE_SHIP, 
		MFD_MODE_FOV,  /*MFD_MODE_FWD,  MFD_MODE_MAP,*/
		MFD_MODE_HSD,  MFD_MODE_3D
	};

	MFD(Window* c, int index);
	virtual ~MFD();

	int operator == (const MFD& that) const { return this == &that; }

	static void          Initialize();
	static void          Close();
	static void          SetColor(Color c);

	// Operations:
	virtual void         Draw();
	virtual void         DrawGameMFD();
	virtual void         DrawStatusMFD();
	virtual void         DrawSensorMFD();
	virtual void         DrawHSD();
	virtual void         Draw3D();
	virtual void         DrawSensorLabels(const char* mfd_mode);
	virtual void         DrawMap();
	virtual void         DrawGauge(int x, int y, int percent);
	virtual void         SetStatusColor(int status);

	virtual void         SetWindow(Window* w)    { window = w; }
	virtual Window*      GetWindow()             { return window; }
	virtual void         SetRect(const Rect& r);
	virtual const Rect&  GetRect() const         { return rect; }
	virtual void         SetMode(int m);
	virtual int          GetMode() const         { return mode; }
	virtual Sprite*      GetSprite()             { return sprite; }

	virtual void         SetShip(Ship* s)        { ship = s;    }
	virtual Ship*        GetShip()               { return ship; }

	virtual void         Show();
	virtual void         Hide();

	virtual void         UseCameraView(CameraView* v);
	void         DrawMFDText(int index, const char* txt, Rect& r, int align, int status=-1);
	void         HideMFDText(int index);
	void         SetText3DColor(Color c);
	void         SetCockpitHUDTexture(Bitmap* bmp) { cockpit_hud_texture = bmp; }

	bool         IsMouseLatched() const;

protected:

	enum { TXT_LAST=20 };

	Window*     window;
	Rect        rect;
	int         index;
	int         mode;
	int         lines;
	Sprite*     sprite;
	bool        hidden;
	Ship*       ship;
	HUDText     mfd_text[TXT_LAST];
	CameraView* camview;
	Bitmap*     cockpit_hud_texture;

	int         mouse_latch;
	bool        mouse_in;
};

#endif MFD_h

