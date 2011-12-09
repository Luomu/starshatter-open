/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnNavDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Briefing Dialog Active Window class
*/

#ifndef MsnNavDlg_h
#define MsnNavDlg_h

#include "Types.h"
#include "MsnDlg.h"
#include "NavDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class PlanScreen;
class Campaign;
class Mission;
class MissionInfo;

// +--------------------------------------------------------------------+

class MsnNavDlg : public NavDlg,
public MsnDlg
{
public:
	MsnNavDlg(Screen* s, FormDef& def, PlanScreen* mgr);
	virtual ~MsnNavDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      OnCommit(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);
};

#endif MsnNavDlg_h

