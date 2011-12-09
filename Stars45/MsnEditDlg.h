/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnEditDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Editor Dialog Active Window class
*/

#ifndef MsnEditDlg_h
#define MsnEditDlg_h

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
class Campaign;
class Mission;
class MissionInfo;

// +--------------------------------------------------------------------+

class MsnEditDlg : public FormWindow
{
public:
	friend class MsnEditNavDlg;

	MsnEditDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~MsnEditDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();
	virtual void      ScrapeForm();

	// Operations:
	virtual void      OnAccept(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);
	virtual void      OnTabButton(AWEvent* event);
	virtual void      OnSystemSelect(AWEvent* event);
	virtual void      OnElemAdd(AWEvent* event);
	virtual void      OnElemEdit(AWEvent* event);
	virtual void      OnElemDel(AWEvent* event);
	virtual void      OnElemSelect(AWEvent* event);
	virtual void      OnElemInc(AWEvent* event);
	virtual void      OnElemDec(AWEvent* event);
	virtual void      OnEventAdd(AWEvent* event);
	virtual void      OnEventEdit(AWEvent* event);
	virtual void      OnEventDel(AWEvent* event);
	virtual void      OnEventSelect(AWEvent* event);
	virtual void      OnEventInc(AWEvent* event);
	virtual void      OnEventDec(AWEvent* event);

	virtual Mission*  GetMission() const      { return mission; }
	virtual void      SetMission(Mission* m);
	virtual void      SetMissionInfo(MissionInfo* m);

protected:
	virtual void      DrawPackages();
	virtual void      ShowTab(int tab);

	MenuScreen*       manager;

	Button*           btn_accept;
	Button*           btn_cancel;

	Button*           btn_elem_add;
	Button*           btn_elem_edit;
	Button*           btn_elem_del;
	Button*           btn_elem_inc;
	Button*           btn_elem_dec;

	Button*           btn_event_add;
	Button*           btn_event_edit;
	Button*           btn_event_del;
	Button*           btn_event_inc;
	Button*           btn_event_dec;

	Button*           btn_sit;
	Button*           btn_pkg;
	Button*           btn_map;

	EditBox*          txt_name;
	ComboBox*         cmb_type;
	ComboBox*         cmb_system;
	ComboBox*         cmb_region;

	EditBox*          txt_description;
	EditBox*          txt_situation;
	EditBox*          txt_objective;

	ListBox*          lst_elem;
	ListBox*          lst_event;

	Mission*          mission;
	MissionInfo*      mission_info;

	int               current_tab;
	bool              exit_latch;
};

#endif MsnEditDlg_h

