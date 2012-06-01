/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnObjDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Briefing Dialog Active Window class
*/

#ifndef MsnObjDlg_h
#define MsnObjDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "MsnDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "CameraView.h"
#include "Scene.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class PlanScreen;
class Campaign;
class Mission;
class MissionInfo;
class ShipSolid;

// +--------------------------------------------------------------------+

class MsnObjDlg : public FormWindow,
public MsnDlg
{
public:
	MsnObjDlg(Screen* s, FormDef& def, PlanScreen* mgr);
	virtual ~MsnObjDlg();

	virtual void      RegisterControls();
	virtual void      ExecFrame();
	virtual void      Show();

	// Operations:
	virtual void      OnCommit(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);
	virtual void      OnTabButton(AWEvent* event);
	virtual void      OnSkin(AWEvent* event);

protected:
	ActiveWindow*     objectives;
	ActiveWindow*     sitrep;
	ActiveWindow*     player_desc;
	ActiveWindow*     beauty;
	ComboBox*         cmb_skin;
	CameraView*       camview;
	Scene             scene;
	Camera            cam;
	ShipSolid*        ship;
};

#endif MsnObjDlg_h

