/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmpCompleteDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Campaign title card and load progress dialog
*/

#ifndef CmpCompleteDlg_h
#define CmpCompleteDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class CmpnScreen;

// +--------------------------------------------------------------------+

class CmpCompleteDlg : public FormWindow
{
public:
	CmpCompleteDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
	virtual ~CmpCompleteDlg();

	virtual void      RegisterControls();
	virtual void      Show();

	// Operations:
	virtual void      ExecFrame();
	virtual void      OnClose(AWEvent* event);

protected:
	ImageBox*         img_title;
	ActiveWindow*     lbl_info;
	Button*           btn_close;
	Bitmap            banner;

	CmpnScreen*       manager;
};

#endif CmpCompleteDlg_h

