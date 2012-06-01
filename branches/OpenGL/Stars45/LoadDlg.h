/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         LoadDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Loading progress dialog box
*/

#ifndef LoadDlg_h
#define LoadDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class LoadDlg : public FormWindow
{
public:
	LoadDlg(Screen* s, FormDef& def);
	virtual ~LoadDlg();

	virtual void      RegisterControls();

	// Operations:
	virtual void      ExecFrame();

protected:
	ActiveWindow*     title;
	ActiveWindow*     activity;
	Slider*           progress;
};

#endif LoadDlg_h

