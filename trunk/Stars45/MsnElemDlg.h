/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnElemDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Editor Element Dialog Active Window class
*/

#ifndef MsnElemDlg_h
#define MsnElemDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class MsnEditDlg;
class Mission;
class MissionElement;

// +--------------------------------------------------------------------+

class MsnElemDlg : public FormWindow
{
public:
	MsnElemDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~MsnElemDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      SetMission(Mission* elem);
	virtual void      SetMissionElement(MissionElement* elem);
	virtual void      OnAccept(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);
	virtual void      OnClassSelect(AWEvent* event);
	virtual void      OnDesignSelect(AWEvent* event);
	virtual void      OnObjectiveSelect(AWEvent* event);
	virtual void      OnIFFChange(AWEvent* event);

	virtual void      UpdateTeamInfo();
	virtual bool      CanCommand(const MissionElement* commander,
	const MissionElement* subordinate) const;

protected:
	MenuScreen*       manager;

	EditBox*          edt_name;
	ComboBox*         cmb_class;
	ComboBox*         cmb_design;
	ComboBox*         cmb_skin;
	EditBox*          edt_size;
	EditBox*          edt_iff;
	ComboBox*         cmb_role;
	ComboBox*         cmb_region;
	EditBox*          edt_loc_x;
	EditBox*          edt_loc_y;
	EditBox*          edt_loc_z;
	ComboBox*         cmb_heading;
	EditBox*          edt_hold_time;

	Button*           btn_player;
	Button*           btn_playable;
	Button*           btn_alert;
	Button*           btn_command_ai;
	EditBox*          edt_respawns;
	ComboBox*         cmb_carrier;
	ComboBox*         cmb_squadron;
	ComboBox*         cmb_commander;
	ComboBox*         cmb_intel;
	ComboBox*         cmb_loadout;
	ComboBox*         cmb_objective;
	ComboBox*         cmb_target;

	Button*           btn_accept;
	Button*           btn_cancel;

	Mission*          mission;
	MissionElement*   elem;
	bool              exit_latch;
};

#endif MsnElemDlg_h

