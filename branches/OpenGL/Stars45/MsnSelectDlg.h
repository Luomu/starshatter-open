/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnSelectDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Select Dialog Active Window class
*/

#ifndef MsnSelectDlg_h
#define MsnSelectDlg_h

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
class Starshatter;

// +--------------------------------------------------------------------+

class MsnSelectDlg : public FormWindow
{
public:
	MsnSelectDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~MsnSelectDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      OnCampaignSelect(AWEvent* event);
	virtual void      OnMissionSelect(AWEvent* event);

	virtual void      OnMod(AWEvent* event);
	virtual void      OnNew(AWEvent* event);
	virtual void      OnEdit(AWEvent* event);
	virtual void      OnDel(AWEvent* event);
	virtual void      OnDelConfirm(AWEvent* event);
	virtual void      OnAccept(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);

protected:
	MenuScreen*       manager;

	Button*           btn_mod;
	Button*           btn_new;
	Button*           btn_edit;
	Button*           btn_del;
	Button*           btn_accept;
	Button*           btn_cancel;

	ComboBox*         cmb_campaigns;
	ListBox*          lst_campaigns;
	ListBox*          lst_missions;

	ActiveWindow*     description;

	Starshatter*      stars;
	Campaign*         campaign;
	int               selected_mission;
	int               mission_id;
	bool              editable;
};

#endif MsnSelectDlg_h

