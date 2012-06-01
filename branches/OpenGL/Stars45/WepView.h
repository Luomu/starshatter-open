/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         WepView.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	View class for Tactical HUD Overlay
*/

#ifndef WepView_h
#define WepView_h

#include "Types.h"
#include "View.h"
#include "Projector.h"
#include "Bitmap.h"
#include "Font.h"
#include "System.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class Graphic;
class Sprite;
class Ship;
class Contact;
class HUDView;

// +--------------------------------------------------------------------+

class WepView : public View,
public SimObserver
{
public:
	WepView(Window* c);
	virtual ~WepView();

	// Operations:
	virtual void      Refresh();
	virtual void      OnWindowMove();
	virtual void      ExecFrame();
	virtual void      SetOverlayMode(int mode);
	virtual int       GetOverlayMode()     const { return mode; }
	virtual void      CycleOverlayMode();

	virtual void      RestoreOverlay();

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

	static WepView*   GetInstance() { return wep_view; }
	static void       SetColor(Color c);

	static bool       IsMouseLatched();

protected:
	void              DrawOverlay();

	void              DoMouseFrame();
	bool              CheckButton(int index, int x, int y);
	void              CycleSubTarget(int direction);

	int         mode;
	int         transition;
	int         mouse_down;
	int         width, height, aw, ah;
	double      xcenter, ycenter;

	Sim*        sim;
	Ship*       ship;
	SimObject*  target;
	HUDView*    hud;

	enum { MAX_WEP = 4, MAX_BTN = 16 };
	Rect        btn_rect[MAX_BTN];

	SimRegion*  active_region;

	static WepView*   wep_view;
};

#endif WepView_h

