/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         KeyDlg.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Navigation Active Window class
*/

#ifndef KeyDlg_h
#define KeyDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class BaseScreen;

// +--------------------------------------------------------------------+

class KeyDlg : public FormWindow
{
public:
	KeyDlg(Screen* s, FormDef& def, BaseScreen* mgr);
	virtual ~KeyDlg();

	virtual void      RegisterControls();
	virtual void      Show();

	// Operations:
	virtual void      ExecFrame();

	virtual void      OnApply(AWEvent* event);
	virtual void      OnCancel(AWEvent* event);
	virtual void      OnClear(AWEvent* event);

	int               GetKeyMapIndex() const     { return key_index; }
	void              SetKeyMapIndex(int i);

protected:
	BaseScreen*       manager;

	int               key_index;
	int               key_key;
	int               key_shift;
	int               key_joy;
	int               key_clear;

	Button*           clear;
	Button*           apply;
	Button*           cancel;

	ActiveWindow*     command;
	ActiveWindow*     current_key;
	ActiveWindow*     new_key;
};

#endif KeyDlg_h

