/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         AudDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#ifndef AudDlg_h
#define AudDlg_h

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

class AudDlg : public FormWindow
{
public:
	AudDlg(Screen* s, FormDef& def, BaseScreen* mgr);
	virtual ~AudDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      ExecFrame();

	// Operations:
	virtual void      Apply();
	virtual void      Cancel();

	virtual void      OnApply(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);

	virtual void      OnAudio(AWEvent* event);
	virtual void      OnVideo(AWEvent* event);
	virtual void      OnOptions(AWEvent* event);
	virtual void      OnControls(AWEvent* event);
	virtual void      OnMod(AWEvent* event);

protected:
	BaseScreen*       manager;

	Slider*           efx_volume_slider;
	Slider*           gui_volume_slider;
	Slider*           wrn_volume_slider;
	Slider*           vox_volume_slider;

	Slider*           menu_music_slider;
	Slider*           game_music_slider;

	Button*           aud_btn;
	Button*           vid_btn;
	Button*           opt_btn;
	Button*           ctl_btn;
	Button*           mod_btn;

	Button*           apply;
	Button*           cancel;

	bool              closed;
};

#endif AudDlg_h

