/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ExitDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Navigation Active Window class
*/

#ifndef ExitDlg_h
#define ExitDlg_h

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

// +--------------------------------------------------------------------+

class ExitDlg : public FormWindow
{
public:
	ExitDlg(Screen* s, FormDef& def, MenuScreen* mgr);
	virtual ~ExitDlg();

	virtual void      RegisterControls();
	virtual void      Show();

	// Operations:
	virtual void      ExecFrame();

	virtual void      OnApply(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);

protected:
	MenuScreen*       manager;

	RichTextBox*      credits;
	Button*           apply;
	Button*           cancel;
	Rect              def_rect;

	bool              exit_latch;
};

#endif ExitDlg_h

