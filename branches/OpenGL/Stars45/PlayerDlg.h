/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         PlayerDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#ifndef PlayerDlg_h
#define PlayerDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class Player;

// +--------------------------------------------------------------------+

class PlayerDlg : public FormWindow
{
public:
	PlayerDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~PlayerDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      OnApply(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);
	virtual void      OnSelectPlayer(AWEvent* event);
	virtual void      OnAdd(AWEvent* event);
	virtual void      OnDel(AWEvent* event);
	virtual void      OnDelConfirm(AWEvent* event);
	virtual void      OnRank(AWEvent* event);
	virtual void      OnMedal(AWEvent* event);

	virtual void      UpdatePlayer();
	virtual void      ShowPlayer();

protected:
	MenuScreen*       manager;

	ListBox*          lst_roster;
	Button*           btn_add;
	Button*           btn_del;

	EditBox*          txt_name;
	EditBox*          txt_password;
	EditBox*          txt_squadron;
	EditBox*          txt_signature;

	EditBox*          txt_chat[10];

	ActiveWindow*     lbl_createdate;
	ActiveWindow*     lbl_rank;
	ActiveWindow*     lbl_flighttime;
	ActiveWindow*     lbl_missions;
	ActiveWindow*     lbl_kills;
	ActiveWindow*     lbl_losses;
	ActiveWindow*     lbl_points;
	ImageBox*         img_rank;
	ImageBox*         img_medals[15];
	int               medals[15];

	Button*           apply;
	Button*           cancel;
};

#endif PlayerDlg_h

