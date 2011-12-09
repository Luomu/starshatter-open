/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmdMissionsDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Operational Command Dialog (Mission List Tab)
*/

#ifndef CmdMissionsDlg_h
#define CmdMissionsDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CmdDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Mission;

// +--------------------------------------------------------------------+

class CmdMissionsDlg : public FormWindow,
public CmdDlg
{
public:
	CmdMissionsDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
	virtual ~CmdMissionsDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      OnMode(AWEvent* event);
	virtual void      OnSave(AWEvent* event);
	virtual void      OnExit(AWEvent* event);
	virtual void      OnMission(AWEvent* event);
	virtual void      OnAccept(AWEvent* event);

protected:
	CmpnScreen*       manager;

	ListBox*          lst_missions;
	ActiveWindow*     txt_desc;
	Button*           btn_accept;

	Starshatter*      stars;
	Campaign*         campaign;
	Mission*          mission;
};

#endif CmdMissionsDlg_h

