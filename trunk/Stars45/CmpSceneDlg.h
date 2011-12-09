/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmpSceneDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Campaign title card and load progress dialog
*/

#ifndef CmpSceneDlg_h
#define CmpSceneDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CameraView.h"
#include "DisplayView.h"

#include "ImageBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmpnScreen;

// +--------------------------------------------------------------------+

class CmpSceneDlg : public FormWindow
{
public:
	CmpSceneDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
	virtual ~CmpSceneDlg();

	virtual void      RegisterControls();
	virtual void      Show();
	virtual void      Hide();

	// Operations:
	virtual void      ExecFrame();

	CameraView*       GetCameraView();
	DisplayView*      GetDisplayView();

protected:
	ActiveWindow*     mov_scene;
	RichTextBox*      subtitles_box;
	CameraView*       cam_view;
	DisplayView*      disp_view;
	Window*           old_disp_win;

	Bitmap*           flare1;
	Bitmap*           flare2;
	Bitmap*           flare3;
	Bitmap*           flare4;

	CmpnScreen*       manager;

	double            subtitles_delay;
	double            subtitles_time;
};

#endif CmpSceneDlg_h

