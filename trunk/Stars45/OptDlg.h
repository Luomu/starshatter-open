/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         OptDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#ifndef OptDlg_h
#define OptDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class BaseScreen;

// +--------------------------------------------------------------------+

class OptDlg : public FormWindow
{
public:
	OptDlg(Screen* s, FormDef& def, BaseScreen* mgr);
	virtual ~OptDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      Apply();
	virtual void      Cancel();

	virtual void      OnApply(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);

	virtual void      OnEnter(AWEvent* event);
	virtual void      OnExit(AWEvent* event);

	virtual void      OnAudio(AWEvent* event);
	virtual void      OnVideo(AWEvent* event);
	virtual void      OnOptions(AWEvent* event);
	virtual void      OnControls(AWEvent* event);
	virtual void      OnMod(AWEvent* event);

protected:
	BaseScreen*       manager;

	ComboBox*         flight_model;
	ComboBox*         flying_start;
	ComboBox*         landings;
	ComboBox*         ai_difficulty;
	ComboBox*         hud_mode;
	ComboBox*         hud_color;
	ComboBox*         joy_mode;
	ComboBox*         ff_mode;
	ComboBox*         grid_mode;
	ComboBox*         gunsight;

	ActiveWindow*     description;

	Button*           aud_btn;
	Button*           vid_btn;
	Button*           opt_btn;
	Button*           ctl_btn;
	Button*           mod_btn;

	Button*           apply;
	Button*           cancel;

	bool              closed;
};

#endif OptDlg_h

