/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MenuDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#ifndef MenuDlg_h
#define MenuDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class  MenuScreen;
class  Campaign;
class  Starshatter;

// +--------------------------------------------------------------------+

class MenuDlg : public FormWindow
{
public:
	MenuDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~MenuDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      OnStart(AWEvent* event);
	virtual void      OnCampaign(AWEvent* event);
	virtual void      OnMission(AWEvent* event);
	virtual void      OnPlayer(AWEvent* event);
	virtual void      OnMultiplayer(AWEvent* event);
	virtual void      OnMod(AWEvent* event);
	virtual void      OnTacReference(AWEvent* event);

	virtual void      OnVideo(AWEvent* event);
	virtual void      OnOptions(AWEvent* event);
	virtual void      OnControls(AWEvent* event);
	virtual void      OnQuit(AWEvent* event);

	virtual void      OnButtonEnter(AWEvent* event);
	virtual void      OnButtonExit(AWEvent* event);

protected:
	MenuScreen*       manager;

	Button*           btn_start;
	Button*           btn_campaign;
	Button*           btn_mission;
	Button*           btn_player;
	Button*           btn_multi;
	Button*           btn_mod;
	Button*           btn_tac;

	Button*           btn_video;
	Button*           btn_options;
	Button*           btn_controls;
	Button*           btn_quit;

	ActiveWindow*     version;
	ActiveWindow*     description;

	Starshatter*      stars;
	Campaign*         campaign;
};

#endif MenuDlg_h

